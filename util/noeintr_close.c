/**
 * Goal: portably close a file descriptor in a multithreaded process without
 * running into problems with EINTR.  POSIX specifies that close(2) may be
 * interrupted by a signal and return -1 with errno set to EINTR, but
 * explicitly does not specify whether the file descriptor has been closed if
 * that occurs.
 *
 * Run once:
 * 1. Create a socket pair s[2].
 * 2. Write a random cookie into s[0].
 *
 * To close(fd), do the following steps wrapped in a mutex:
 * 1. dup2(s[1], fd) until it succeeds.  (If it fails with !EINTR, bail.)
 * 2. close(fd).
 * 3. If we got EINTR, recv(fd, MSG_PEEK).
 * 4. If we read the random cookie, goto 2.
 *
 * The first trick here is that unlike close(), dup2() has well-defined
 * semantics in POSIX with respect to EINTR: It's atomic, so there's no way
 * for another thread to accidentally reuse the same descriptor # between the
 * implied close and reopen.
 *
 * The second trick here is that while we can use dup2 to guarantee that
 * the original descriptor was safely closed, we now need to close the
 * duplicate -- but we can distinguish between "EINTR and closed" and "EINTR
 * and still open" by seeing if MSG_PEEK returns our magic cookie.
 *
 * If we're on a perverse OS where close(fd) can successfully close the
 * descriptor and still return -1 / EINTR, *and* another thread wins a race
 * and reuses that descriptor, there's still no way we'll see our random
 * cookie, so we won't make the mistake of trying to close it again.
 */
#include <sys/socket.h>

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "entropy.h"
#include "optional_mutex.h"
#include "warnp.h"

#include "noeintr.h"

#ifdef NOEINTR_CLOSE_TESTING
#include "noeintr_close_testing.h"
#endif

/* Synchronization. */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/* Initialization. */
static enum noeintr_close_status {
	NOEINTR_UNINITIALIZED,
	NOEINTR_INITIALIZED,
	NOEINTR_FATAL
} status = NOEINTR_UNINITIALIZED;

/* Magic cookie. */
#define MAGIC_COOKIE_LEN 32
static int s[2] = {-1, -1};
static uint8_t cookie_write[MAGIC_COOKIE_LEN];
static struct entropy_read_cookie * entropy_read_cookie = NULL;

/* Clean up on program exit. */
static void
noeintr_close_atexit(void)
{

	/*
	 * Clean up the entropy_read_cookie; this may call noeintr_close().
	 * As such, it must be handled before freeing any other resources.
	 */
	if (entropy_read_cookie != NULL)
		entropy_read_done(entropy_read_cookie);

	/* Attempt to close s[0] and s[1] using the normal close(). */
	if ((s[0] != -1) && close(s[0]))
		warnp("close");
	if ((s[1] != -1) && close(s[1]))
		warnp("close");
}

/* Initialize socketpair, atexit, and magic cookie. */
static int
noeintr_close_init(void)
{

	/* Create a socket pair. */
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, s)) {
		warnp("socketpair");
		goto err1;
	}

	/* Set up cleanup function. */
	if (atexit(noeintr_close_atexit)) {
		warnp("atexit");
		goto err2;
	}

	/* Entropy cookie; don't clean it until noeintr_close_atexit(). */
	if ((entropy_read_cookie = entropy_read_init()) == NULL) {
		warn0("entropy_read_init");
		goto err2;
	}

	/* Generate a random cookie. */
	if (entropy_read_fill(entropy_read_cookie, cookie_write,
	    MAGIC_COOKIE_LEN)) {
		warn0("entropy_read");
		goto err2;
	}

	/* Write the cookie to s[0]. */
	if (noeintr_write(s[0], cookie_write, MAGIC_COOKIE_LEN)
	    != MAGIC_COOKIE_LEN) {
		warnp("noeintr_write");
		goto err2;
	}

	/* We've finished initializing. */
	status = NOEINTR_INITIALIZED;

	/* Success! */
	return (0);

err2:
	/* Attempt to close s[0] and s[1] using the normal close(). */
	if (close(s[0]))
		warnp("close");
	s[0] = -1;
	if (close(s[1]))
		warnp("close");
	s[1] = -1;
err1:
	status = NOEINTR_FATAL;

	/* Failure! */
	return (-1);
}

/* Check if ${sock} produces our magic cookie.  Return 0 if it doesn't. */
static int
is_sock_still_open(int sock)
{
	uint8_t cookie_recv[MAGIC_COOKIE_LEN];

	/* Read from ${sock}. */
	errno = 0;
	while (recv(sock, &cookie_recv, MAGIC_COOKIE_LEN, MSG_PEEK)
	    != MAGIC_COOKIE_LEN) {
		/*
		 * If recv() didn't read the full value but doesn't
		 * have an errno, try again.
		 */
		if (errno == 0)
			continue;

		/*
		 * If the descriptor is not open to a socket, it must
		 * have been closed.
		 */
		if ((errno == EBADF) || (errno == ENOTSOCK))
			goto closed;

		/* Handle non-EINTR errors. */
		if (errno != EINTR) {
			warnp("recv");
			goto err0;
		}
	}

	/* Check if what we read was the magic cookie. */
	if ((memcmp(cookie_write, cookie_recv, MAGIC_COOKIE_LEN) != 0)) {
		/*
		 * If this descriptor doesn't have the cookie we
		 * stuffed into s[0], it no longer refers to the same
		 * file as s[1]; the close call must have succeeded.
		 */
		goto closed;
	}

	/* Success: we've determined that the descriptor is still open. */
	return (1);

closed:
	/* Success: it was closed! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/**
 * noeintr_close(fd):
 * Close the file descriptor ${fd} per the close(2) system call, but retry on
 * EINTR if the descriptor was not closed.  Unlike close(2), this function is
 * not async-signal-safe.
 */
int
noeintr_close(int fd)
{
	int rc;

	/* Lock if we're using pthread; otherwise, do nothing. */
	if ((rc = optional_mutex_lock(&mutex)) != 0) {
		warn0("optional_mutex_lock: %s", strerror(rc));
		goto err0;
	}

	/* If we failed to initialize, don't try again. */
	if (status == NOEINTR_FATAL)
		goto err1;

	/* Initialize (if needed). */
	if ((status == NOEINTR_UNINITIALIZED) && noeintr_close_init()) {
		warn0("noeintr_close_init");
		goto err1;
	}

	/* Prepare a duplicate file descriptor. */
	while (dup2(s[1], fd) == -1) {
		if (errno == EINTR) {
			/*
			 * POSIX specifies that the file descriptor fd is
			 * unchanged, so we can retry.
			 */
			continue;
		}

		/* Bail if it's any other error. */
		warnp("dup2");
		goto err1;
	}

	/* Close the descriptor, which now refers to the same socket at s[1]. */
#ifdef NOEINTR_CLOSE_TESTING
	while (noeintr_close_testing_evil_close(fd)) {
#else
	while (close(fd)) {
#endif
		/*
		 * Since fd is now a copy of s[1], the only way this close()
		 * should be able to fail is with EINTR.
		 */
		if (errno != EINTR)
			goto err1;

		/*
		 * Check if EINTR was returned with the descriptor still
		 * referring to the same socket as s[1]; if so, the
		 * descriptor wasn't actually closed.
		 */
		switch (is_sock_still_open(fd)) {
		case -1:
			/* Fatal error. */
			goto err1;
		case 0:
			/* The file descriptor was closed. */
			goto closed;
		case 1:
			/* We saw the magic cookie; loop. */
			continue;
		}
	}

closed:
	/* Unlock if we're using pthread; otherwise, do nothing. */
	if ((rc = optional_mutex_unlock(&mutex)) != 0) {
		warn0("optional_mutex_unlock: %s", strerror(rc));
		goto err0;
	}

	/* Success! */
	return (0);

err1:
	/* Unlock if we're using pthread; otherwise, do nothing. */
	if ((rc = optional_mutex_unlock(&mutex)) != 0)
		warn0("optional_mutex_unlock: %s", strerror(rc));
err0:
	/* Failure! */
	return (-1);
}

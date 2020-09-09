#include <sys/socket.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "noeintr.h"
#include "warnp.h"

#include "posix_close.h"

/*
 * The two files pthread_close_normal.c and pthread_close_pthread.c should be
 * identical, other than the #define USE_PTHREAD line.
 */
#define USE_PTHREAD 0

#if USE_PTHREAD
#include <pthread.h>

static pthread_mutex_t mutex;
#endif

#define MAGIC_COOKIE_LEN 4

/**
 * posix_close(fd, flags):
 * Close a descriptor, but looping until completion if interrupted by a signal.
 * ${flags} must be 0.
 */
int
posix_close(int fd, int flags)
{
	uint8_t cookie_write[MAGIC_COOKIE_LEN];
	uint8_t cookie_recv[MAGIC_COOKIE_LEN];
	int s[2];
	int i;
	ssize_t r;

	/* Sanity check. */
	assert(flags == 0);

	/* Create a socket pair. */
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, s)) {
		warnp("socketpair");
		goto err0;
	}

	/* Generate a random cookie. */
	for (i = 0; i < MAGIC_COOKIE_LEN; i++)
		cookie_write[i] = random() & 0xff;

	/* Write the cookie to s[0]. */
	if (noeintr_write(s[0], cookie_write, MAGIC_COOKIE_LEN) == -1) {
		warnp("noeintr_write");
		goto err1;
	}

	/* In a multithreaded environment, lock the mutex here. */
#if USE_PTHREAD
	if ((rc = pthread_mutex_lock(mutex)) != 0) {
		warn0("pthread_mutex_lock: %s", strerror(rc);
		goto err1;
	}
#endif

	/* Prepare a duplicate file descriptor. */
	while (dup2(s[1], fd) == -1) {
		if (errno != EINTR) {
			warnp("dup2");
			goto err1;
		}
	}

	/* Attempt to close the original descriptor. */
	while (close(fd)) {
		if (errno != EINTR) {
			warnp("close");
			goto err1;
		}

		/* Try to read the cookie. */
		if ((r = recv(fd, &cookie_recv, MAGIC_COOKIE_LEN, MSG_PEEK))
		    == -1) {
			if (errno != EINTR) {
				warnp("recv");
				goto err1;
			}
		}

		/* If we read the correct amount, check if it's the cookie. */
		if ((r == MAGIC_COOKIE_LEN) && (memcmp(cookie_write,
		    cookie_recv, MAGIC_COOKIE_LEN) != 0)) {
			/* No cookie; don't try to close again. */
			break;
		}
	}

	/* In a multithreaded environment, unlock the mutex here. */
#if USE_PTHREAD
	if ((rc = pthread_mutex_unlock(mutex)) != 0) {
		warn0("pthread_mutex_unlock: %s", strerror(rc);
		goto err1;
	}
#endif

	/* Clean up; if we get EINTR here, then oh well. */
	if (close(s[0]))
		warnp("close");
	if (close(s[1]))
		warnp("close");

	/* Success! */
	return (0);

err1:
	/* Attempt to close s[0] and s[1] using the normal close(). */
	if (close(s[0]))
		warnp("close");
	if (close(s[1]))
		warnp("close");
err0:
	/* Failure! */
	return (-1);
}

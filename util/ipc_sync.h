#ifndef IPC_SYNC_H_
#define IPC_SYNC_H_

#include <errno.h>
#include <unistd.h>

#include "noeintr.h"
#include "warnp.h"

/* Convention for read/write ends of a pipe. */
#define W 1
#define R 0

/* Data for each barrier. */
struct ipc_sync {
	int fd[2];
};

/**
 * ipc_sync_init(IS):
 * Initialize an inter-process synchronization barrier in ${IS}.
 */
static inline int ipc_sync_init(struct ipc_sync *);

/**
 * ipc_sync_wait(IS):
 * Block until ipc_sync_signal() has been called with ${IS}.
 */
static inline int ipc_sync_wait(struct ipc_sync *);

/**
 * ipc_sync_signal(IS):
 * Indicate that ${IS} should no longer block.
 */
static inline int ipc_sync_signal(struct ipc_sync *);

/**
 * ipc_sync_done(IS):
 * Free resources associated with ${IS}.
 */
static inline int ipc_sync_done(struct ipc_sync *);

/* Implementation details below. */

/**
 * ipc_sync_init(IS):
 * Initialize an inter-process synchronization barrier in ${IS}.
 */
static inline int
ipc_sync_init(struct ipc_sync * IS)
{

	/* Set up synchronization. */
	if (pipe(IS->fd)) {
		warnp("pipe");
		goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/**
 * ipc_sync_wait(IS):
 * Block until ipc_sync_signal() has been called with ${IS}.
 */
static inline int
ipc_sync_wait(struct ipc_sync * IS)
{
	char dummy;
	char done = 0;

	/*
	 * Close write end of pipe so that if the other process dies we will
	 * notice the pipe being reset.
	 */
	while (close(IS->fd[W])) {
		if (errno == EINTR)
			continue;
		warnp("close");
		goto err0;
	}
	IS->fd[W] = -1;

	/* Read one byte from IS->fd[R]. */
	do {
		switch (read(IS->fd[R], &dummy, 1)) {
		case -1:
			/* Anything other than EINTR is bad. */
			if (errno != EINTR) {
				warnp("read");
				goto err0;
			}

			/* Otherwise, loop and read again. */
			break;
		case 0:
			warn0("Unexpected EOF in pipe");
			goto err0;
		case 1:
			/* Expected value; quit the loop. */
			done = 1;
		}
	} while (!done);

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/**
 * ipc_sync_signal(IS):
 * Indicate that ${IS} should no longer block.
 */
static inline int
ipc_sync_signal(struct ipc_sync * IS)
{
	char dummy = 0;

	if (noeintr_write(IS->fd[W], &dummy, 1) == -1) {
		warnp("write");
		goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/**
 * ipc_sync_done(IS):
 * Free resources associated with ${IS}.
 */
static inline int
ipc_sync_done(struct ipc_sync * IS)
{

	/* Close any open file descriptors. */
	if (IS->fd[R] != -1) {
		while (close(IS->fd[R])) {
			if (errno == EINTR)
				continue;
			warnp("close");
			goto err1;
		}
	}
	if (IS->fd[W] != -1 ) {
		while (close(IS->fd[W])) {
			if (errno == EINTR)
				continue;
			warnp("close");
			goto err0;
		}
	}

	/* Success! */
	return (0);

err1:
	if (IS->fd[W] != -1 ) {
		while (close(IS->fd[W])) {
			if (errno == EINTR)
				continue;
			warnp("close");
			goto err1;
		}
	}
err0:
	/* Failure! */
	return (-1);
}

#endif /* !IPC_SYNC_H_ */

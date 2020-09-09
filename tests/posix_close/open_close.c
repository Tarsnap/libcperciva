#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "posix_close.h"
#include "warnp.h"

#include "open_close.h"

static int alarmed = 0;

struct open_close {
	const char * filename;
	int numbytes;
	volatile int * stop;
};

static void
alarm_handler(int signo)
{

	(void)signo; /*	UNUSED */
	alarmed++;
}

static int
start_catch_signal(void)
{
	struct sigaction act;

	/* Initialize sigaction. */
	act.sa_sigaction = NULL;
	act.sa_handler = alarm_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGALRM);

	/* Catch SIGALRM. */
	if (sigaction(SIGALRM, &act, NULL)) {
		warnp("signal");
		goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

static void *
workthread_open_close(void * cookie)
{
	struct open_close * OC = cookie;
	int fd;
	char buf[1];
	ssize_t r;

	/* Start signal handler to catch SIGALRM. */
	start_catch_signal();

	while (*OC->stop == 0) {
		/* Open the file. */
		if ((fd = open(OC->filename, O_RDONLY)) == -1) {
			warnp("open(%s)", OC->filename);
			goto err0;
		}

		/*
		 * Read from the file.  If numbytes is 0, some platforms will
		 * still be able to detect certain problems with fd.
		 */
		if ((r = read(fd, buf, (size_t)OC->numbytes)) == -1) {
			warnp("read");
			goto err1;
		}
		if (r != (ssize_t)OC->numbytes) {
			warn0("wanted %zu byte(s) from read; got %zi",
			    OC->numbytes, r);
			goto err1;
		}

		/* Close the file. */
		if (posix_close(fd, 0)) {
			warnp("posix_close");
			goto err0;
		}
	}

	warn0("caught SIGALRM: %i\n", alarmed);
	return (NULL);

err1:
	close(fd);
err0:
	warn0("Failed!");
	return (NULL);
}

/*
 * start_repeated_open_close(filename, numbytes, stop, thr):
 * Start a thread, stored in ${thr}, to repeatedly open ${filename}, read
 * ${numbytes}, then close it.  Stop when ${*stop} is non-zero.  ${numbytes}
 * must be 0 or 1.  Catch SIGALRM.
 */
int
start_repeated_open_close(const char * filename, int numbytes,
    volatile int * stop, pthread_t * thr)
{
	struct open_close * OC;
	int rc;

	/* Sanity check. */
	assert((numbytes == 0) || (numbytes == 1));

	/* Allocate structure. */
	if ((OC = malloc(sizeof(struct open_close))) == NULL)
		goto err0;
	OC->filename = filename;
	OC->numbytes = numbytes;
	OC->stop = stop;

	/* Create thread. */
	if ((rc = pthread_create(thr, NULL, workthread_open_close, OC)) != 0) {
		warn0("pthread_create: %s", strerror(rc));
		goto err1;
	}

	/* Success! */
	return (0);

err1:
	free(OC);
err0:
	/* Failure! */
	return(1);
}

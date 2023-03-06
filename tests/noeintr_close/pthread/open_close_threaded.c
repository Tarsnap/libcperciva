#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "warnp.h"

#include "open_close.h"
#include "open_close_threaded.h"

struct open_close {
	pthread_t thr;
	const char * filename;
	int nbytes;
	volatile int * stop;
	size_t nopen_close;
	int rc;
};

static void *
workthread_open_close(void * cookie)
{
	struct open_close * OC = cookie;

	while (*OC->stop == 0) {
		if (open_close(OC->filename, OC->nbytes))
			goto err0;

		/* Update count. */
		OC->nopen_close++;
	}

	/* Success! */
	return (NULL);

err0:
	/* Failure! */
	OC->rc = -1;
	return (NULL);
}

/**
 * open_close_threaded_start(filename, nbytes, stop):
 * Start a thread to repeatedly open ${filename}, read ${nbytes}, then close
 * it.  Stop when ${*stop} is non-zero.  ${nbytes} must be 0 or 1.
 */
struct open_close *
open_close_threaded_start(const char * filename, int nbytes,
    volatile int * stop)
{
	struct open_close * OC;
	int rc;

	/* Sanity check. */
	assert((nbytes == 0) || (nbytes == 1));

	/* Allocate structure. */
	if ((OC = malloc(sizeof(struct open_close))) == NULL)
		goto err0;
	OC->filename = filename;
	OC->nbytes = nbytes;
	OC->stop = stop;
	OC->nopen_close = 0;
	OC->rc = 0;

	/* Create thread. */
	if ((rc = pthread_create(&OC->thr, NULL, workthread_open_close, OC))
	    != 0) {
		warn0("pthread_create: %s", strerror(rc));
		goto err1;
	}

	/* Success! */
	return (OC);

err1:
	free(OC);
err0:
	/* Failure! */
	return (NULL);
}

/**
 * open_close_threaded_join(OC):
 * Join the thread in ${OC}.
 */
int
open_close_threaded_join(struct open_close * OC)
{
	int rc;

	if ((rc = pthread_join(OC->thr, NULL)) != 0) {
		warn0("pthread_join: %s", strerror(rc));
		goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/**
 * open_close_threaded_cleanup(OC):
 * Check ${OC} for any previous errors, print statistics, and free memory.
 */
int
open_close_threaded_cleanup(struct open_close * OC)
{

	/* Bail if we've already cleaned up. */
	if (OC == NULL)
		return (0);

	/* Check for any previous errors. */
	if (OC->rc != 0)
		goto err1;

	/* Print statistics. */
	printf("Opened and closed the file %zu times.\n", OC->nopen_close);

	/* Clean up. */
	free(OC);

	/* Success! */
	return (0);

err1:
	free(OC);

	/* Failure! */
	return (-1);
}

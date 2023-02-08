#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef NUM_THREADS
#include <pthread.h>
#endif

#include "optional_mutex.h"
#include "warnp.h"

#define NUM_TIMES 100

static void * mutex;
static int value = 0;
static int counter = 0;

/* Wait duration can be interrupted by signals. */
static inline void
millisleep(size_t msec)
{
	struct timespec ts;

	ts.tv_sec = msec / 1000;
	ts.tv_nsec = (msec % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

static void *
inc_dec_counter(void * cookie)
{
	int i;
	int rc;

	(void)cookie; /* UNUSED */

	for (i = 0; i < NUM_TIMES; i++) {
		/* Lock if we're using pthread. */
		if ((rc = optional_mutex_lock(mutex))) {
			warn0("optional_mutex_lock: %s", strerror(rc));
			goto err0;
		}

		/* Increment, wait, decrement value. */
		value++;
		millisleep(10);
		value--;

		/* Increment overall counter. */
		counter++;

		/* Unlock if we're using pthread. */
		if ((rc = optional_mutex_unlock(mutex))) {
			warn0("optional_mutex_unlock: %s", strerror(rc));
			goto err0;
		}
	}

	/* Success! */
	return (NULL);

err0:
	/* Failure! */
	return (NULL);
}

#ifdef NUM_THREADS
static int
test_pthread(void)
{
	pthread_t thr[NUM_THREADS];
	int rc;
	int i;

	/* Create threads. */
	for (i = 0; i < NUM_THREADS; i++) {
		if ((rc = pthread_create(&thr[i], NULL, inc_dec_counter,
		    NULL))) {
			warn0("pthread_create: %s", strerror(rc));
			goto err0;
		}
	}

	/* Wait for threads to finish. */
	for (i = 0; i < NUM_THREADS; i++) {
		if ((rc = pthread_join(thr[i], NULL))) {
			warn0("pthread_join: %s", strerror(rc));
			goto err0;
		}
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}
#endif

int
main(int argc, char * argv[])
{
	int counter_expected;
	int rc;

	WARNP_INIT;
	(void)argc; /* UNUSED */

	/* Create the mutex if we're using pthread. */
	if ((optional_mutex_alloc(&mutex))) {
		warnp("optional_mutex_alloc");
		goto err0;
	}
	if ((rc = optional_mutex_init(mutex))) {
		warn0("optional_mutex_init: %s", strerror(rc));
		goto err1;
	}

#ifdef NUM_THREADS
	counter_expected = NUM_TIMES * NUM_THREADS;
	if (test_pthread())
		goto err2;
#else
	counter_expected = NUM_TIMES;
	inc_dec_counter(NULL);
#endif

	/* Check the counter. */
	if ((value != 0) || (counter != counter_expected))
		goto err2;

	/* Destroy the mutex if we're using pthread. */
	if ((rc = optional_mutex_destroy(mutex))) {
		warn0("optional_mutex_destroy: %s", strerror(rc));
		goto err1;
	}
	optional_mutex_free(mutex);

	/* Success! */
	exit(0);

err2:
	if ((rc = optional_mutex_destroy(mutex)))
		warn0("optional_mutex_destroy: %s", strerror(rc));
err1:
	optional_mutex_free(mutex);
err0:
	/* Failure! */
	exit(1);
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "noeintr_close_testing.h"
#include "parsenum.h"
#include "warnp.h"

#include "open_close_threaded.h"

#define NUM_THREADS 10

/* Wait duration can be interrupted by signals. */
static int
wait_ms(size_t msec)
{
	struct timespec ts;

	/* Try to wait for the desired duration. */
	ts.tv_sec = msec / 1000;
	ts.tv_nsec = (msec % 1000) * 1000000;
	nanosleep(&ts, NULL);

	/* Success! */
	return (0);
}

static int
check_open_close_threaded(const char * filename, int nbytes)
{
	struct open_close * OC[NUM_THREADS];
	volatile int stop_open_close = 0;
	size_t i;

	/* Indicate that we have no open_close threads yet. */
	for (i = 0; i < NUM_THREADS; i++)
		OC[i] = NULL;

	/* Start threads to open, read 0 or 1 bytes, then close the file. */
	for (i = 0; i < NUM_THREADS; i++) {
		if ((OC[i] = open_close_threaded_start(filename, nbytes,
		    &stop_open_close)) == NULL)
			goto err1;
	}

	/* Let the threads do some open & close operations. */
	wait_ms(100);

	/* Stop thread(s) and wait for them to finish. */
	stop_open_close = 1;
	for (i = 0; i < NUM_THREADS; i++) {
		if (open_close_threaded_join(OC[i]))
			goto err1;
	}

	/* Check for any errors and free memory. */
	for (i = 0; i < NUM_THREADS; i++) {
		if (open_close_threaded_cleanup(OC[i])) {
			OC[i] = NULL;
			goto err1;
		}
		OC[i] = NULL;
	}

	/*
	 * Print stats.  It's expected that the sum of these values will not
	 * equal the number of open_close() function calls, because the whole
	 * point of noeintr_close's evil_close() is that calling it might not
	 * result in the file descriptor being closed.
	 */
	noeintr_close_testing_print_stats();

	/* Success! */
	return (0);

err1:
	/* Free memory. */
	for (i = 0; i < NUM_THREADS; i++)
		open_close_threaded_cleanup(OC[i]);

	/* Failure! */
	return (-1);
}

int
main(int argc, char * argv[])
{
	const char * filename;
	int nbytes;

	WARNP_INIT;

	/* Process arguments. */
	if (argc != 3) {
		fprintf(stderr, "usage: %s filename nbytes\n", argv[0]);
		goto err0;
	}
	filename = argv[1];
	if (PARSENUM(&nbytes, argv[2], 0, 1)) {
		warn0("%s must be 0 or 1", argv[2]);
		goto err0;
	}

	/* Run test. */
	if (check_open_close_threaded(filename, nbytes))
		goto err0;

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

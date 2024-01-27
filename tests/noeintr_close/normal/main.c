#include <stdio.h>
#include <stdlib.h>

#include "noeintr_close_testing.h"
#include "parsenum.h"
#include "warnp.h"

#include "open_close.h"

static int
check_open_close(const char * filename, int nbytes)
{
	int i;

	/* Open and close a file descriptor a bunch of times. */
	for (i = 0; i < 100; i++) {
		if (open_close(filename, nbytes)) {
			warnp("open_close");
			goto err0;
		}
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

err0:
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
		warn0("nbytes must be 0 or 1");
		goto err0;
	}

	/* Run test. */
	if (check_open_close(filename, nbytes))
		goto err0;

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

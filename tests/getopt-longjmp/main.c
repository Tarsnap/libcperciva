#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

enum E {
	ZERO,
	ONE
};

/* Error on Solaris 11.4 with gcc 7.3.0 -O1. */
static int
getopt_int(int argc, char * argv[])
{
	/* Command-line arguments. */
	int a = 0;

	/* Working variable. */
	const char * ch;

	/* Parse with magic getopt. */
	while ((ch = GETOPT(argc, argv)) != NULL) {
		GETOPT_SWITCH(ch) {
		GETOPT_OPT("-a"):
			a = 1;
			break;
		GETOPT_DEFAULT:
			break;
		}
	}

	/* Check for undesired assignment. */
	if (a == 1)
		goto err0;

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/* Error on Solaris 11.4 with gcc 7.3.0 -O1 and -O2. */
static int
getopt_int_int(int argc, char * argv[])
{
	/* Command-line arguments. */
	int a = 0;
	int b = 0;

	/* Working variable. */
	const char * ch;

	/* Parse with magic getopt. */
	while ((ch = GETOPT(argc, argv)) != NULL) {
		GETOPT_SWITCH(ch) {
		GETOPT_OPT("-a"):
			a = 1;
			break;
		GETOPT_OPT("-b"):
			b = 1;
			break;
		GETOPT_DEFAULT:
			break;
		}
	}

	/* Check for undesired assignment. */
	if ((a == 1) || (b == 1))
		goto err0;

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/*
 * Error on Solaris 11.4 with gcc 7.3.0 -O1 and -O2.
 * If the clang workaround in util/getopt.h is disabled, also shows
 * an error on Debian buster-i386 with clang-7.0.1 -O1 and O2.
 */
static int
getopt_int_int_enum(int argc, char * argv[])
{
	/* Command-line arguments. */
	int a = 0;
	int b = 0;
	enum E E = ZERO;

	/* Working variable. */
	const char * ch;

	/* Parse with magic getopt. */
	while ((ch = GETOPT(argc, argv)) != NULL) {
		GETOPT_SWITCH(ch) {
		GETOPT_OPT("-a"):
			a = 1;
			break;
		GETOPT_OPT("-b"):
			b = 1;
			break;
		GETOPT_OPT("-E"):
			E = ONE;
			break;
		GETOPT_DEFAULT:
			break;
		}
	}

	/* Check for undesired assignment. */
	if ((a == 1) || (b == 1) || (E == ONE))
		goto err0;

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

int
main(int argc, char * argv[])
{
	int err = 0;

	/* Run tests; keep on going on failure. */
	if (getopt_int(argc, argv)) {
		fprintf(stderr, "Error in getopt_int\n");
		err++;
	}
	optreset = 1;
	if (getopt_int_int(argc, argv)) {
		fprintf(stderr, "Error in getopt_int_int\n");
		err++;
	}
	optreset = 1;
	if (getopt_int_int_enum(argc, argv)) {
		fprintf(stderr, "Error in getopt_int_int_enum\n");
		err++;
	}

	/* Did we fail anything? */
	if (err > 0)
		goto err0;

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

static void
usage(void)
{

	fprintf(stderr, "usage: test_getopt [-b] [-f bar] [--foo bar] ...\n");
	exit(1);
}

int
main(int argc, char * argv[])
{
	int argc_orig = argc;
	char ** argv_orig = argv;
	const char * ch;
	int bflag = 0;

	/* Process arguments. */
	while ((ch = GETOPT(argc, argv)) != NULL) {
		fprintf(stderr, "Option being processed: %s\n", ch);
		GETOPT_SWITCH(ch) {
		GETOPT_OPT("-b"):
		GETOPT_OPT("--bar"):
			bflag++;
			break;
		GETOPT_OPTARG("-f"):
		GETOPT_OPTARG("--foo"):
			fprintf(stderr, "foo: %s\n", optarg);
			break;
		GETOPT_DEFAULT:
			fprintf(stderr, "usage: blah blah blah\n");
		}
	}
	argc -= optind;
	argv += optind;

	/* Output some status. */
	fprintf(stderr, "bflag = %d\n", bflag);
	fprintf(stderr, "Remaining arguments:");
	while (argc > 0) {
		fprintf(stderr, " %s", *argv);
		argc--;
		argv++;
	}
	fprintf(stderr, "\n");

	/* We want to make sure that reset works. */
	optreset = 1;
	argc = argc_orig;
	argv = argv_orig;

	/* Process the arguments again, with GETOPT_MISSING_ARG this time. */
	while ((ch = GETOPT(argc, argv)) != NULL) {
		fprintf(stderr, "Option being processed: %s\n", ch);
		GETOPT_SWITCH(ch) {
		GETOPT_OPT("-b"):
		GETOPT_OPT("--bar"):
			bflag++;
			break;
		GETOPT_OPTARG("-f"):
		GETOPT_OPTARG("--foo"):
			fprintf(stderr, "foo: %s\n", optarg);
			break;
		GETOPT_MISSING_ARG:
			fprintf(stderr, "missing argument\n");
		GETOPT_DEFAULT:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
}

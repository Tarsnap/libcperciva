#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

static void
usage(void)
{

	fprintf(stderr, "usage: test_getopt [-b] [-f bar] [--foo bar] ...\n");
	exit(1);
}

static void
pass1(int argc, char * argv[])
{
	const char * ch;
	int bflag = 0;

	/* Process the arguments without GETOPT_MISSING_ARG. */
	while ((ch = GETOPT(argc, argv)) != NULL) {
		fprintf(stderr, "Option being processed: %s\n", ch);
		GETOPT_SWITCH(ch) {
		GETOPT_OPT("-b"):
			/* FALLTHROUGH */
		GETOPT_OPT("--bar"):
			bflag++;
			break;
		GETOPT_OPTARG("-f"):
			/* FALLTHROUGH */
		GETOPT_OPTARG("--foo"):
			fprintf(stderr, "foo: %s\n", optarg);
			break;
		GETOPT_DEFAULT:
			/*
			 * We can't call usage(), because that would exit(1).
			 * This test deliberately does not include
			 * GETOPT_MISSING_ARG, so it is likely that we will
			 * reach this point.
			 */
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

	/*
	 * Silence "value stored is never read" warnings; the adjustments to
	 * arg[cv] at the end of the argument-parsing loop are idiomatic.
	 */
	(void)argc;
	(void)argv;
}

static void
pass2(int argc, char * argv[])
{
	const char * ch;
	int bflag = 0;

	/* Process the arguments again, with GETOPT_MISSING_ARG this time. */
	while ((ch = GETOPT(argc, argv)) != NULL) {
		fprintf(stderr, "Option being processed: %s\n", ch);
		GETOPT_SWITCH(ch) {
		GETOPT_OPT("-b"):
			/* FALLTHROUGH */
		GETOPT_OPT("--bar"):
			bflag++;
			break;
		GETOPT_OPTARG("-f"):
			/* FALLTHROUGH */
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

	/*
	 * Silence "value stored is never read" warnings; the adjustments to
	 * arg[cv] at the end of the argument-parsing loop are idiomatic.
	 */
	(void)argc;
	(void)argv;

}

int
main(int argc, char * argv[])
{

	/* Process the arguments without GETOPT_MISSING_ARG. */
	pass1(argc, argv);

	/* Reset getopt state. */
	optreset = 1;

	/* Process the arguments again, with GETOPT_MISSING_ARG this time. */
	pass2(argc, argv);

	return (0);
}

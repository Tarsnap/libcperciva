#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

/* Work around a false positive warning from clang. */
#if defined(__clang__)

#define WARNING_UNINITIALIZED_SUPPRESS					\
_Pragma("clang diagnostic push")					\
_Pragma("clang diagnostic ignored \"-Wdeclaration-after-statement\"")	\
_Pragma("clang diagnostic ignored \"-Wconditional-uninitialized\"")
#define WARNING_UNINITIALIZED_ALLOW					\
_Pragma("clang diagnostic pop")

/* Do nothing. */
#else
#define WARNING_UNINITIALIZED_SUPPRESS
#define WARNING_UNINITIALIZED_ALLOW

#endif

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

	/* Process the arguments without GETOPT_MISSING_ARG. */
	while ((ch = GETOPT(argc, argv)) != NULL) {
		fprintf(stderr, "Option being processed: %s\n", ch);
		WARNING_UNINITIALIZED_SUPPRESS
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
			WARNING_UNINITIALIZED_ALLOW
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

	/* Reset getopt state. */
	optreset = 1;
	argc = argc_orig;
	argv = argv_orig;
	bflag = 0;

	/*
	 * Process the arguments again, with GETOPT_MISSING_ARG this time.
	 * This should be in the same function as before, to make sure
	 * that we're not (ab)using any function-local language constructs.
	 */
	while ((ch = GETOPT(argc, argv)) != NULL) {
		fprintf(stderr, "Option being processed: %s\n", ch);
		WARNING_UNINITIALIZED_SUPPRESS
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
			WARNING_UNINITIALIZED_ALLOW
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

	return (0);
}

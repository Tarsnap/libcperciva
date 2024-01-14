#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"
#include "parsenum.h"
#include "warnp.h"

#include "check.h"

enum modes {
	UNSET,
	CHECK_ORDER,
	CHECK_EXEC,
	CHECK_EXIT,
	PERFTEST
};

static void
usage(void)
{

	fprintf(stderr, "usage: test_fork_func [-c|-e|-x]\n");
	fprintf(stderr, "       test_fork_func -t -d CHILD_MS -n NUM_REPS\n");
	exit(1);
}

int
main(int argc, char ** argv)
{
	int child_ms = -1;
	int num_reps = -1;
	const char * ch;
	enum modes mode = UNSET;

	WARNP_INIT;

	/* Process arguments. */
	while ((ch = GETOPT(argc, argv)) != NULL) {
		GETOPT_SWITCH(ch) {
		GETOPT_OPT("-c"):
			mode = CHECK_ORDER;
			break;
		GETOPT_OPTARG("-d"):
			if (PARSENUM(&child_ms, optarg, 0, 10000)) {
				warnp("parsenum");
				goto err0;
			}
			break;
		GETOPT_OPT("-e"):
			mode = CHECK_EXEC;
			break;
		GETOPT_OPTARG("-n"):
			if (PARSENUM(&num_reps, optarg, 0, 10000)) {
				warnp("parsenum");
				goto err0;
			}
			break;
		GETOPT_OPT("-t"):
			mode = PERFTEST;
			break;
		GETOPT_OPT("-x"):
			mode = CHECK_EXIT;
			break;
		GETOPT_DEFAULT:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	/* We should have processed all the arguments. */
	if (argc != 0)
		usage();
	(void)argv; /* argv is not used beyond this point. */

	/* Run test. */
	switch (mode) {
	case CHECK_ORDER:
		/* Sanity test. */
		if ((child_ms != -1) || (num_reps != -1))
			usage();

		/* Run check. */
		if (check_order())
			goto err0;
		break;
	case CHECK_EXEC:
		/* Sanity test. */
		if ((child_ms != -1) || (num_reps != -1))
			usage();

		/* Run check. */
		if (check_exec())
			goto err0;
		break;
	case CHECK_EXIT:
		/* Sanity test. */
		if ((child_ms != -1) || (num_reps != -1))
			usage();

		/* Run check. */
		if (check_exit())
			goto err0;
		break;
	case PERFTEST:
		/* Sanity test. */
		if ((child_ms == -1) || (num_reps == -1))
			usage();

		/* Run perftest. */
		if (check_perftest(child_ms, num_reps))
			goto err0;
		break;
	case UNSET:
		usage();
	}

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

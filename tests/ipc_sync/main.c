#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "fork_func.h"
#include "ipc_sync.h"
#include "parsenum.h"
#include "warnp.h"

#define PRINT_PID(x) printf("%jd\t%s\n", (intmax_t)getpid(), (x))

static struct testcase {
	int wait_in_child;
	int prep_in_main;
	int prep_in_child;
} tests [] = {
	{ 0, 0, 0 },
	{ 0, 0, 1 },
	{ 0, 1, 0 },
	{ 0, 1, 1 },
	{ 1, 0, 0 },
	{ 1, 0, 1 },
	{ 1, 1, 0 },
	{ 1, 1, 1 },
};

struct child_info {
	int wait;
	int prep;
	struct ipc_sync * IS;
};

/* Either wait, or signal. */
static int
wait_or_signal(struct ipc_sync * ip, int which, int prep)
{

	/* Are we going to wait, or signal? */
	if (which) {
		/* If desired, prep for a wait. */
		if (prep) {
			PRINT_PID("_wait_prep");
			if (ipc_sync_wait_prep(ip))
				goto err0;
		}

		/* Wait. */
		PRINT_PID("_wait");
		if (ipc_sync_wait(ip))
			goto err0;
		PRINT_PID("finished _wait");
	} else {
		/* If desired, prep for a signal. */
		if (prep) {
			PRINT_PID("_signal_prep");
			if (ipc_sync_signal_prep(ip))
				goto err0;
		}

		/* Signal. */
		PRINT_PID("_signal");
		if (ipc_sync_signal(ip))
			goto err0;
		PRINT_PID("finished _signal");
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

static int
child(void * cookie)
{
	struct child_info * ci = cookie;

	PRINT_PID("post-fork child");

	/* Wait or signal, optionally with a prep. */
	if (wait_or_signal(ci->IS, ci->wait, ci->prep))
		goto err1;

	/* Clean up. */
	if (ipc_sync_done(ci->IS))
		goto err0;

	/* Ensure that we've finished writing to stdout. */
	if (fflush(stdout)) {
		warnp("fflush");
		goto err0;
	}

	/* Success! */
	return (0);

err1:
	if (ipc_sync_done(ci->IS))
		goto err0;
err0:
	/* Failure! */
	return (-1);
}

static int
check(const struct testcase * test)
{
	struct child_info CI_actual;
	struct child_info * CI = &CI_actual;
	struct ipc_sync * IS;
	pid_t pid;

	/* Initialize the synchronization. */
	if ((IS = ipc_sync_init()) == NULL)
		goto err0;

	/* Populate the child cookie. */
	CI->IS = IS;
	CI->wait = test->wait_in_child;
	CI->prep = test->prep_in_child;

	/* Fork. */
	if ((pid = fork_func(child, CI)) == -1)
		goto err0;
	PRINT_PID("post-fork parent");

	/* Wait or signal, optionally with a prep. */
	if (wait_or_signal(IS, !test->wait_in_child, test->prep_in_main))
		goto err0;

	/* Wait for the child to finish. */
	if (fork_func_wait(pid))
		goto err0;

	/* Clean up. */
	if (ipc_sync_done(IS))
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
	int num;

	WARNP_INIT;

	/* Parse command-line argument. */
	if (argc != 2) {
		fprintf(stderr, "usage: test_ipc_sync NUM\n");
		exit(1);
	}
	if (PARSENUM(&num, argv[1], 0, sizeof(tests) / sizeof(tests[0]) - 1)) {
		warnp("parsenum");
		goto err0;
	}

	/* Run the specified test. */
	if (check(&tests[num]))
		goto err0;

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

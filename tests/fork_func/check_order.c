#include <sys/types.h>

#include <stdio.h>

#include "fork_func.h"
#include "millisleep.h"
#include "warnp.h"

#include "check.h"

/**
 * Do a "sleep sort".  This isn't guaranteed to sort successfully, but it's
 * very unlikely that any processes will be delayed by enough for it to fail.
 *
 * We want to output two different lists: the "spawning" messages, and the
 * "waited" messages.  This allows the test to be more resistant to systems
 * having different overheads for fork().
 *
 * To achieve this, the "spawning" messages go to stdout, while the "waited"
 * messages go to stderr.
 */

#define NUM_PROCESSES 5
static int SLEEP_SORT_MS[5] = {200, 800, 0, 600, 400};

/* Wait X milliseconds, then print to stderr. */
static int
sleep_print_func(void * cookie)
{
	int ms = *((int *)cookie);

	/* Wait. */
	millisleep((size_t)ms);

	/* Write the "waited" messages to stderr. */
	if (fprintf(stderr, "waited %i ms\n", ms) < 0) {
		warnp("fprintf");
		goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/**
 * check_order(void):
 * Check that fork_func() doesn't impose any order on multiple functions.
 */
int
check_order(void)
{
	pid_t pids[NUM_PROCESSES];
	int i;

	/*-
	 * We want to output two different lists: the "spawning"
	 * messages, and the "waited" messages.  This allows the test
	 * to be more resistant to systems having different overheads
	 * for fork().
	 *
	 * To achieve this, the "spawning" messages go to stdout,
	 * while the "waited" messages go to stderr.
	 */

	/* Write the "spawning" messages to stdout. */
	for (i = 0; i < NUM_PROCESSES; i++) {
		if (fprintf(stdout, "spawning a process to wait %i ms\n",
		    SLEEP_SORT_MS[i]) < 0) {
			warnp("fprintf");
			goto err0;
		}
	}

	/* Flush stdout so that its buffer isn't duplicated in the forks. */
	if (fflush(stdout)) {
		warnp("fflush");
		goto err0;
	}

	/* Spawn the processes. */
	for (i = 0; i < NUM_PROCESSES; i++) {
		if ((pids[i] = fork_func(sleep_print_func,
		    &SLEEP_SORT_MS[i])) == -1)
			goto err0;
	}

	/* Wait for the processes to finish. */
	for (i = 0; i < NUM_PROCESSES; i++) {
		if (fork_func_wait(pids[i]))
			goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

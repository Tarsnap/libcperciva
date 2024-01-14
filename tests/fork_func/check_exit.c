#include <sys/types.h>

#include "fork_func.h"

#include "check.h"

static int
func_exit(void * cookie)
{
	int * exitcode_p = (int *)cookie;

	/* Return the specified exit code. */
	return (*exitcode_p);
}

/**
 * check_exit(void):
 * Check that fork_func() works with exit codes.
 */
int
check_exit(void)
{
	pid_t pid;
	int i;
	int rc;

	for (i = 0; i < 4; i++) {
		/* Fork. */
		if ((pid = fork_func(func_exit, &i)) == -1)
			goto err0;

		/* Check the result. */
		if ((rc = fork_func_wait(pid)) == -1)
			goto err0;
		if (rc != i)
			goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

#include <sys/types.h>

#include <unistd.h>

#include "fork_func.h"
#include "warnp.h"

#include "check.h"

static int
func_exec(void * cookie)
{

	(void)cookie; /* UNUSED */

	/* Execute the "true" binary found in the $PATH. */
	if (execlp("true", "true", NULL))
		warnp("execvp");

	/* We should never reach this. */
	return (127);
}

/**
 * check_exec(void):
 * Check that fork_func() works with an exec* function.
 */
int
check_exec(void)
{
	pid_t pid;

	/* Fork. */
	if ((pid = fork_func(func_exec, NULL)) == -1)
		goto err0;

	/* Check that it didn't fail. */
	if (fork_func_wait(pid) != 0)
		goto err0;

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

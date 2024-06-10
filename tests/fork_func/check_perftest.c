#include <sys/types.h>

#include <sys/time.h>

#include <pthread.h>
#include <stdio.h>

#include "fork_func.h"
#include "millisleep.h"
#include "monoclock.h"
#include "warnp.h"

#include "check.h"

/* Wait X milliseconds. */
static int
millisleep_func(void * cookie)
{
	int ms = *((int *)cookie);

	/* Wait. */
	millisleep((size_t)ms);

	/* Success! */
	return (0);
}

static void *
millisleep_func_pthread(void * cookie)
{
	int ms = *((int *)cookie);

	/* Wait. */
	millisleep((size_t)ms);

	return (NULL);
}

static inline void
print_timediff(int type, struct timeval tv_orig, struct timeval tv_now, int ms)
{

	printf("%d\t%.4f\n", type,
	    1000.0 * timeval_diff(tv_orig, tv_now) - (double)ms);
}

static int
benchmark_only_millisleep(int ms)
{
	struct timeval tv_orig, tv_now;

	/* Get the original time. */
	if (monoclock_get(&tv_orig)) {
		warnp("monoclock_get");
		goto err0;
	}

	/* Wait. */
	millisleep_func(&ms);

	/* Get the current time, and print the difference. */
	if (monoclock_get(&tv_now)) {
		warnp("monoclock_get");
		goto err0;
	}
	print_timediff(0, tv_orig, tv_now, ms);

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

static int
benchmark_pthread_millisleep(int ms)
{
	struct timeval tv_orig, tv_now;
	pthread_t thr;

	/* Get the original time. */
	if (monoclock_get(&tv_orig)) {
		warnp("monoclock_get");
		goto err0;
	}

	/* Create a new thread and wait for it to finish. */
	if (pthread_create(&thr, NULL, millisleep_func_pthread, &ms)) {
		warn0("pthread");
		goto err0;
	}
	if (pthread_join(thr, NULL)) {
		warn0("pthread_join");
		goto err0;
	}

	/* Get the current time, and print the difference. */
	if (monoclock_get(&tv_now)) {
		warnp("monoclock_get");
		goto err0;
	}
	print_timediff(1, tv_orig, tv_now, ms);

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

static int
benchmark_fork_func_millisleep(int ms)
{
	struct timeval tv_orig, tv_now;
	pid_t pid;

	/* Get the original time. */
	if (monoclock_get(&tv_orig)) {
		warnp("monoclock_get");
		goto err0;
	}

	/* Create a new process, and wait for it to finish. */
	if ((pid = fork_func(millisleep_func, &ms)) == -1) {
		warn0("fork_func");
		goto err0;
	}
	if (fork_func_wait(pid))
		goto err0;

	/* Get the current time, and print the difference. */
	if (monoclock_get(&tv_now)) {
		warnp("monoclock_get");
		goto err0;
	}
	print_timediff(2, tv_orig, tv_now, ms);

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/**
 * check_perftest(child_ms, num_reps):
 * Check the amount of delay added by func_fork() and func_fork_wait().
 */
int
check_perftest(int child_ms, int num_reps)
{
	int i;

	/* Explain columns. */
	printf("# type\textra_delay\n");

	/* Baseline: time the delay without any threads or processes. */
	for (i = 0; i < num_reps; i++)
		if (benchmark_only_millisleep(child_ms))
			goto err0;

	/* Time the delay with pthread. */
	for (i = 0; i < num_reps; i++)
		if (benchmark_pthread_millisleep(child_ms))
			goto err0;

	/* Time the delay with fork_func. */
	for (i = 0; i < num_reps; i++)
		if (benchmark_fork_func_millisleep(child_ms))
			goto err0;

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

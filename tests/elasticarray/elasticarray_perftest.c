#include <sys/time.h>

#include <limits.h>
#include <stdio.h>

#include "elasticarray.h"
#include "monoclock.h"
#include "warnp.h"

#include "elasticarray_perftest.h"

ELASTICARRAY_DECL(INTLIST, intlist, int);

#define REPS 1000000	/* 1 million. */

#define BENCH_START						\
	if (monoclock_get(&tv0)) {				\
		warnp("monoclock");				\
		goto err1;					\
	}							\
	for (i = 0; i < REPS; i++)

#define BENCH_END(delta_ns)					\
	if (monoclock_get(&tv1)) {				\
		warnp("monoclock");				\
		goto err1;					\
	}							\
	(delta_ns) = timeval_diff(tv0, tv1) * 1e9 / (double)REPS;

static int
check_append_shrink(void)
{
	INTLIST list;
	struct timeval tv0, tv1;
	double delta_append, delta_shrink, delta_combo;
	int i;

	/* Allocate list. */
	if ((list = intlist_init(0)) == NULL)
		goto err0;

	/* Append some values. */
	BENCH_START {
		if (intlist_append(list, &i, 1))
			goto err1;
	}
	BENCH_END(delta_append)

	/* Remove some values. */
	BENCH_START {
		intlist_shrink(list, 1);
	}
	BENCH_END(delta_shrink)

	/* Append and remove. */
	BENCH_START {
		if (intlist_append(list, &i, 1))
			goto err1;
		intlist_shrink(list, 1);
	}
	BENCH_END(delta_combo)

	/* Clean up. */
	intlist_free(list);

	/* Print timing results. */
	printf("# _append\t_shrink\t_append and _shrink (all in ns)\n");
	printf("%.1f\t%.1f\t%.1f\n", delta_append, delta_shrink, delta_combo);

	/* Success! */
	return (0);

err1:
	intlist_free(list);
err0:
	/* Failure! */
	return (-1);
}

/**
 * elasticarray_peftest(void):
 * Benchmark various elasticarray operations.
 */
int
elasticarray_perftest(void)
{

	/* Benchmark elasticarray_append() and elasticarray_shrink(). */
	if (check_append_shrink())
		goto err0;

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

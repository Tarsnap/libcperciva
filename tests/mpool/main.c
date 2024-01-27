#include <sys/time.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "monoclock.h"
#include "mpool.h"
#include "parsenum.h"
#include "warnp.h"

#define INITIAL_POOL 100

struct stuff {
	int i;
};

MPOOL(stuff, struct stuff, INITIAL_POOL);

/* Function prototypes. */
int check_mem(volatile struct stuff *);
long long time_func(unsigned long sets, unsigned long reps,
    unsigned int use_mpool);

/* Check that we can write & read to that memory (mainly for valgrind). */
int
check_mem(volatile struct stuff * vol)
{

	/* This is `volatile`, so it shouldn't be optimized out. */
	vol->i = 0;
	if (vol->i != 0)
		goto err0;

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/* Time the allocation and freeing using malloc or mpool. */
long long
time_func(unsigned long sets, unsigned long reps, unsigned int use_mpool)
{
	struct timeval begin, end;
	long long delta_us;
	unsigned long i, j;
	struct stuff ** arr;

	/* Sanity test. */
	assert((sets > 0) && (reps > 0));

	/* Allocate temporary array. */
	if ((arr = malloc(reps * sizeof(struct stuff *))) == NULL) {
		warnp("Out of memory");
		goto err0;
	}

	/* Get start time. */
	if (monoclock_get_cputime(&begin)) {
		warnp("monoclock_get_cputime()");
		goto err1;
	}

	for (j = 0; j < sets; j++) {
		/* Allocate structures. */
		if (use_mpool) {
			for (i = 0; i < reps; i++) {
				if ((arr[i] = mpool_stuff_malloc()) == NULL) {
					warnp("mpool_stuff_malloc()");
					goto err1;
				}
			}
		} else {
			for (i = 0; i < reps; i++) {
				if ((arr[i] = malloc(sizeof(struct stuff)))
				    == NULL) {
					warnp("malloc()");
					goto err1;
				}
			}
		}

		/*
		 * Free one structure to potentially trigger mpool growth,
		 * then test memory access.
		 */
		if (use_mpool)
			mpool_stuff_free(arr[0]);
		else
			free(arr[0]);
		for (i = 1; i < reps; i++)
			if (check_mem(arr[i]))
				goto err1;

		/* Free remaining structures. */
		if (use_mpool)
			for (i = 1; i < reps; i++)
				mpool_stuff_free(arr[i]);
		else
			for (i = 1; i < reps; i++)
				free(arr[i]);
	}

	/* Get end time and difference. */
	if (monoclock_get_cputime(&end)) {
		warnp("monoclock_get_cputime()");
		goto err1;
	}
	delta_us = (long long)(1e6 * timeval_diff(begin, end));

	/* Clean up. */
	free(arr);

	/* Success! */
	return (delta_us);

err1:
	free(arr);
err0:
	/* Failure! */
	return (-1);
}

int
main(int argc, char * argv[])
{
	unsigned long sets, reps;
	unsigned int use_mpool;
	long long delta_us;

	WARNP_INIT;

	/* Parse args. */
	if ((argc != 4) || (PARSENUM(&sets, argv[1])) ||
	    (PARSENUM(&reps, argv[2])) || (PARSENUM(&use_mpool, argv[3]))) {
		printf("usage: test_mpool SETS REPS TYPE\n");
		goto err0;
	}

	/* Sanity check. */
	if (!((sets > 0) && (reps > 0))) {
		fprintf(stderr, "SETS and REPS must be greater than zero\n");
		goto err0;
	}

	/* Time memory allocation method, and output it. */
	if ((delta_us = time_func(sets, reps, use_mpool)) < 0)
		goto err0;
	printf("%lld\n", delta_us);

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

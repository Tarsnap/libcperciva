#include <stdio.h>
#include <stdlib.h>

#include "elasticarray.h"
#include "getopt.h"
#include "warnp.h"

#include "elasticarray_perftest.h"

#define NUM_ELEM 2

struct dyn {
	int * arr;
};

ELASTICARRAY_DECL(INTLIST, intlist, int);
ELASTICARRAY_DECL(DYNLIST, dynlist, struct dyn);
ELASTICARRAY_DECL(POINTERLIST, pointerlist, struct dyn *);

/* Elasticarray with simple integers. */
static int
test_intlist(void)
{
	INTLIST list;
	size_t i;
	int x;

	/* Allocate list. */
	if ((list = intlist_init(0)) == NULL)
		goto err0;

	/* Append some values. */
	for (i = 0; i < NUM_ELEM; i++) {
		x = (int)i;
		if (intlist_append(list, &x, 1))
			goto err1;
	}

	/* Use the list. */
	for (i = 0; i < intlist_getsize(list); i++) {
		x = *intlist_get(list, i);
		printf("int:\t%i\n", x);
	}

	/* Free memory. */
	intlist_free(list);

	/* Success! */
	return (0);

err1:
	intlist_free(list);
err0:
	/* Failure! */
	return (-1);
}

/* Free a pointer to 'struct dyn'. */
static void
free_dyn(struct dyn * d)
{

	free(d->arr);
}

/* Elasticarray with structures containing dynamically-allocated values. */
static int
test_dynlist(void)
{
	DYNLIST list;
	struct dyn d;
	size_t i;

	/* Allocate list. */
	if ((list = dynlist_init(0)) == NULL)
		goto err0;

	/* Append some values. */
	for (i = 0; i < NUM_ELEM; i++) {
		/* Dynamically allocate part of structure. */
		if ((d.arr = malloc(sizeof(int))) == NULL)
			goto err1;
		d.arr[0] = (int)i;

		/* Append item. */
		if (dynlist_append(list, &d, 1))
			goto err2;
	}

	/* Use the list. */
	for (i = 0; i < dynlist_getsize(list); i++) {
		d = *dynlist_get(list, i);
		printf("dyn:\t%i\n", d.arr[0]);
	}

	/* Free memory. */
	dynlist_iter(list, free_dyn);
	dynlist_free(list);

	/* Success! */
	return (0);

err2:
	free(d.arr);
err1:
	dynlist_iter(list, free_dyn);
	dynlist_free(list);
err0:
	/* Failure! */
	return (-1);
}

/* Free a pointer to a pointer to 'struct dyn'. */
static void
free_p_dyn(struct dyn ** dp)
{

	free((*dp)->arr);
	free(*dp);
}

/* Elasticarray with pointers. */
static int
test_pointerlist(void)
{
	POINTERLIST list;
	struct dyn * p;
	size_t i;

	/* Allocate list. */
	if ((list = pointerlist_init(0)) == NULL)
		goto err0;

	/* Append some values. */
	for (i = 0; i < NUM_ELEM; i++) {
		/* Dynamically allocate item to append. */
		if ((p = malloc(sizeof(struct dyn))) == NULL)
			goto err1;
		if ((p->arr = malloc(sizeof(int))) == NULL)
			goto err2;
		p->arr[0] = (int)i;

		/* Append pointer to structure. */
		if (pointerlist_append(list, &p, 1))
			goto err3;
	}

	/* Use the list. */
	for (i = 0; i < pointerlist_getsize(list); i++) {
		p = *pointerlist_get(list, i);
		printf("pointer:\t%i\n", p->arr[0]);
	}

	/* Free memory. */
	pointerlist_iter(list, free_p_dyn);
	pointerlist_free(list);

	/* Success! */
	return (0);

err3:
	free(p->arr);
err2:
	free(p);
err1:
	pointerlist_iter(list, free_p_dyn);
	pointerlist_free(list);
err0:
	/* Failure! */
	return (-1);
}

/* Elasticarray with size 0. */
static int
test_emptylist(void)
{
	INTLIST list;
	int x;		/* Not set to a value. */

	/* Allocate list and check size. */
	if ((list = intlist_init(0)) == NULL)
		goto err0;
	if (intlist_getsize(list) != 0)
		goto err1;

	/* Resize to 0 (should be a no-op) and check size. */
	if (intlist_resize(list, 0))
		goto err1;
	if (intlist_getsize(list) != 0)
		goto err1;

	/* Append nothing.  The value of x should not be read.  Check size. */
	if (intlist_append(list, &x, 0))
		goto err1;
	if (intlist_getsize(list) != 0)
		goto err1;

	/* Free memory. */
	intlist_free(list);

	/* Success! */
	return (0);

err1:
	intlist_free(list);
err0:
	/* Failure! */
	return (-1);
}

static int
selftest(void)
{

	/* Elasticarray with simple integers. */
	if (test_intlist())
		goto err0;

	/* Elasticarray with dynamically-allocated structures. */
	if (test_dynlist())
		goto err0;

	/* Elasticarray with pointers. */
	if (test_pointerlist())
		goto err0;

	/* Elasticarray with size 0. */
	if (test_emptylist())
		goto err0;

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

static void
usage(void)
{

	fprintf(stderr, "usage: test_elasticarray -x\n");
	fprintf(stderr, "       test_elasticarray -t\n");
	exit(1);
}

int
main(int argc, char * argv[])
{
	const char * ch;

	WARNP_INIT;

	/* Process arguments. */
	while ((ch = GETOPT(argc, argv)) != NULL) {
		GETOPT_SWITCH(ch) {
		GETOPT_OPT("-x"):
			exit(selftest());
		GETOPT_OPT("-t"):
			exit(elasticarray_perftest());
		GETOPT_DEFAULT:
			usage();
		}
	}

	usage();
}

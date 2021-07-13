#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"
#include "perftest.h"
#include "sysendian.h"
#include "warnp.h"

/* Test cases. */
static struct testcase16 {
	uint16_t val;
	uint8_t be_arr[2];
	uint8_t le_arr[2];
} tests16[] = {
	{ 0x0001, {0, 1}, {1, 0} },
	{ 0x0100, {1, 0}, {0, 1} }
};

static struct testcase32 {
	uint32_t val;
	uint8_t be_arr[4];
	uint8_t le_arr[4];
} tests32[] = {
	{ 0x00000001, {0, 0, 0, 1}, {1, 0, 0, 0} },
	{ 0x00000100, {0, 0, 1, 0}, {0, 1, 0, 0} },
	{ 0x00010000, {0, 1, 0, 0}, {0, 0, 1, 0} },
	{ 0x01000000, {1, 0, 0, 0}, {0, 0, 0, 1} }
};

static struct testcase64 {
	uint64_t val;
	uint8_t be_arr[8];
	uint8_t le_arr[8];
} tests64[] = {
	{ 0x0000000000000001, {0, 0, 0, 0, 0, 0, 0, 1},
	    {1, 0, 0, 0, 0, 0, 0, 0} },
	{ 0x0000000000000100, {0, 0, 0, 0, 0, 0, 1, 0},
	    {0, 1, 0, 0, 0, 0, 0, 0} },
	{ 0x0000000000010000, {0, 0, 0, 0, 0, 1, 0, 0},
	    {0, 0, 1, 0, 0, 0, 0, 0} },
	{ 0x0000000001000000, {0, 0, 0, 0, 1, 0, 0, 0},
	    {0, 0, 0, 1, 0, 0, 0, 0} },
	{ 0x0000000100000000, {0, 0, 0, 1, 0, 0, 0, 0},
	    {0, 0, 0, 0, 1, 0, 0, 0} },
	{ 0x0000010000000000, {0, 0, 1, 0, 0, 0, 0, 0},
	    {0, 0, 0, 0, 0, 1, 0, 0} },
	{ 0x0001000000000000, {0, 1, 0, 0, 0, 0, 0, 0},
	    {0, 0, 0, 0, 0, 0, 1, 0} },
	{ 0x0100000000000000, {1, 0, 0, 0, 0, 0, 0, 0},
	    {0, 0, 0, 0, 0, 0, 0, 1} },
};

struct tests {
	volatile uint16_t val16[2];
	volatile uint16_t val16_be[2];
	volatile uint16_t val16_le[2];
	volatile uint32_t val32[4];
	volatile uint32_t val32_be[4];
	volatile uint32_t val32_le[4];
	volatile uint64_t val64[8];
	volatile uint64_t val64_be[8];
	volatile uint64_t val64_le[8];
};

/* Performance tests. */
static const size_t perfsizes[] = {32, 64}; /* Bits. */
static const size_t num_perf = sizeof(perfsizes) / sizeof(perfsizes[0]);
static const size_t nbytes_perftest = 100000000;	/* 1 GB */
static const size_t nbytes_warmup = 100000000;		/* 1 GB */

static void
fill_testcases(struct tests * tests)
{
	uint16_t val16;
	uint32_t val32;
	uint64_t val64;
	size_t i;

	/* Load 16-bit tests. */
	for (i = 0; i < 2; i++) {
		tests->val16[i] = tests16[i].val;
		memcpy(&val16, tests16[i].be_arr, 2);
		tests->val16_be[i] = val16;
		memcpy(&val16, tests16[i].le_arr, 2);
		tests->val16_le[i] = val16;
	}

	/* Load 32-bit tests. */
	for (i = 0; i < 4; i++) {
		tests->val32[i] = tests32[i].val;
		memcpy(&val32, tests32[i].be_arr, 4);
		tests->val32_be[i] = val32;
		memcpy(&val32, tests32[i].le_arr, 4);
		tests->val32_le[i] = val32;
	}

	/* Load 64-bit tests. */
	for (i = 0; i < 8; i++) {
		tests->val64[i] = tests64[i].val;
		memcpy(&val64, tests64[i].be_arr, 8);
		tests->val64_be[i] = val64;
		memcpy(&val64, tests64[i].le_arr, 8);
		tests->val64_le[i] = val64;
	}
}

/**
 * PERFTEST_FULL(tests, N):
 * Do all endian conversions for $N$ bits, without checking for accuracy.
 *
 * Implementation note: Using the volatile variables $v_val$ and $tests$
 * prevents the compiler from generating the answers at compile-time.
 */
#define PERFTEST_FULL(tests, N) do {				\
	volatile uint ## N ## _t v_val;				\
	uint ## N ## _t val;					\
	uint8_t arr[N/8];					\
	size_t i;						\
								\
	for (i = 0; i < N/8; i++) {				\
		be ## N ## enc(arr, tests->val ## N [i]);	\
								\
		le ## N ## enc(arr, tests->val ## N [i]);	\
								\
		val = tests->val ## N ## _be[i];		\
		v_val = be ## N ## dec(&val);			\
								\
		val = tests->val ## N ## _le[i];		\
		v_val = le ## N ## dec(&val);			\
								\
		(void)v_val;					\
	}							\
} while (0)

static int
perftest_func(void * cookie, uint8_t * buf, size_t buflen, size_t nreps)
{
	struct tests * tests = cookie;
	size_t nreps_i;

	(void)buf; /* UNUSED */

	if (buflen == 32) {
		for (nreps_i = 0; nreps_i < nreps; nreps_i++)
			PERFTEST_FULL(tests, 32);
	} else if (buflen == 64) {
		for (nreps_i = 0; nreps_i < nreps; nreps_i++)
			PERFTEST_FULL(tests, 64);
	}

	return (0);
}

static int
perftest(void)
{
	struct tests tests_actual;
	struct tests * tests = &tests_actual;

	/* Copy the tests into the volatile variables. */
	fill_testcases(tests);

	/* Time the function. */
	if (perftest_buffers(nbytes_perftest, perfsizes, num_perf,
	    nbytes_warmup, 1, NULL, perftest_func, NULL, tests)) {
		warn0("perftest_buffers");
		goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (1);
}

/* Print array as a series of 8-bit hex values. */
static void
print_arr(const char * str, const uint8_t * arr, size_t n)
{
	size_t i;

	printf("%s", str);
	for (i = 0; i < n; i++)
		printf("%02x ", arr[i]);
	printf("\n");
}

/**
 * TEST_FULL(tests, N):
 * Do all endian conversions for $N$ bits and check for accuracy.
 */
#define TEST_FULL(tests, N) do {				\
	volatile uint ## N ## _t v_val;				\
	uint ## N ## _t val;					\
	uint8_t arr[N/8];					\
	size_t i;						\
								\
	for (i = 0; i < N/8; i++) {				\
		be ## N ## enc(arr, tests->val ## N [i]);	\
		if (memcmp(arr, tests ## N [i].be_arr, N/8)) {	\
			warnp("Failed be" # N "enc test %zu", i); \
			print_arr("Computed:\t", arr, N/8);	\
			print_arr("Correct:\t",			\
			    tests ## N [i].be_arr, N/8);	\
			goto err0;				\
		}						\
								\
		le ## N ## enc(arr, tests->val ## N [i]);	\
		if (memcmp(arr, tests ## N [i].le_arr, N/8)) {	\
			warnp("Failed le" # N "enc test %zu", i); \
			print_arr("Computed:\t", arr, N/8);	\
			print_arr("Correct:\t",			\
			    tests ## N [i].le_arr, N/8);	\
			goto err0;				\
		}						\
								\
		val = tests->val ## N ## _be[i];		\
		v_val = be ## N ## dec(&val);			\
		if (v_val != tests->val ## N [i]) {		\
			warnp("Failed be" #N "dec test %zu", i); \
			printf("Computed:\t%" PRIu ## N "\n",	\
			    v_val);				\
			printf("Correct:\t%" PRIu ## N "\n",	\
			    tests->val ## N [i]);		\
			goto err0;				\
		}						\
								\
		val = tests->val ## N ## _le[i];		\
		v_val = le ## N ## dec(&val);			\
		if (v_val != tests->val ## N [i]) {		\
			warnp("Failed le" #N "dec test %zu", i); \
			printf("Computed:\t%" PRIu ## N "\n",	\
			    v_val);				\
			printf("Correct:\t%" PRIu ## N "\n",	\
			    tests->val ## N [i]);		\
			goto err0;				\
		}						\
	}							\
} while (0)

static int
selftest(void)
{
	struct tests tests_actual;
	struct tests * tests = &tests_actual;

	/* Copy the tests into the volatile variables. */
	fill_testcases(tests);

	TEST_FULL(tests, 16);
	TEST_FULL(tests, 32);
	TEST_FULL(tests, 64);

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (1);
}

static void
usage(void)
{

	fprintf(stderr, "usage: test_sysendian -t\n");
	fprintf(stderr, "       test_sysendian -x\n");
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
		GETOPT_OPT("-t"):
			exit(perftest());
		GETOPT_OPT("-x"):
			exit(selftest());
		GETOPT_DEFAULT:
			usage();
		}
	}

	usage();
}

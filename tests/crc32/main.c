#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"
#include "monoclock.h"
#include "warnp.h"

#include "crc32c.h"

static struct testcase {
	const char * s;
	uint8_t cbuf[4];
} tests[] = {
	{ "", {0x78, 0x3b, 0xf6, 0x82}},
	{ " ", {0x27, 0x74, 0x7e, 0xdb}},
	{ "A", {0x46, 0x64, 0xd3, 0x48}},
	{ "AAAA", {0x68, 0xf2, 0xc0, 0x25}},
	{ "AB", {0x7b, 0x44, 0xd2, 0xc7}},
	{ "hello", {0xaf, 0x7a, 0x0b, 0xc3}},
	{ "hello world", {0xca, 0x13, 0x0b, 0xaa}},
	{ "This is a CRC32 hash using the Catagnoli polynomial",
	    {0x1b, 0xc4, 0xb4, 0x28}}
};

/* Largest buffer must be first. */
static const size_t perfsizes[] = {16384, 8192, 4096, 2048, 1024, 512, 256,
    128, 64, 32, 16};
static const size_t num_perf = sizeof(perfsizes) / sizeof(perfsizes[0]);
static const size_t bytes_to_hash = 1 << 29;	/* approx 500 Mb */

static int
perftest()
{
	CRC32C_CTX ctx;
	uint8_t * largebuf;
	uint8_t cbuf[4];
	struct timeval begin, end;
	long long delta;
	size_t i, j;
	size_t num_hashes;

	/* Allocate buffer to hold largest message. */
	if ((largebuf = malloc(perfsizes[0])) == NULL) {
		warnp("malloc");
		goto err0;
	}
	memset(largebuf, 0, perfsizes[0]);

	/* Inform user. */
	printf("Hashing %.03g bytes...\n", (double)bytes_to_hash);

	/* Warm up. */
	for (j = 0; j < 8000; j++) {
		CRC32C_Init(&ctx);
		CRC32C_Update(&ctx, largebuf, perfsizes[0]);
		CRC32C_Final(cbuf, &ctx);
	}

	/* Run operations. */
	for (i = 0; i < num_perf; i++) {
		num_hashes = bytes_to_hash / perfsizes[i];

		/* Get beginning time. */
		if (monoclock_get(&begin)) {
			warnp("monoclock_get()");
			goto err1;
		}

		/* Hash all the bytes. */
		for (j = 0; j < num_hashes; j++) {
			CRC32C_Init(&ctx);
			CRC32C_Update(&ctx, largebuf, perfsizes[i]);
			CRC32C_Final(cbuf, &ctx);
		}

		/* Get ending time. */
		if (monoclock_get(&end)) {
			warnp("monoclock_get()");
			goto err1;
		}

		/* Find and print elasped time. */
		delta = 1000000*((long long)(end.tv_sec - begin.tv_sec)) +
		    (end.tv_usec - begin.tv_usec);
		printf("... in %zu blocks of size %zu:\t%.02f s\n",
		    num_hashes, perfsizes[i], (double)delta / 1000000);
	}

	/* Clean up. */
	free(largebuf);

	/* Success! */
	return (0);

err1:
	free(largebuf);
err0:
	/* Failure! */
	return (1);
}

static int
selftest(void)
{
	CRC32C_CTX ctx;
	uint8_t cbuf[4];
	size_t i, j;
	size_t failures = 0;

	for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
		printf("Computing CRC32C of \"%s\"...", tests[i].s);
		CRC32C_Init(&ctx);
		CRC32C_Update(&ctx, (const uint8_t *)tests[i].s,
		    strlen(tests[i].s));
		CRC32C_Final(cbuf, &ctx);
		if (memcmp(cbuf, tests[i].cbuf, 4)) {
			printf(" FAILED!\n");
			printf("Computed CRC32C: ");
			for (j = 0; j < 4; j++)
				printf("%02x", cbuf[j]);
			printf("\nCorrect CRC32C:  ");
			for (j = 0; j < 4; j++)
				printf("%02x", tests[i].cbuf[j]);
			printf("\n");
			failures++;
		} else {
			printf(" PASSED!\n");
		}
	}

	if (failures)
		return (1);
	else
		return (0);
}

static void
usage(void)
{

	fprintf(stderr, "usage: test_crc32 -t\n");
	fprintf(stderr, "       test_crc32 -x\n");
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

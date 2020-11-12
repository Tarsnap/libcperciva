#include <sys/time.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpusupport.h"
#include "getopt.h"
#include "perftest.h"
#include "warnp.h"

#include "crc32c.h"

#define LARGE_BUFSIZE 65536
#define MAX_CHUNK 256

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

/* Performance tests. */
static const size_t perfsizes[] = {16384, 8192, 4096, 2048, 1024, 512, 256,
    128, 64, 32, 16};
static const size_t num_perf = sizeof(perfsizes) / sizeof(perfsizes[0]);
static const size_t nbytes_perftest = 1 << 29;		/* approx 500 MB */
static const size_t nbytes_warmup = 16384 * 8000;	/* approx 131 MB */

/* Print a string, then whether or not we're using hardware instructions. */
static void
print_hardware(const char * str)
{
	int use_hardware = 0;

#ifdef CPUSUPPORT_X86_CRC32_64
	if (cpusupport_x86_crc32_64())
		use_hardware = 1;
#endif

	/* Inform the user. */
	printf("%s", str);
	if (use_hardware)
		printf(" using hardware CRC32.\n");
	else
		printf(" using software CRC32.\n");
}

static int
perftest_init(void * cookie, uint8_t * buf, size_t buflen)
{

	(void)cookie;	/* UNUSED */

	/* Clear buffer. */
	memset(buf, 0, buflen);

	/* Success! */
	return (0);
}

static int
perftest_func(void * cookie, uint8_t * buf, size_t buflen, size_t nreps)
{
	CRC32C_CTX ctx;
	uint8_t cbuf[4];
	size_t i;

	(void)cookie; /* UNUSED */

	/* Do the hashing. */
	for (i = 0; i < nreps; i++) {
		CRC32C_Init(&ctx);
		CRC32C_Update(&ctx, buf, buflen);
		CRC32C_Final(cbuf, &ctx);
	}

	/* Success! */
	return (0);
}

static int
perftest(void)
{

	/* Inform user. */
	print_hardware("CRC32C time trial");
	printf("Hashing %zu bytes.\n", nbytes_perftest);
	fflush(stdout);

	/* Time the function. */
	if (perftest_buffers(nbytes_perftest, perfsizes, num_perf,
	    nbytes_warmup, 1, perftest_init, perftest_func, NULL)) {
		warn0("perftest_buffers");
		goto err0;
	}

	/* Success! */
	return (0);

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
	char * largebuf;
	size_t bytes_processed;
	size_t new_chunk;
	uint8_t alt_buf[4];

	/* Run regular test cases. */
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

	/* Test with a large buffer and unaligned access. */
	printf("Computing CRC32C of a large buffer two different ways...");

	/* Prepare a large buffer with repeating 01010101_2 = 85. */
	if ((largebuf = malloc(LARGE_BUFSIZE)) == NULL)
		goto err0;
	memset(largebuf, 85, LARGE_BUFSIZE);

	/* Compute checksum with one call. */
	CRC32C_Init(&ctx);
	CRC32C_Update(&ctx, (const uint8_t *)largebuf, LARGE_BUFSIZE);
	CRC32C_Final(cbuf, &ctx);

	/* Ensure we have a repeatable pattern of random values. */
	srandom(0);

	/* Compute checksum with multiple calls. */
	CRC32C_Init(&ctx);
	bytes_processed = 0;
	while (bytes_processed < LARGE_BUFSIZE - MAX_CHUNK) {
		new_chunk = ((unsigned long int)random()) % MAX_CHUNK;
		CRC32C_Update(&ctx, (const uint8_t *)&largebuf[bytes_processed],
		    new_chunk);
		bytes_processed += new_chunk;
	}
	new_chunk = LARGE_BUFSIZE - bytes_processed;
	CRC32C_Update(&ctx, (const uint8_t *)&largebuf[bytes_processed],
	    new_chunk);
	CRC32C_Final(alt_buf, &ctx);

	/* Compare checksums. */
	if (memcmp(cbuf, alt_buf, 4)) {
		printf(" FAILED!\n");
		failures++;
	} else
		printf(" PASSED!\n");

	/* Clean up. */
	free(largebuf);

	/* Report overall success to exit code. */
	if (failures)
		return (1);
	else
		return (0);

err0:
	/* Failure! */
	return (1);
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

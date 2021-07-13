#include <sys/time.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"
#include "hexify.h"
#include "monoclock.h"
#include "perftest.h"
#include "sha1.h"
#include "warnp.h"

/* Performance tests. */
static const size_t perfsizes[] = {256, 1024, 10000};
static const size_t num_perf = sizeof(perfsizes) / sizeof(perfsizes[0]);
static const size_t nbytes_perftest = 1000000000;	/* 1 GB */
static const size_t nbytes_warmup = 100000000;		/* approx 100 MB */

static int
perftest_init(void * cookie, uint8_t * buf, size_t buflen)
{
	size_t i;

	(void)cookie; /* UNUSED */

	/* Set the input. */
	for (i = 0; i < buflen; i++)
		buf[i] = (uint8_t)(i & 0xff);

	/* Success! */
	return (0);
}

static int
perftest_func(void * cookie, uint8_t * buf, size_t buflen, size_t nreps)
{
	SHA1_CTX ctx;
	uint8_t hbuf[20];
	size_t i;

	(void)cookie; /* UNUSED */

	/* Do the hashing. */
	SHA1_Init(&ctx);
	for (i = 0; i < nreps; i++)
		SHA1_Update(&ctx, buf, buflen);
	SHA1_Final(hbuf, &ctx);

	/* Success! */
	return (0);
}

static int
perftest(void)
{

	/* Time the function. */
	if (perftest_buffers(nbytes_perftest, perfsizes, num_perf,
	    nbytes_warmup, perftest_init, perftest_func, NULL, NULL)) {
		warn0("perftest_buffers");
		goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (1);
}

static struct testcase {
	const char * s;
	const char * o;
} tests[] = {
	{ "",
	"da39a3ee5e6b4b0d3255bfef95601890afd80709" },
	{ "a",
	"86f7e437faa5a7fce15d1ddcb9eaeaea377667b8" },
	{ "abc",
	"a9993e364706816aba3e25717850c26c9cd0d89d" },
	{ "message digest",
	"c12252ceda8be8994d5fa0290a47231c1d16aae3" },
	{ "abcdefghijklmnopqrstuvwxyz",
	"32d10c7b8cf96570ca04ce37f2a19d84240d3a89" },
	{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
	"761c457bf73b14d27e9e9265c46f4b4dda11f940" },
	{ "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
	"50abf5706a150990a08b2c5ea40fa0e585554732" },
	{ "MD5 has not yet (2001-09-03) been broken, but sufficient attacks have been made that its security is in some doubt",
	"18eca4333979c4181199b7b4fab8786d16cf2846" },
	{ "MD5 is now considered broken for cryptographic purposes; for more information, see https://tools.ietf.org/html/rfc6151",
	"2ae087bc6f7e02c7e4d1b962639126c711bfd05f"}
};

static int
selftest(void)
{
	SHA1_CTX ctx;
	uint8_t hbuf[20];
	char hbuf_hex[41];
	size_t i;
	size_t failures = 0;

	for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
		printf("Computing SHA1 of \"%s\"...", tests[i].s);
		SHA1_Init(&ctx);
		SHA1_Update(&ctx, (const uint8_t *)tests[i].s,
		    strlen(tests[i].s));
		SHA1_Final(hbuf, &ctx);
		hexify(hbuf, hbuf_hex, 20);
		if (strcmp(hbuf_hex, tests[i].o)) {
			printf(" FAILED!\n");
			printf("Computed SHA1: %s\n", hbuf_hex);
			printf("Correct SHA1:  %s\n", tests[i].o);
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

	fprintf(stderr, "usage: test_sha1 -t\n");
	fprintf(stderr, "       test_sha1 -x\n");
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

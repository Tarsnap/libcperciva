#include <sys/time.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"
#include "hexify.h"
#include "md5.h"
#include "perftest.h"
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

	(void)cookie;	/* UNUSED */

	/* Set the input. */
	for (i = 0; i < buflen; i++)
		buf[i] = (uint8_t)(i & 0xff);

	/* Success! */
	return (0);
}

static int
perftest_func(void * cookie, uint8_t * buf, size_t buflen, size_t nreps)
{
	MD5_CTX ctx;
	uint8_t hbuf[16];
	size_t i;

	(void)cookie; /* UNUSED */

	/* Do the hashing. */
	MD5_Init(&ctx);
	for (i = 0; i < nreps; i++)
		MD5_Update(&ctx, buf, buflen);
	MD5_Final(hbuf, &ctx);

	/* Success! */
	return (0);
}

static int
perftest(void)
{

	/* Time the function. */
	if (perftest_buffers(nbytes_perftest, perfsizes, num_perf,
	    nbytes_warmup, perftest_init, perftest_func, NULL)) {
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
	"d41d8cd98f00b204e9800998ecf8427e" },
	{ "a",
	"0cc175b9c0f1b6a831c399e269772661" },
	{ "abc",
	"900150983cd24fb0d6963f7d28e17f72" },
	{ "message digest",
	"f96b697d7cb7938d525a2f31aaf161d0" },
	{ "abcdefghijklmnopqrstuvwxyz",
	"c3fcd3d76192e4007dfb496cca67e13b" },
	{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
	"d174ab98d277d9f5a5611c2c9f419d9f" },
	{ "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
	"57edf4a22be3c955ac49da2e2107b67a" },
	{ "MD5 has not yet (2001-09-03) been broken, but sufficient attacks have been made that its security is in some doubt",
	"b50663f41d44d92171cb9976bc118538" },
	{ "MD5 is now considered broken for cryptographic purposes; for more information, see https://tools.ietf.org/html/rfc6151",
	"3a6b65dc78a585d7aad5a284ea1a688c"}
};

static int
selftest(void)
{
	MD5_CTX ctx;
	uint8_t hbuf[16];
	char hbuf_hex[33];
	size_t i;
	size_t failures = 0;

	for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
		printf("Computing MD5 of \"%s\"...", tests[i].s);
		MD5_Init(&ctx);
		MD5_Update(&ctx, (const uint8_t *)tests[i].s,
		    strlen(tests[i].s));
		MD5_Final(hbuf, &ctx);
		hexify(hbuf, hbuf_hex, 16);
		if (strcmp(hbuf_hex, tests[i].o)) {
			printf(" FAILED!\n");
			printf("Computed MD5: %s\n", hbuf_hex);
			printf("Correct MD5:  %s\n", tests[i].o);
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

	fprintf(stderr, "usage: test_MD5 -t\n");
	fprintf(stderr, "       test_MD5 -x\n");
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

#include <sys/time.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"
#include "hexify.h"
#include "md5.h"
#include "monoclock.h"
#include "warnp.h"

#define BLOCKLEN 10000
#define BLOCKCOUNT 100000

static int
perftest(void)
{
	struct timeval begin, end;
	double delta_s;
	MD5_CTX ctx;
	uint8_t hbuf[16];
	char hbuf_hex[33];
	uint8_t * buf;
	size_t i;

	/* Allocate and initialize input per FreeBSD md5(1) utility. */
	if ((buf = malloc(BLOCKLEN)) == NULL) {
		warnp("malloc");
		goto err0;
	}
	for (i = 0; i < BLOCKLEN; i++)
		buf[i] = (uint8_t)(i & 0xff);

	/* Report what we're doing. */
	printf("MD5 time trial. Digesting %d %d-byte blocks ...",
	    BLOCKCOUNT, BLOCKLEN);
	fflush(stdout);

        /* Start timer */
	if (monoclock_get_cputime(&begin)) {
		warnp("monoclock_get_cputime()");
		goto err1;
	}

	/* Perform the computation. */
	MD5_Init(&ctx);
	for (i = 0; i < BLOCKCOUNT; i++)
		MD5_Update(&ctx, buf, BLOCKLEN);
	MD5_Final(hbuf, &ctx);
	hexify(hbuf, hbuf_hex, 16);

	/* End timer. */
	if (monoclock_get_cputime(&end)) {
		warnp("monoclock_get_cputime()");
		goto err1;
	}

	/* Report status. */
	printf(" done\n");
	printf("Digest = %s\n", hbuf_hex);

	delta_s = timeval_diff(begin, end);

	printf("Time = %f seconds\n", delta_s);
	printf("Speed = %f bytes/second\n",
	    (double)BLOCKLEN * (double)BLOCKCOUNT / delta_s);

	/* Free allocated buffer. */
	free(buf);

	/* Success! */
	return (0);

err1:
	free(buf);
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

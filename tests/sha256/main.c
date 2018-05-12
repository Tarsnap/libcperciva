#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "getopt.h"
#include "hexify.h"
#include "sha256.h"
#include "warnp.h"

#define BLOCKLEN 10000
#define BLOCKCOUNT 100000

static void
perftest(void)
{
#ifdef CLOCK_VIRTUAL
	struct timespec st, en;
	double secs;
#endif
	SHA256_CTX ctx;
	uint8_t hbuf[32];
	char hbuf_hex[65];
	uint8_t * buf;
	size_t i;

	/* Allocate and initialize input per FreeBSD md5(1) utility. */
	if ((buf = malloc(BLOCKLEN)) == NULL) {
		warnp("malloc");
		exit(1);
	}
	for (i = 0; i < BLOCKLEN; i++)
		buf[i] = (uint8_t)(i & 0xff);

	/* Report what we're doing. */
	printf("SHA256 time trial. Digesting %d %d-byte blocks ...",
	    BLOCKCOUNT, BLOCKLEN);
	fflush(stdout);

#ifdef CLOCK_VIRTUAL
        /* Start timer */
	if (clock_gettime(CLOCK_VIRTUAL, &st)) {
		warnp("clock_gettime(CLOCK_VIRTUAL)");
		exit(1);
	}
#endif

	/* Perform the computation. */
	SHA256_Init(&ctx);
	for (i = 0; i < BLOCKCOUNT; i++)
		SHA256_Update(&ctx, buf, BLOCKLEN);
	SHA256_Final(hbuf, &ctx);
	hexify(hbuf, hbuf_hex, 32);

#ifdef CLOCK_VIRTUAL
	/* End timer. */
	if (clock_gettime(CLOCK_VIRTUAL, &en)) {
		warnp("clock_gettime(CLOCK_VIRTUAL)");
		exit(1);
	}
#endif

	/* Report status. */
	printf(" done\n");
	printf("Digest = %s\n", hbuf_hex);
#ifdef CLOCK_VIRTUAL
	secs = (en.tv_sec - st.tv_sec) +
	    (en.tv_nsec - st.tv_nsec) * 0.000000001;
	printf("Time = %f seconds\n", secs);
	printf("Speed = %f bytes/second\n",
	    (double)BLOCKLEN * (double)BLOCKCOUNT / secs);
#endif

	/* Free allocated buffer. */
	free(buf);
}

static struct testcase {
	const char * s;
	const char * o;
} tests[] = {
	{ "",
	"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" },
	{ "a",
	"ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb" },
	{ "abc",
	"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" },
	{ "message digest",
	"f7846f55cf23e14eebeab5b4e1550cad5b509e3348fbc4efa3a1413d393cb650" },
	{ "abcdefghijklmnopqrstuvwxyz",
	"71c480df93d6ae2f1efad1447c66c9525e316218cf51fc8d9ed832f2daf18b73" },
	{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
	"db4bfcbd4da0cd85a60c3c37d3fbd8805c77f15fc6b1fdfe614ee0a7c8fdb4c0" },
	{ "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
	"f371bc4a311f2b009eef952dd83ca80e2b60026c8e935592d0f9c308453c813e" },
	{ "MD5 has not yet (2001-09-03) been broken, but sufficient attacks have been made \
that its security is in some doubt",
	"e6eae09f10ad4122a0e2a4075761d185a272ebd9f5aa489e998ff2f09cbfdd9f" }
};

static int
selftest(void)
{
	SHA256_CTX ctx;
	uint8_t hbuf[32];
	char hbuf_hex[65];
	size_t i;
	size_t failures = 0;

	for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
		printf("Computing SHA256 of \"%s\"...", tests[i].s);
		SHA256_Init(&ctx);
		SHA256_Update(&ctx, (const uint8_t *)tests[i].s,
		    strlen(tests[i].s));
		SHA256_Final(hbuf, &ctx);
		hexify(hbuf, hbuf_hex, 32);
		if (strcmp(hbuf_hex, tests[i].o)) {
			printf(" FAILED!\n");
			printf("Computed SHA256: %s\n", hbuf_hex);
			printf("Correct CRC32C:  %s\n", tests[i].o);
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

static _Noreturn void
usage(void)
{

	fprintf(stderr, "usage: test_sha256 -t\n");
	fprintf(stderr, "       test_sha256 -x\n");
	exit(1);
}

int
main(int argc, char * argv[])
{
	const char * ch;

	/* Process arguments. */
	while ((ch = GETOPT(argc, argv)) != NULL) {
		GETOPT_SWITCH(ch) {
		GETOPT_OPT("-t"):
			perftest();
			exit(0);
		GETOPT_OPT("-x"):
			exit(selftest());
		GETOPT_DEFAULT:
			usage();
		}
	}

	usage();
}

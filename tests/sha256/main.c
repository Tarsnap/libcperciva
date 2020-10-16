#include <sys/time.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpusupport.h"
#include "getopt.h"
#include "hexify.h"
#include "monoclock.h"
#include "sha256.h"
#include "warnp.h"

#define BLOCKLEN 10000
#define BLOCKCOUNT 100000

/* Print a string, then whether or not we're using hardware instructions. */
static void
print_hardware(const char * str)
{
	int use_hardware = 0;

#if defined(CPUSUPPORT_X86_SHANI) && defined(CPUSUPPORT_X86_SSSE3)
	if (cpusupport_x86_shani() && cpusupport_x86_ssse3())
		use_hardware = 1;
#endif

	/* Inform the user. */
	printf("%s", str);
	if (use_hardware)
		printf(" using hardware SHANI.\n");
	else
		printf(" using software SHA.\n");
}

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
perftest_func(void * cookie, uint8_t * buf, size_t buflen, size_t num_buffers)
{
	SHA256_CTX ctx;
	uint8_t hbuf[32];
	size_t i;

	(void)cookie; /* UNUSED */

	/* Set the input. */
	for (i = 0; i < buflen; i++)
		buf[i] = (uint8_t)(i & 0xff);

	/* Do the hashing. */
	SHA256_Init(&ctx);
	for (i = 0; i < num_buffers; i++)
		SHA256_Update(&ctx, buf, buflen);
	SHA256_Final(hbuf, &ctx);

	/* Success! */
	return (0);
}

static int
perftest(void)
{
	struct timeval begin, end;
	double delta_s;
	uint8_t * buf;

	/* Allocate buffer. */
	if ((buf = malloc(BLOCKLEN)) == NULL) {
		warnp("malloc");
		goto err0;
	}

	/* Report what we're doing. */
	print_hardware("SHA256 time trial");
	fflush(stdout);

	/* Set up. */
	if (perftest_init(NULL, buf, BLOCKLEN))
		goto err1;

        /* Start timer */
	if (monoclock_get_cputime(&begin)) {
		warnp("monoclock_get_cputime()");
		goto err1;
	}

	/* Perform the computation. */
	if (perftest_func(NULL, buf, BLOCKLEN, BLOCKCOUNT))
		goto err1;

	/* End timer. */
	if (monoclock_get_cputime(&end)) {
		warnp("monoclock_get_cputime()");
		goto err1;
	}

	/* Prepare output. */
	delta_s = timeval_diff(begin, end);

	/* Print results. */
	printf("%zu blocks of size %zu\t%.06f s, %.01f MB/s\n",
	    (size_t)BLOCKCOUNT, (size_t)BLOCKLEN, delta_s,
	    (double)(BLOCKCOUNT * BLOCKLEN) / 1e6 / delta_s);

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
	{ "MD5 has not yet (2001-09-03) been broken, but sufficient attacks have been made that its security is in some doubt",
	"e6eae09f10ad4122a0e2a4075761d185a272ebd9f5aa489e998ff2f09cbfdd9f" },
	{ "MD5 is now considered broken for cryptographic purposes; for more information, see https://tools.ietf.org/html/rfc6151",
	"c1faed8b43f81861a508d9f4034acb854706597d3d4eea52f55bdd47debd3e70"}
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
			printf("Correct SHA256:  %s\n", tests[i].o);
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

	fprintf(stderr, "usage: test_sha256 -t\n");
	fprintf(stderr, "       test_sha256 -x\n");
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

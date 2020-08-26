#include <sys/time.h>

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpusupport.h"
#include "crypto_aes.h"
#include "getopt.h"
#include "hexify.h"
#include "insecure_memzero.h"
#include "monoclock.h"
#include "warnp.h"

#define MAX_PLAINTEXT_LENGTH 16

/* Forward declaration. */
struct crypto_aes_key;

struct testcase {
	const char * keytext_hex;
	const char * plaintext_hex;
	const char * ciphertext_hex;
};

/* Test vectors. */
static const struct testcase tests[] = {
	/* NIST FIPS 197 Appendix C.1. */
	{"000102030405060708090a0b0c0d0e0f",
	    "00112233445566778899aabbccddeeff",
	    "69c4e0d86a7b0430d8cdb78070b4c55a"},
	/* NIST FIPS 197 Appendix C.3. */
	{"000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f",
	    "00112233445566778899aabbccddeeff",
	    "8ea2b7ca516745bfeafc49904b496089"}
};

/* Test vector for the performance test. */
static const struct testcase perftestcase = {
	"000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f",
	    "00112233445566778899aabbccddeeff",
	    "a7cbdceb16a2b37924794d01fa4a5796"
};
static const size_t bytes_to_hash = 1 << 30;	/* approx 1 GB */

/* Print a name, then an array in hex. */
static void
print_arr(const char * name, uint8_t * arr, size_t len)
{
	size_t i;

	printf("%s", name);
	for (i = 0; i < len; i++)
		printf("%02x", arr[i]);
	printf("\n");
}

/* Print a string, then whether or not we're using hardware AESNI. */
static void
print_hardware(const char * str)
{
	int use_hardware = 0;

#ifdef CPUSUPPORT_X86_AESNI
	if (cpusupport_x86_aesni())
		use_hardware = 1;
#endif

	/* Inform the user. */
	printf("%s", str);
	if (use_hardware)
		printf(" using hardware AESNI.\n");
	else
		printf(" using software AES.\n");
}

static int
parse_testcase(const struct testcase testcase,
    struct crypto_aes_key ** key_exp_p, size_t * keylen_p,
    uint8_t plaintext_arr[static MAX_PLAINTEXT_LENGTH],
    uint8_t ciphertext_arr[static MAX_PLAINTEXT_LENGTH])
{
	uint8_t key[32];	/* We will use 16 or 32 of these bytes. */
	const size_t len = strlen(testcase.plaintext_hex) / 2;

	/* Determine key length. */
	*keylen_p = strlen(testcase.keytext_hex) / 2;

	/* Sanity check. */
	assert((*keylen_p == 16) || (*keylen_p == 32));
	assert(len == 16);

	/* Prepare the key. */
	if (unhexify(testcase.keytext_hex, key, *keylen_p)) {
		warn0("unhexify(%s)", testcase.keytext_hex);
		goto err0;
	}
	if ((*key_exp_p = crypto_aes_key_expand(key, *keylen_p)) == NULL) {
		warn0("crypto_aes_key_expand");
		goto err0;
	}

	/* Prepare the arrays. */
	if (unhexify(testcase.plaintext_hex, plaintext_arr, len)) {
		warn0("unhexify(%s)", testcase.plaintext_hex);
		goto err1;
	}
	if (unhexify(testcase.ciphertext_hex, ciphertext_arr, len)) {
		warn0("unhexify(%s)", testcase.ciphertext_hex);
		goto err1;
	}

	/* Clean up.  Irrelevant for a test, but it's a good habit. */
	insecure_memzero(key, 32);

	/* Success! */
	return (0);

err1:
	crypto_aes_key_free(*key_exp_p);
err0:
	/* Failure! */
	return (1);
}

static int
perftest(void)
{
	struct crypto_aes_key * key_exp;
	uint8_t plaintext_arr[16];
	uint8_t ciphertext_arr[16];
	uint8_t cbuf[16];
	size_t keylen;
	struct timeval begin, end;
	double delta_s;
	size_t i;
	size_t num_blocks = bytes_to_hash / 16;

	/* Inform user about the hardware optimization status. */
	print_hardware("Performance test of AES");
	fflush(stdout);

	/* Prepare arrays. */
	if (parse_testcase(perftestcase, &key_exp, &keylen, plaintext_arr,
	    ciphertext_arr)) {
		warn0("parse_testcase");
		goto err0;
	}
	memcpy(cbuf, plaintext_arr, 16);

	/* Warm up. */
	for (i = 0; i < 10000000; i++)
		crypto_aes_encrypt_block(cbuf, cbuf, key_exp);

	/* Reset plaintext input. */
	memcpy(cbuf, plaintext_arr, 16);

	/* Get beginning time. */
	if (monoclock_get_cputime(&begin)) {
		warnp("monoclock_get_cputime()");
		goto err1;
	}

	/* Hash all the bytes. */
	for (i = 0; i < num_blocks; i++)
		crypto_aes_encrypt_block(cbuf, cbuf, key_exp);

	/* Get ending time. */
	if (monoclock_get_cputime(&end)) {
		warnp("monoclock_get_cputime()");
		goto err1;
	}

	/* Find and print elapsed time and speed. */
	delta_s = timeval_diff(begin, end);
	printf("... in %zu blocks of size 16:\t%.06f s\t%.01f MB/s\n",
	    num_blocks, delta_s, (double)bytes_to_hash / 1e6 / delta_s);

	/* Clean up. */
	crypto_aes_key_free(key_exp);

	/* Success! */
	return (0);

err1:
	crypto_aes_key_free(key_exp);
err0:
	/* Failure! */
	return (1);
}

static int
selftest(void)
{
	struct crypto_aes_key * key_exp;
	uint8_t plaintext_arr[MAX_PLAINTEXT_LENGTH];
	uint8_t ciphertext_arr[MAX_PLAINTEXT_LENGTH];
	uint8_t cbuf[MAX_PLAINTEXT_LENGTH];
	size_t keylen;
	size_t i;
	size_t failures = 0;
	size_t num_tests = sizeof(tests) / sizeof(tests[0]);

	/* Inform user about the hardware optimization status. */
	print_hardware("Checking test vectors of AES");

	/* Run regular test cases. */
	for (i = 0; i < num_tests; i++) {
		/* Prepare for the test case. */
		if (parse_testcase(tests[i], &key_exp, &keylen,
		    plaintext_arr, ciphertext_arr)) {
			warn0("parse_testcase");
			goto err0;
		}
		printf("Computing %zu-bit AES of \"%s\"...",
		    keylen * 8, tests[i].plaintext_hex);

		/* Run AES. */
		crypto_aes_encrypt_block(plaintext_arr, cbuf, key_exp);

		/* Check result. */
		if (memcmp(cbuf, ciphertext_arr, 16)) {
			printf(" FAILED!\n");
			print_arr("Computed AES:\t", cbuf, 16);
			print_arr("Correct AES:\t", ciphertext_arr, 16);
			failures++;
		} else {
			printf(" PASSED!\n");
		}

		/* Clean up. */
		crypto_aes_key_free(key_exp);
	}

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

	fprintf(stderr, "usage: test_crypto_aes -t\n");
	fprintf(stderr, "       test_crypto_aes -x\n");
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

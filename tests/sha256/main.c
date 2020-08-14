#include <sys/time.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"
#include "hexify.h"
#include "monoclock.h"
#include "sha256.h"
#include "warnp.h"

#include "crypto_aes.h"
#include "crypto_aesctr.h"
#include "sysendian.h"

#define BLOCKLEN 1028
#define SMALLBLOCKLEN 8
#define BLOCKCOUNT 965251

/* INVESTIGATE: copied from spiped/proto/proto_crypt.h */

/* Maximum size of an unencrypted packet. */
#define PCRYPT_MAXDSZ 1024

/* Size of an encrypted packet. */
#define PCRYPT_ESZ (PCRYPT_MAXDSZ + 4 /* len */ + 32 /* hmac */)

/* INVESTIGATE: copied from spiped/proto/proto_crypt.c */

struct proto_keys {
	struct crypto_aes_key * k_aes;
	uint8_t k_hmac[32];
	uint64_t pnum;
};

/**
 * mkkeypair(kbuf):
 * Convert the 64 bytes of ${kbuf} into a protocol key structure.
 */
static struct proto_keys *
mkkeypair(uint8_t kbuf[64])
{
	struct proto_keys * k;

	/* Allocate a structure. */
	if ((k = malloc(sizeof(struct proto_keys))) == NULL)
		goto err0;

	/* Expand the AES key. */
	if ((k->k_aes = crypto_aes_key_expand(&kbuf[0], 32)) == NULL)
		goto err1;

	/* Fill in HMAC key. */
	memcpy(k->k_hmac, &kbuf[32], 32);

	/* The first packet will be packet number zero. */
	k->pnum = 0;

	/* Success! */
	return (k);

err1:
	free(k);
err0:
	/* Failure! */
	return (NULL);
}

/**
 * proto_crypt_free(k):
 * Free the protocol key structure ${k}.
 */
static void
proto_crypt_free(struct proto_keys * k)
{

	/* Be compatible with free(NULL). */
	if (k == NULL)
		return;

	/* Free the AES key. */
	crypto_aes_key_free(k->k_aes);

	/* Free the key structure. */
	free(k);
}

/*
 * proto_crypt_enc(ibuf, len, obuf, k):
 * Encrypt ${len} bytes from ${ibuf} into PCRYPT_ESZ bytes using the keys in
 * ${k}, and write the result into ${obuf}.
 */
static void
proto_crypt_enc(uint8_t * ibuf, size_t len, uint8_t obuf[PCRYPT_ESZ],
    struct proto_keys * k)
{
	HMAC_SHA256_CTX ctx;
	uint8_t pnum_exp[8];

	/* Sanity-check the length. */
	assert(len <= PCRYPT_MAXDSZ);

	/* Copy the decrypted data into the encrypted buffer. */
	memcpy(obuf, ibuf, len);

	/* Pad up to PCRYPT_MAXDSZ with zeroes. */
	memset(&obuf[len], 0, PCRYPT_MAXDSZ - len);

	/* Add the length. */
	be32enc(&obuf[PCRYPT_MAXDSZ], (uint32_t)len);

	/* Encrypt the buffer in-place. */
	crypto_aesctr_buf(k->k_aes, k->pnum, obuf, obuf, PCRYPT_MAXDSZ + 4);

	/* Append an HMAC. */
	be64enc(pnum_exp, k->pnum);
	HMAC_SHA256_Init(&ctx, k->k_hmac, 32);
	HMAC_SHA256_Update(&ctx, obuf, PCRYPT_MAXDSZ + 4);
	HMAC_SHA256_Update(&ctx, pnum_exp, 8);
	HMAC_SHA256_Final(&obuf[PCRYPT_MAXDSZ + 4], &ctx);

	/* Increment packet number. */
	k->pnum += 1;
}

static int
perftest(void)
{
	struct timeval begin, end;
	double delta_s;
	char hbuf_hex[65];
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
	printf("SHA256 time trial. Digesting %d pairs of %d bytes followed "
	    "by %d-byte blocks...",
	    BLOCKCOUNT, BLOCKLEN, SMALLBLOCKLEN);
	fflush(stdout);

	/* Prepare for proto_crypt_enc(). */
	uint8_t kbuf[64];
	memset(kbuf, 0, 64);

	static struct proto_keys * k;
	if ((k = mkkeypair(kbuf)) == NULL) {
		warn0("mkkeypair");
		goto err1;
	}

	uint8_t obuf[1028];
	memset(&obuf[0], 0, 1024);
	/* Pretend to have 1024 bytes of data. */
	be32enc(&obuf[1024], 1024);

        /* Start timer */
	if (monoclock_get_cputime(&begin)) {
		warnp("monoclock_get_cputime()");
		goto err1;
	}

	/* Perform the computation. */
	for (i = 0; i < BLOCKCOUNT; i++) {
		proto_crypt_enc(buf, 1024, obuf, k);
	}
	hexify(&obuf[PCRYPT_MAXDSZ + 4], hbuf_hex, 32);

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
	/*
	 * We're hashing BLOCKLEN + SMALLBLOCKLEN, but in spiped
	 * we're sending 1024 bytes, plus the extra overhead of
	 * 4+8.  So to imitate spiped, we'll calculate the speed
	 * here pretending that we only hashed 1024 bytes.
	 */
	printf("Speed = %f bytes/second\n",
	    (double)(BLOCKLEN - 4) * (double)BLOCKCOUNT / delta_s);

	/* Free allocated buffer. */
	free(buf);
	proto_crypt_free(k);

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

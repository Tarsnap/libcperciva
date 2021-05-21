#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/aes.h>

#include "cpusupport.h"
#include "crypto_aes_aesni.h"
#include "crypto_aes_arm.h"
#include "insecure_memzero.h"
#include "warnp.h"

#include "crypto_aes.h"

#if defined(CPUSUPPORT_X86_AESNI) | \
    defined(CPUSUPPORT_ARM_AES)
#define HWACCEL

static enum {
	HW_SOFTWARE = 0,
#if defined(CPUSUPPORT_X86_AESNI)
	HW_X86_AESNI,
#endif
#if defined(CPUSUPPORT_ARM_AES)
	HW_ARM_AES,
#endif
	HW_UNSET
} hwaccel = HW_UNSET;
#endif

/**
 * This represents either an AES_KEY or a struct crypto_aes_key_aesni; we
 * know which it is based on whether we're using AESNI code or not.  As such,
 * it's just an opaque pointer; but declaring it as a named structure type
 * prevents type-mismatch bugs in upstream code.
 */
struct crypto_aes_key;

#ifdef HWACCEL
static struct aes_test {
	const uint8_t key[32];
	const size_t len;
	const uint8_t ptext[16];
	const uint8_t ctext[32];
} testcase[] = { {
	/* NIST FIPS 179, Appendix C - Example Vectors, AES-128, p. 35. */
	.key = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},
	.len = 16,
	.ptext = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff },
	.ctext = { 0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30,
		   0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a }
	}, {
	/* NIST FIPS 179, Appendix C - Example Vectors, AES-256, p. 42. */
	.key = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, },
	.len = 32,
	.ptext = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff },
	.ctext = { 0x8e, 0xa2, 0xb7, 0xca, 0x51, 0x67, 0x45, 0xbf,
		   0xea, 0xfc, 0x49, 0x90, 0x4b, 0x49, 0x60, 0x89 }
	}
};

/* Test hardware intrinsics against test vectors. */
static int
hwtest(struct aes_test * knowngood)
{
	void * kexp_hw;
	uint8_t ctext_hw[16];

	/* Sanity-check. */
	assert((knowngood->len == 16) || (knowngood->len == 32));

	/* Expand the key and encrypt with hardware intrinsics. */
#if defined(CPUSUPPORT_X86_AESNI)
	if ((kexp_hw = crypto_aes_key_expand_aesni(knowngood->key,
	    knowngood->len)) == NULL)
		goto err0;
	crypto_aes_encrypt_block_aesni(knowngood->ptext, ctext_hw, kexp_hw);
	crypto_aes_key_free_aesni(kexp_hw);
#endif
#if defined(CPUSUPPORT_ARM_AES)
	if ((kexp_hw = crypto_aes_key_expand_arm(key, len)) == NULL)
		goto err0;
	crypto_aes_encrypt_block_arm(ptext, ctext_hw, kexp_hw);
	crypto_aes_key_free_arm(kexp_hw);
#endif

	/* Do the outputs match? */
	return (memcmp(knowngood->ctext, ctext_hw, 16));

err0:
	/* Failure! */
	return (-1);
}

/* Which type of hardware acceleration should we use, if any? */
static void
hwaccel_init(void)
{

	/* If we've already set hwaccel, we're finished. */
	if (hwaccel != HW_UNSET)
		return;

	/* Default to software. */
	hwaccel = HW_SOFTWARE;

#if defined(CPUSUPPORT_X86_AESNI)
	CPUSUPPORT_VALIDATE(hwaccel, HW_X86_AESNI, cpusupport_x86_aesni(),
	    hwtest(&testcase[0]) || hwtest(&testcase[1]));
#endif
#if defined(CPUSUPPORT_ARM_AES)
	CPUSUPPORT_VALIDATE(hwaccel, HW_ARM_AES, cpusupport_arm_aes(),
	    hwtest(ptext, key, 16) || hwtest(ptext, key, 32));
#endif
}
#endif /* HWACCEL */

/**
 * crypto_aes_use_x86_aesni(void):
 * Return non-zero if AESNI operations are available.
 */
int
crypto_aes_use_x86_aesni(void)
{

#ifdef HWACCEL
	/* Ensure that we've chosen the type of hardware acceleration. */
	hwaccel_init();

#if defined(CPUSUPPORT_X86_AESNI)
	if (hwaccel == HW_X86_AESNI)
		return (1);
#endif
#endif /* HWACCEL */

	/* Software only. */
	return (0);
}

/**
 * crypto_aes_key_expand(key, len):
 * Expand the ${len}-byte AES key ${key} into a structure which can be passed
 * to crypto_aes_encrypt_block().  The length must be 16 or 32.
 */
struct crypto_aes_key *
crypto_aes_key_expand(const uint8_t * key, size_t len)
{
	AES_KEY * kexp;

	/* Sanity-check. */
	assert((len == 16) || (len == 32));

#ifdef HWACCEL
	/* Ensure that we've chosen the type of hardware acceleration. */
	hwaccel_init();

#ifdef CPUSUPPORT_X86_AESNI
	if (hwaccel == HW_X86_AESNI)
		return (crypto_aes_key_expand_aesni(key, len));
#endif
#ifdef CPUSUPPORT_ARM_AES
	if (hwaccel == HW_ARM_AES)
		return (crypto_aes_key_expand_arm(key, len));
#endif
#endif /* HWACCEL */

	/* Allocate structure. */
	if ((kexp = malloc(sizeof(AES_KEY))) == NULL)
		goto err0;

	/* Expand the key. */
	AES_set_encrypt_key(key, (int)(len * 8), kexp);

	/* Success! */
	return ((void *)kexp);

err0:
	/* Failure! */
	return (NULL);
}

/**
 * crypto_aes_encrypt_block(in, out, key):
 * Using the expanded AES key ${key}, encrypt the block ${in} and write the
 * resulting ciphertext to ${out}.  ${in} and ${out} can overlap.
 */
void
crypto_aes_encrypt_block(const uint8_t in[16], uint8_t out[16],
    const struct crypto_aes_key * key)
{

#ifdef HWACCEL
#ifdef CPUSUPPORT_X86_AESNI
	if (hwaccel == HW_X86_AESNI) {
		crypto_aes_encrypt_block_aesni(in, out, (const void *)key);
		return;
	}
#endif
#ifdef CPUSUPPORT_ARM_AES
	if (hwaccel == HW_ARM_AES) {
		crypto_aes_encrypt_block_arm(in, out, (const void *)key);
		return;
	}
#endif
#endif /* HWACCEL */

	/* Get AES to do the work. */
	AES_encrypt(in, out, (const void *)key);
}

/**
 * crypto_aes_key_free(key):
 * Free the expanded AES key ${key}.
 */
void
crypto_aes_key_free(struct crypto_aes_key * key)
{

#ifdef HWACCEL
#ifdef CPUSUPPORT_X86_AESNI
	if (hwaccel == HW_X86_AESNI) {
		crypto_aes_key_free_aesni((void *)key);
		return;
	}
#endif
#ifdef CPUSUPPORT_ARM_AES
	if (hwaccel == HW_ARM_AES) {
		crypto_aes_key_free_arm((void *)key);
		return;
	}
#endif
#endif /* HWACCEL */

	/* Behave consistently with free(NULL). */
	if (key == NULL)
		return;

	/* Attempt to zero the expanded key. */
	insecure_memzero(key, sizeof(AES_KEY));

	/* Free the key. */
	free(key);
}

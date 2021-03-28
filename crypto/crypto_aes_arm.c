#include "cpusupport.h"
#ifdef CPUSUPPORT_ARM_AES
/**
 * CPUSUPPORT CFLAGS: ARM_AES
 */

#include <stdint.h>
#include <stdlib.h>

#include <openssl/aes.h>

#include "insecure_memzero.h"

#include "crypto_aes_arm.h"

/**
 * crypto_aes_key_expand_arm(key, len):
 * Expand the ${len}-byte AES key ${key} into a structure which can be passed
 * to crypto_aes_encrypt_block_arm().  The length must be 16 or 32.  This
 * implementation uses ARM AES instructions, and should only be used if
 * CPUSUPPORT_ARM_AES is defined and cpusupport_arm_aes() returns nonzero.
 */
void *
crypto_aes_key_expand_arm(const uint8_t * key, size_t len)
{
	AES_KEY * kexp;

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
 * crypto_aes_encrypt_block_arm(in, out, key):
 * Using the expanded AES key ${key}, encrypt the block ${in} and write the
 * resulting ciphertext to ${out}.  ${in} and ${out} can overlap.  This
 * implementation uses ARM AES instructions, and should only be used if
 * CPUSUPPORT_ARM_AES is defined and cpusupport_arm_aes() returns nonzero.
 */
void
crypto_aes_encrypt_block_arm(const uint8_t in[16], uint8_t out[16],
    const void * key)
{

	/* Get AES to do the work. */
	AES_encrypt(in, out, (const void *)key);
}

/**
 * crypto_aes_key_free_arm(key):
 * Free the expanded AES key ${key}.
 */
void
crypto_aes_key_free_arm(void * key)
{

	/* Behave consistently with free(NULL). */
	if (key == NULL)
		return;

	/* Attempt to zero the expanded key. */
	insecure_memzero(key, sizeof(AES_KEY));

	/* Free the key. */
	free(key);
}

#endif /* CPUSUPPORT_ARM_AES */

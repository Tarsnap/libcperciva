#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <openssl/aes.h>

#include "cpusupport.h"
#include "crypto_aes_aesni.h"

#include "crypto_aes.h"

/**
 * This represents either an AES_KEY or a struct crypto_aes_key_aesni; we
 * know which it is based on whether we're using AESNI code or not.  As such,
 * it's just an opaque pointer; but declaring it as a named structure type
 * prevents type-mismatch bugs in upstream code.
 */
struct crypto_aes_key;

/**
 * crypto_aes_key_expand(key, len):
 * Expand the ${len}-byte AES key ${key} into a structure which can be passed
 * to crypto_aes_encrypt_block.  The length must be 16 or 32.
 */
struct crypto_aes_key *
crypto_aes_key_expand(const uint8_t * key, size_t len)
{
	AES_KEY * kexp;

	/* Sanity-check. */
	assert((len == 16) || (len == 32));

#ifdef CPUSUPPORT_X86_AESNI
	/* Use AESNI if we can. */
	if (cpusupport_x86_aesni())
		return (crypto_aes_key_expand_aesni(key, len));
#endif

	/* Allocate structure. */
	if ((kexp = malloc(sizeof(AES_KEY))) == NULL)
		goto err0;

	/* Expand the key. */
	AES_set_encrypt_key(key, len * 8, kexp);

	/* Success! */
	return ((void *)kexp);

err0:
	/* Failure! */
	return (NULL);
}

/**
 * crypto_aes_encrypt_block(in, out, key):
 * Using the expanded AES key ${key}, encrypt the block ${in} and write the
 * resulting ciphertext to ${out}.
 */
void
crypto_aes_encrypt_block(const uint8_t * in, uint8_t * out,
    const struct crypto_aes_key * key)
{

#ifdef CPUSUPPORT_X86_AESNI
	if (cpusupport_x86_aesni()) {
		crypto_aes_encrypt_block_aesni(in, out, (const void *)key);
		return;
	}
#endif

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

#ifdef CPUSUPPORT_X86_AESNI
	if (cpusupport_x86_aesni()) {
		crypto_aes_key_free_aesni((void *)key);
		return;
	}
#endif

	/* Free the key. */
	free(key);
}

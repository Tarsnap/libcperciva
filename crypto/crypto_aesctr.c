#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "cpusupport.h"
#include "crypto_aes.h"
#include "crypto_aesctr_aesni.h"
#include "insecure_memzero.h"
#include "sysendian.h"

#include "crypto_aesctr.h"
#include "crypto_aesctr_internal.h"

static int use_aesni = -1;

/**
 * crypto_aesctr_alloc(void):
 * Allocate an object for performing AES in CTR code.  This must be followed
 * by calling _init2().
 */
struct crypto_aesctr *
crypto_aesctr_alloc(void)
{
	struct crypto_aesctr * stream;

#if defined(CPUSUPPORT_X86_AESNI)
	/* Can we use AESNI? */
	if (use_aesni == -1) {
		if (crypto_aes_use_x86_aesni())
			use_aesni = 1;
		else
			use_aesni = 0;
	}
#endif

	/* Allocate memory. */
	if ((stream = malloc(sizeof(struct crypto_aesctr))) == NULL)
		goto err0;

	/* Success! */
	return (stream);

err0:
	/* Failure! */
	return (NULL);
}

/**
 * crypto_aesctr_init2(stream, key, nonce):
 * Reset the AES-CTR stream ${stream}, using the ${key} and ${nonce}.  If ${key}
 * is NULL, retain the previous AES key.
 */
void
crypto_aesctr_init2(struct crypto_aesctr * stream,
    const struct crypto_aes_key * key, uint64_t nonce)
{

	/* If the key is NULL, retain the previous AES key. */
	if (key != NULL)
		stream->key = key;

	/* Set nonce as provided and reset bytectr. */
	be64enc(stream->pblk, nonce);
	stream->bytectr = 0;

	/*
	 * Set the counter such that the least significant byte will wrap once
	 * incremented.
	 */
	stream->pblk[15] = 0xff;

	/* Sanity check. */
	assert(stream->key != NULL);
}

/**
 * crypto_aesctr_init(key, nonce):
 * Prepare to encrypt/decrypt data with AES in CTR mode, using the provided
 * expanded ${key} and ${nonce}.  The key provided must remain valid for the
 * lifetime of the stream.  This is the same as calling _alloc() followed by
 * _init2().
 */
struct crypto_aesctr *
crypto_aesctr_init(const struct crypto_aes_key * key, uint64_t nonce)
{
	struct crypto_aesctr * stream;

	/* Sanity check. */
	assert(key != NULL);

	/* Allocate memory. */
	if ((stream = crypto_aesctr_alloc()) == NULL)
		goto err0;

	/* Initialize values. */
	crypto_aesctr_init2(stream, key, nonce);

	/* Success! */
	return (stream);

err0:
	/* Failure! */
	return (NULL);
}

/**
 * crypto_aesctr_stream(stream, inbuf, outbuf, buflen):
 * Generate the next ${buflen} bytes of the AES-CTR stream ${stream} and xor
 * them with bytes from ${inbuf}, writing the result into ${outbuf}.  If the
 * buffers ${inbuf} and ${outbuf} overlap, they must be identical.
 */
void
crypto_aesctr_stream(struct crypto_aesctr * stream, const uint8_t * inbuf,
    uint8_t * outbuf, size_t buflen)
{

#if defined(CPUSUPPORT_X86_AESNI)
	if ((use_aesni == 1) && (buflen >= 16)) {
		crypto_aesctr_aesni_stream(stream, inbuf, outbuf, buflen);
		return;
	}
#endif

	if (crypto_aesctr_stream_preblock(stream, &inbuf, &outbuf, &buflen))
		return;

	/* Process blocks of 16 bytes; we need a new cipherblock. */
	while (buflen >= 16) {
		/* Generate a block of cipherstream. */
		crypto_aesctr_stream_cipherblock_generate(stream);

		/* Encrypt the bytes and update the positions. */
		crypto_aesctr_stream_cipherblock_use(stream, &inbuf, &outbuf,
		    &buflen, 16, 0);
	}

	crypto_aesctr_stream_postblock(stream, &inbuf, &outbuf, &buflen);
}

/**
 * crypto_aesctr_free(stream):
 * Free the AES-CTR stream ${stream}.
 */
void
crypto_aesctr_free(struct crypto_aesctr * stream)
{

	/* Behave consistently with free(NULL). */
	if (stream == NULL)
		return;

	/* Zero potentially sensitive information. */
	insecure_memzero(stream, sizeof(struct crypto_aesctr));

	/* Free the stream. */
	free(stream);
}

/**
 * crypto_aesctr_buf(key, nonce, inbuf, outbuf, buflen):
 * Equivalent to _init(key, nonce); _stream(inbuf, outbuf, buflen); _free().
 */
void
crypto_aesctr_buf(const struct crypto_aes_key * key, uint64_t nonce,
    const uint8_t * inbuf, uint8_t * outbuf, size_t buflen)
{
	struct crypto_aesctr stream_rec;
	struct crypto_aesctr * stream = &stream_rec;

	/* Sanity check. */
	assert(key != NULL);

#if defined(CPUSUPPORT_X86_AESNI)
	/* Can we use AESNI? */
	if (use_aesni == -1) {
		if (crypto_aes_use_x86_aesni())
			use_aesni = 1;
		else
			use_aesni = 0;
	}
#endif

	/* Initialize values. */
	crypto_aesctr_init2(stream, key, nonce);

	/* Perform the encryption. */
	crypto_aesctr_stream(stream, inbuf, outbuf, buflen);

	/* Zero potentially sensitive information. */
	insecure_memzero(stream, sizeof(struct crypto_aesctr));
}

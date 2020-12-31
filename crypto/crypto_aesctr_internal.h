#ifndef _CRYPTO_AESCTR_INTERNAL_H_
#define _CRYPTO_AESCTR_INTERNAL_H_

#include <stdint.h>

/* Opaque type. */
struct crypto_aes_key;

/* AES-CTR state. */
struct crypto_aesctr {
	const struct crypto_aes_key * key;
	uint64_t bytectr;
	uint8_t buf[16];
	uint8_t pblk[16];
};

/* Generate a block of cipherstream. */
static inline void
crypto_aesctr_stream_cipherblock_generate(struct crypto_aesctr * stream)
{

	/* Sanity check. */
	assert(stream->bytectr % 16 == 0);

	/* Prepare counter. */
	stream->pblk[15]++;
	if (stream->pblk[15] == 0) {
		/*
		 * If incrementing the least significant byte resulted in it
		 * wrapping, re-encode the complete 64-bit value.
		 */
		be64enc(stream->pblk + 8, stream->bytectr / 16);
	}

	/* Encrypt the cipherblock. */
	crypto_aes_encrypt_block(stream->pblk, stream->buf, stream->key);
}

/* Encrypt ${nbytes} bytes, then update ${inbuf}, ${outbuf}, and ${buflen}. */
static inline void
crypto_aesctr_stream_cipherblock_use(struct crypto_aesctr * stream,
    const uint8_t ** inbuf, uint8_t ** outbuf, size_t * buflen, size_t nbytes,
    size_t bytemod)
{
	size_t i;

	/* Encrypt the byte(s). */
	for (i = 0; i < nbytes; i++)
		(*outbuf)[i] = (*inbuf)[i] ^ stream->buf[bytemod + i];

	/* Move to the next byte(s) of cipherstream. */
	stream->bytectr += nbytes;

	/* Update the positions. */
	*inbuf += nbytes;
	*outbuf += nbytes;
	*buflen -= nbytes;
}

#endif /* !_CRYPTO_AESCTR_INTERNAL_H_ */

#include "cpusupport.h"
#ifdef CPUSUPPORT_X86_AESNI

#include <assert.h>
#include <stdint.h>

#include <emmintrin.h>

#include "crypto_aes.h"
#include "sysendian.h"

#include "crypto_aesctr_aesni.h"
#include "crypto_aesctr_internal.h"

/* Generate a block of cipherstream. */
static inline void
crypto_aesctr_aesni_stream_cipherblock_generate(struct crypto_aesctr * stream)
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
crypto_aesctr_aesni_stream_cipherblock_use(struct crypto_aesctr * stream,
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

static inline void
crypto_aesctr_aesni_stream_wholeblock(struct crypto_aesctr * stream,
    const uint8_t ** inbuf, uint8_t ** outbuf, size_t * buflen)
{
	__m128i bufsse;
	__m128i inbufsse;

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
	bufsse = _mm_loadu_si128((const __m128i *)stream->buf);

	/* Encrypt the byte(s). */
	inbufsse = _mm_loadu_si128((const __m128i *)(*inbuf));
	bufsse = _mm_xor_si128(inbufsse, bufsse);
	_mm_storeu_si128((__m128i *)(*outbuf), bufsse);

	/* Update the positions. */
	stream->bytectr += 16;
	*inbuf += 16;
	*outbuf += 16;
	*buflen -= 16;
}

/**
 * crypto_aesctr_aesni_stream(stream, inbuf, outbuf, buflen):
 * Generate the next ${buflen} bytes of the AES-CTR stream ${stream} and xor
 * them with bytes from ${inbuf}, writing the result into ${outbuf}.  If the
 * buffers ${inbuf} and ${outbuf} overlap, they must be identical.
 */
void
crypto_aesctr_aesni_stream(struct crypto_aesctr * stream, const uint8_t * inbuf,
    uint8_t * outbuf, size_t buflen)
{
	size_t bytemod;

	/* Do we have any bytes left in the current cipherblock? */
	bytemod = stream->bytectr % 16;
	if (bytemod != 0) {
		/* Do we have enough to complete the request? */
		if (bytemod + buflen <= 16) {
			/* Process only buflen bytes, then return. */
			crypto_aesctr_aesni_stream_cipherblock_use(stream,
			    &inbuf, &outbuf, &buflen, buflen, bytemod);
			return;
		}

		/* Encrypt the byte(s) and update the positions. */
		crypto_aesctr_aesni_stream_cipherblock_use(stream, &inbuf,
		    &outbuf, &buflen, 16 - bytemod, bytemod);
	}

	/* Process blocks of 16 bytes; we need a new cipherblock. */
	while (buflen >= 16) {
		/* Generate a cipherblock & use it completely. */
		crypto_aesctr_aesni_stream_wholeblock(stream, &inbuf,
		    &outbuf, &buflen);
	}

	/* Process any final bytes; we need a new cipherblock. */
	if (buflen > 0) {
		/* Generate a block of cipherstream. */
		crypto_aesctr_aesni_stream_cipherblock_generate(stream);

		/* Encrypt the byte(s) and update the positions. */
		crypto_aesctr_aesni_stream_cipherblock_use(stream, &inbuf,
		    &outbuf, &buflen, buflen, 0);
	}
}

#endif /* CPUSUPPORT_X86_AESNI */

#include "cpusupport.h"
#ifdef CPUSUPPORT_X86_AESNI

#include <assert.h>
#include <stdint.h>

#include <emmintrin.h>

#include "crypto_aes.h"
#include "crypto_aes_aesni_m128i.h"
#include "sysendian.h"

#include "crypto_aesctr_aesni.h"
#include "crypto_aesctr_internal.h"

static inline void
crypto_aesctr_aesni_stream_wholeblock(struct crypto_aesctr * stream,
    const uint8_t ** inbuf, uint8_t ** outbuf)
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
	bufsse = _mm_loadu_si128((const __m128i *)stream->pblk);
	bufsse = crypto_aes_encrypt_block_aesni_m128i(bufsse, stream->key);

	/* Encrypt the byte(s). */
	inbufsse = _mm_loadu_si128((const __m128i *)(*inbuf));
	bufsse = _mm_xor_si128(inbufsse, bufsse);
	_mm_storeu_si128((__m128i *)(*outbuf), bufsse);

	/* Update the positions. */
	stream->bytectr += 16;
	*inbuf += 16;
	*outbuf += 16;
}

/**
 * crypto_aesctr_aesni_stream_blocks(stream, inbuf, outbuf, nblocks):
 * Generate the next ${nblocks} blocks of the AES-CTR stream ${stream} and xor
 * them with bytes from ${inbuf}, writing the result into ${outbuf}.  If the
 * buffers ${inbuf} and ${outbuf} overlap, they must be identical.
 */
void
crypto_aesctr_aesni_stream_blocks(struct crypto_aesctr * stream,
    const uint8_t * inbuf, uint8_t * outbuf, size_t nblocks)
{
	size_t i;

	/* Process nblocks blocks of 16 bytes. */
	for (i = 0; i < nblocks; i++)
		crypto_aesctr_aesni_stream_wholeblock(stream, &inbuf, &outbuf);
}

#endif /* CPUSUPPORT_X86_AESNI */

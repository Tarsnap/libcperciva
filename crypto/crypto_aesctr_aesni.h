#ifndef _CRYPTO_AESCTR_AESNI_H_
#define _CRYPTO_AESCTR_AESNI_H_

#include <stddef.h>
#include <stdint.h>

/* Opaque type. */
struct crypto_aesctr;

/**
 * crypto_aesctr_aesni_stream_blocks(stream, inbuf, outbuf, nblocks):
 * Generate the next ${nblocks} blocks of the AES-CTR stream ${stream} and xor
 * them with bytes from ${inbuf}, writing the result into ${outbuf}.  If the
 * buffers ${inbuf} and ${outbuf} overlap, they must be identical.
 */
void crypto_aesctr_aesni_stream_blocks(struct crypto_aesctr *,
    const uint8_t *, uint8_t *, size_t);

#endif /* !_CRYPTO_AESCTR_AESNI_H_ */

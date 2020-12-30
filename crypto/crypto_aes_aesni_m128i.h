#ifndef _CRYPTO_AES_AESNI_M128I_H_
#define _CRYPTO_AES_AESNI_M128I_H_

#include <emmintrin.h>

/**
 * crypto_aes_encrypt_block_aesni_m128i(in, key):
 * Using the expanded AES key ${key}, encrypt the block ${in} and return the
 * resulting ciphertext.  This implementation uses x86 AESNI instructions,
 * and should only be used if CPUSUPPORT_X86_AESNI is defined and
 * cpusupport_x86_aesni() returns nonzero.
 */
__m128i crypto_aes_encrypt_block_aesni_m128i(const __m128i, const void *);

#endif /* !_CRYPTO_AES_AESNI_M128I_H_ */

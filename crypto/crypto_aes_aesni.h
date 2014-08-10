#ifndef _CRYPTO_AES_AESNI_H_
#define _CRYPTO_AES_AESNI_H_

/**
 * crypto_aes_encrypt_block_aesni(in, out, key):
 * Using the expanded AES key ${key}, encrypt the block ${in} and write the
 * resulting ciphertext to ${out}.  This implementation uses x86 AESNI
 * instructions, and should only be used if CPUSUPPORT_X86_AESNI is defined
 * and cpusupport_x86_aesni() returns nonzero.
 */
void crypto_aes_encrypt_block_aesni(const uint8_t *, uint8_t *, const AES_KEY *);

#endif /* !_CRYPTO_AES_AESNI_H_ */

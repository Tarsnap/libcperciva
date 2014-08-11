#ifndef _CRYPTO_AES_H_
#define _CRYPTO_AES_H_

#include <stdint.h>

#include <openssl/aes.h>

/**
 * crypto_aes_encrypt_block(in, out, key):
 * Using the expanded AES key ${key}, encrypt the block ${in} and write the
 * resulting ciphertext to ${out}.
 */
void crypto_aes_encrypt_block(const uint8_t *, uint8_t *, const AES_KEY *);

#endif /* !_CRYPTO_AES_H_ */

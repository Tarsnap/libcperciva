#ifndef _CRYPTO_AES_H_
#define _CRYPTO_AES_H_

#include <stdint.h>

#include <openssl/aes.h>

void aes_encrypt_block(const uint8_t *, uint8_t *, const AES_KEY *);

#endif /* !_CRYPTO_AES_H_ */

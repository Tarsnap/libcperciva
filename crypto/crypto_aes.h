#include <openssl/aes.h>

#include <stdint.h>

void aes_encrypt_block(const uint8_t *, uint8_t *, const AES_KEY *);

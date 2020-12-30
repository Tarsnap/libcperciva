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

#endif /* !_CRYPTO_AESCTR_INTERNAL_H_ */

#include <stdint.h>

#include <openssl/aes.h>

#include "cpusupport.h"
#include "crypto_aes_aesni.h"

#include "crypto_aes.h"

/**
 * crypto_aes_encrypt_block(in, out, key):
 * Using the expanded AES key ${key}, encrypt the block ${in} and write the
 * resulting ciphertext to ${out}.
 */
void
crypto_aes_encrypt_block(const uint8_t * in, uint8_t * out, const AES_KEY * key)
{

#ifdef CPUSUPPORT_X86_AESNI
	if (cpusupport_x86_aesni())
		return crypto_aes_encrypt_block_aesni(in, out, key);
#endif
	return AES_encrypt(in, out, key);
}

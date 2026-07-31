#ifndef CRYPTO_DH_H_
#define CRYPTO_DH_H_

#include <stdint.h>

/* Sizes of Diffie-Hellman private, public, and exchanged keys. */
#define CRYPTO_DH_PRIVLEN	32
#define CRYPTO_DH_PUBLEN	256
#define CRYPTO_DH_KEYLEN	256

/**
 * crypto_dh_generate_pub(pub, priv):
 * Compute ${pub} equal to 2^(2^258 + ${priv}) in Diffie-Hellman group #14.
 */
int crypto_dh_generate_pub(uint8_t[CRYPTO_DH_PUBLEN],
    const uint8_t[CRYPTO_DH_PRIVLEN]);

/**
 * crypto_dh_generate(pub, priv):
 * Generate a 256-bit private key ${priv}, and compute ${pub} equal to
 * 2^(2^258 + ${priv}) mod p where p is the Diffie-Hellman group #14 modulus.
 * Both values are stored as big-endian integers.
 */
int crypto_dh_generate(uint8_t[CRYPTO_DH_PUBLEN], uint8_t[CRYPTO_DH_PRIVLEN]);

/**
 * crypto_dh_compute(pub, priv, key):
 * In the Diffie-Hellman group #14, compute ${pub}^(2^258 + ${priv}) and
 * write the result into ${key}.  All values are big-endian.  Note that the
 * value ${pub} is the public key produced by the call to crypto_dh_generate()
 * made by the *other* participant in the key exchange.
 */
int crypto_dh_compute(const uint8_t[CRYPTO_DH_PUBLEN],
    const uint8_t[CRYPTO_DH_PRIVLEN], uint8_t[CRYPTO_DH_KEYLEN]);

/**
 * crypto_dh_sanitycheck(pub):
 * Sanity-check the Diffie-Hellman public value ${pub} by checking that it
 * is less than the group #14 modulus.  Return 0 if sane, -1 if insane.
 *
 * Note that we do not check for "weak" parameters (e.g. ${pub} equal to 0,
 * 1, or -1); doing so is an anti-pattern since (a) if an attacker can MITM
 * (e.g. if you don't verify a signature on the public parameter) then they
 * can break the security without needing "weak" parameters, and (b) if there
 * is no MITM then "weak" parameters can only be sent by a misbehaving peer,
 * and deliberately breaking the cryptographic security doesn't win anything
 * since they can deliberately leak information if they want.
 */
int crypto_dh_sanitycheck(const uint8_t[CRYPTO_DH_PUBLEN]);

#endif /* !CRYPTO_DH_H_ */

#ifdef CPUSUPPORT_X86_AESNI
#include <openssl/aes.h>

#include <stdint.h>
#include <wmmintrin.h>

void
aes_encrypt_block_aesni(const uint8_t *in, uint8_t *out, const AES_KEY *key)
{
	int final_index;
	const __m128i *aes_key;
	__m128i aes_state;

	aes_key = (const __m128i *)key->rd_key;

	aes_state = _mm_loadu_si128((__m128i *)in);
	aes_state = _mm_xor_si128(aes_state, aes_key[0]);
	aes_state = _mm_aesenc_si128(aes_state, aes_key[1]);
	aes_state = _mm_aesenc_si128(aes_state, aes_key[2]);
	aes_state = _mm_aesenc_si128(aes_state, aes_key[3]);
	aes_state = _mm_aesenc_si128(aes_state, aes_key[4]);
	aes_state = _mm_aesenc_si128(aes_state, aes_key[5]);
	aes_state = _mm_aesenc_si128(aes_state, aes_key[6]);
	aes_state = _mm_aesenc_si128(aes_state, aes_key[7]);
	aes_state = _mm_aesenc_si128(aes_state, aes_key[8]);
	aes_state = _mm_aesenc_si128(aes_state, aes_key[9]);
	final_index = 10;
	if (key->rounds > 10) {
		aes_state = _mm_aesenc_si128(aes_state, aes_key[10]);
		aes_state = _mm_aesenc_si128(aes_state, aes_key[11]);
		final_index = 12;

		if (key->rounds > 12) {
			aes_state = _mm_aesenc_si128(aes_state, aes_key[12]);
			aes_state = _mm_aesenc_si128(aes_state, aes_key[13]);
			final_index = 14;
		}
	}

	aes_state = _mm_aesenclast_si128(aes_state, aes_key[final_index]);
	_mm_storeu_si128((__m128i *)out, aes_state);
}

#endif

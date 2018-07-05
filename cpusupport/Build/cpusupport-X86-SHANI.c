#include <stdint.h>
#include <immintrin.h>

static uint8_t block[16] = {0};

int
main(void)
{
	uint32_t state[8] = {0};
	__m128i x, y;

	x = _mm_loadu_si128((const __m128i *)&state[0]);
	y = _mm_loadu_si128((const __m128i *)&block[0]);
	y = _mm_sha256msg1_epu32(x, y);
	_mm_storeu_si128((__m128i *)&state[0], y);
	return ((int)state[0]);
}

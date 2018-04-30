#include <smmintrin.h>

int
main(void)
{
	unsigned char x = 0;
	unsigned long y = 0;

	/* Test both the 8-bit and 64-bit data versions. */
	_mm_crc32_u8(x, x);
	_mm_crc32_u64(y, y);
	return (0);
}

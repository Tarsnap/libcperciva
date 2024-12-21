#include <immintrin.h>

int
main(void)
{
	unsigned int x;

	/* Done! */
	return (!_rdrand32_step(&x));
}

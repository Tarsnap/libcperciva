#include "cpusupport.h"
#ifdef CPUSUPPORT_X86_RDRAND
/**
 * CPUSUPPORT CFLAGS: X86_RDRAND
 */

#include <immintrin.h>
#include <stddef.h>

#include "crypto_entropy_rdrand.h"

/*
 * This value is suggested by the Intel Digital Random Number Generator
 * Software Implementation Guide, 5.2.1 Retry Recommendations.
 */
#define RDRAND_RETRIES 10

/**
 * generate_seed_rdrand(buf, len):
 * Fill the ${buf} buffer with values from RDRAND.  This implementation uses
 * the RDRAND instruction, and should only be used if CPUSUPPORT_X86_RDRAND is
 * defined and cpusupport_x86_rdrand() returns nonzero.
 */
int
generate_seed_rdrand(unsigned int * buf, size_t len)
{
	size_t i;
	int attempts;

	/* Fill buffer. */
	for (i = 0; i < len; i++) {
		for (attempts = 0; attempts < RDRAND_RETRIES; attempts++) {
			if (_rdrand32_step(&buf[i]))
				break;
		}
		if (attempts >= RDRAND_RETRIES)
			goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (1);
}
#endif /* CPUSUPPORT_X86_RDRAND */

#include <stdio.h>

#include "cpusupport.h"

int
main()
{

	/*
	 * Try to include some symbols, but it's not a problem for the
	 * automatic test if they aren't present.
	 */
#if defined(CPUSUPPORT_X86_SSE2)
	if (cpusupport_x86_sse2())
		printf("supports SSE2 instructions (others not tested)\n");
#endif

	/*
	 * Make sure that we don't include all potential cpusupport symbols,
	 * to check that the compiler can deal with their lack.
	 */

	return (0);
}

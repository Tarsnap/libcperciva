#include "cpusupport.h"

#ifdef CPUSUPPORT_X86_CPUID
#include <cpuid.h>
#endif

#define CPUID_EAX 1
#define CPUID_ECX_BIT (0x2000000)

CPUSUPPORT_FEATURE_DECL(x86, aesni)
{
#ifdef CPUSUPPORT_X86_CPUID
	unsigned int a, b, c, d;
	if (__get_cpuid(CPUID_EAX, &a, &b, &c, &d) == 0) {
		/* Failure. */
		return 0;
	}

	return (c & CPUID_ECX_BIT) ? 1 : 0;
#else
	return 0;
#endif
}

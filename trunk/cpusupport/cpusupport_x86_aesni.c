#include "cpusupport.h"

#ifdef CPUSUPPORT_X86_CPUID
#include <cpuid.h>
#endif

#define CPUID_AESNI_BIT (1 << 25)

CPUSUPPORT_FEATURE_DECL(x86, aesni)
{
#ifdef CPUSUPPORT_X86_CPUID
	unsigned int eax, ebx, ecx, edx;

	if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx))
		goto unsupported;

	/* Return the relevant feature bit. */
	return (ecx & CPUID_AESNI_BIT);

unsupported:
#endif
	return (0);
}

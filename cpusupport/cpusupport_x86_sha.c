#include "cpusupport.h"

#ifdef CPUSUPPORT_X86_CPUID
#include <cpuid.h>

#define CPUID_SHA_BIT (1 << 29)
#endif

CPUSUPPORT_FEATURE_DECL(x86, sha)
{
#ifdef CPUSUPPORT_X86_CPUID
	unsigned int eax, ebx, ecx, edx;

	/* Check if CPUID supports the level we need. */
	if (!__get_cpuid(0, &eax, &ebx, &ecx, &edx))
		goto unsupported;
	if (eax < 7)
		goto unsupported;

	/* Ask about CPU features.  This macro takes e?x as pointers! */
	__cpuid_count(7, 0, eax, ebx, ecx, edx);

	/* Return the relevant feature bit. */
	return ((ebx & CPUID_SHA_BIT) ? 1 : 0);

unsupported:
#endif
	return (0);
}

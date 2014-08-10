#include <cpuid.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#include "cpusupport.h"

#define CPUID_EAX 1
#define CPUID_ECX_BIT (0x2000000)

int
cpusupport_x86_aesni_detect(void)
{
#ifdef CPUSUPPORT_X86_CPUID
	unsigned int a, b, c, d;
	if (__get_cpuid(CPUID_EAX, &a, &b, &c, &d) == 0) {
		/* Failure. */
		return -1;
	}

#ifdef DEBUG
	if ((c & CPUID_ECX_BIT)) {
		printf("AES-NI enabled\n");
	}
#endif

	return (c & CPUID_ECX_BIT) ? 1 : -1;
#else
	return -1;
#endif
}

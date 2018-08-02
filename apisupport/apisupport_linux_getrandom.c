#include "apisupport.h"

#ifdef APISUPPORT_LINUX_GETRANDOM
#include <sys/random.h>

#define BUFLEN 2
#endif

APISUPPORT_FEATURE_DECL(linux, getrandom)
{
#ifdef APISUPPORT_LINUX_GETRANDOM
	char buf[BUFLEN];

	/* Bail if there's any error. */
	if (getrandom(buf, BUFLEN, 0) != BUFLEN)
		goto unsupported;

	/* Success! */
	return (1);

unsupported:
#endif
	return (0);
}

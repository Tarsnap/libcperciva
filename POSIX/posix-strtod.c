#include <stdlib.h>

int
main(void)
{
	char * eptr = NULL;

	/* Try to parse a hex value. */
	strtod("0x1", &eptr);

	/* Problem occurred. */
	if (eptr[0] != '\0')
		return (1);

	/* Success! */
	return (0);
}

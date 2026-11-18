#include <stdlib.h>

#include "warnp.h"

#include "check.h"

int
main(int argc, char * argv[])
{

	WARNP_INIT;
	(void)argc; /* UNUSED */

	if (check_mutex())
		goto err0;

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>

#include "warnp.h"

#include "monoclock.h"

int
main(int argc, char * argv[])
{
	struct timeval tv;

	WARNP_INIT;

	(void)argc; /* UNUSED */
	(void)argv; /* UNUSED */

	if (monoclock_get(&tv)) {
		warnp("monoclock_get()");
		goto err0;
	}

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/* Problem with FreeBSD 11.0 merely linking with -lrt. */
static void
pl_freebsd_link_lrt()
{
	/* Do nothing. */
}

/* Problem with FreeBSD 11.0 and printf(). */
static void
pl_freebsd_printf()
{
	const char * printme = "";

	printf("%s", printme);
}

/* Problem with FreeBSD 11.0 and getdelim(). */
static void
pl_freebsd_getdelim()
{
	size_t linecap = 0;
	char * line = NULL;

	if (getline(&line, &linecap, stdin) == -1)
		printf("error in getline()\n");
}

int
main()
{

	/* Test potential memory leaks. */
	pl_freebsd_link_lrt();
	pl_freebsd_printf();
	pl_freebsd_getdelim();

	/* Success! */
	exit(0);
}

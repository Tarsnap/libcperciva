#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ptrheap.h"
#include "warnp.h"

static int
compar(void * cookie, const void * x, const void * y)
{
	const char * _x = x;
	const char * _y = y;

	(void)cookie; /* UNUSED */

	return (strcmp(_x, _y));
}

int
main(int argc, char * argv[])
{
	struct ptrheap * H;
	char * s, * dups;
	size_t buflen;
	ssize_t l;

	WARNP_INIT;

	(void)argc; /* UNUSED */
	(void)argv; /* UNUSED */

	/* Create a heap. */
	if ((H = ptrheap_init(compar, NULL, NULL)) == 0) {
		warnp("ptrheap_init");
		goto err0;
	}

	/* Suck in the input. */
	s = NULL;
	buflen = 0;
	while ((l = getline(&s, &buflen, stdin)) != -1) {
		/* Check for embedded NUL characters. */
		if (strlen(s) != (size_t)l) {
			warnp("Line of length %zu has embedded NUL: %s",
			    (size_t)l, s);
			goto err1;
		}

		/* Remove trailing '\n'. */
		while ((l > 0) && (s[l - 1] == '\n'))
			s[--l] = '\0';

		/* Duplicate string. */
		if ((dups = strdup(s)) == NULL) {
			warnp("strdup");
			goto err2;
		}

		/* Insert string. */
		if (ptrheap_add(H, dups)) {
			warn0("ptrheap_add");
			goto err2;
		}
	}

	/* Free string allocated by getline. */
	free(s);

	/* Write out the lines in order. */
	while ((s = ptrheap_getmin(H)) != NULL) {
		printf("%s\n", s);
		ptrheap_deletemin(H);
		free(s);
	}

	/* Free the heap. */
	ptrheap_free(H);

	/* Success! */
	return (0);

err2:
	free(s);
err1:
	ptrheap_free(H);
err0:
	/* Failure! */
	return (1);
}

#include <stdlib.h>

#include "hexify.h"

#include "rfc3986.h"

/**
 * rfc3986_encode(s):
 * URL-encode the provided string by replacing any characters which are not
 * in [a-zA-Z0-9~._-] with %XX equivalents.
 */
char *
rfc3986_encode(const char * s)
{
	size_t i, j;
	char * t;
	uint8_t b;

	/* Scan the string and compute its URL-encoded length. */
	for (j = i = 0; s[i] != '\0'; i++) {
		if ((('a' <= s[i]) && (s[i] <= 'z')) ||
		    (('A' <= s[i]) && (s[i] <= 'Z')) ||
		    (('0' <= s[i]) && (s[i] <= '9')) ||
		    (s[i] == '-') || (s[i] == '_') ||
		    (s[i] == '.') || (s[i] == '~')) {
			/* These characters do not need to be encoded. */
			j += 1;
		} else {
			/* This gets encoded as %XX, i.e. 3 bytes. */
			j += 3;
		}
	}

	/* Allocate space for a NUL-terminated URL-encoded string. */
	if ((t = malloc(j + 1)) == NULL)
		goto err0;

	/* URL-encode the string. */
	for (j = i = 0; s[i] != '\0'; i++) {
		if ((('a' <= s[i]) && (s[i] <= 'z')) ||
		    (('A' <= s[i]) && (s[i] <= 'Z')) ||
		    (('0' <= s[i]) && (s[i] <= '9')) ||
		    (s[i] == '-') || (s[i] == '_') ||
		    (s[i] == '.') || (s[i] == '~')) {
			/* These characters do not need to be encoded. */
			t[j++] = s[i];
		} else {
			/* Treat the byte as unsigned. */
			b = s[i];

			/* Encode the character. */
			t[j++] = '%';
			hexify(&b, &t[j], 1);

			/* Convert [a-f] to [A-Z]. */
			if (('a' <= t[j]) && (t[j] <= 'f'))
				t[j] += 'A' - 'a';
			if (('a' <= t[j + 1]) && (t[j + 1] <= 'f'))
				t[j + 1] += 'A' - 'a';
			j += 2;
		}
	}

	/* NUL-terminate the string. */
	t[j] = '\0';

	/* Success! */
	return (t);

err0:
	/* Failure! */
	return (NULL);
}

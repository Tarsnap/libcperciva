#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "humansize.h"
#include "warnp.h"

static struct testcase {
	const char * s;
	int bad;
	uint64_t val;
	const char * canonical;
} tests[] = {
	/* good strings */
	{ "1", 0, 1ULL, "1 B" },
	{ "2B", 0, 2ULL, "2 B" },
	{ "1234G", 0, 1234000000000ULL, "1.2 TB" },
	{ "9 B", 0, 9ULL, "9 B" },
	{ "0 E", 0, 0ULL, "0 B" },
	{ "321 k", 0, 321000ULL, "321 kB" },
	{ "10 EB", 0, 10000000000000000000ULL, "10 EB" },
	/* bad strings */
	{ "", 1, 0ULL, "" },
	{ "1 BB", 1, 0ULL, "" },
	{ "1  EB", 1, 0ULL, "" },
	{ "100 EB", 1, 0ULL, "" }
};

int
main(int argc, char * argv[])
{
	size_t i;
	uint64_t size;
	char * s;
	int failures = 0;

	WARNP_INIT;
	(void)argc; /* UNUSED */

	for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
		/* Notify about what we're trying to do. */
		if (!tests[i].bad)
			printf("Parsing \"%s\" as a number of bytes:",
			    tests[i].s);
		else
			printf("Should fail to parse \"%s\" as a number"
			    " of bytes:", tests[i].s);

		/* Try to parse the test string. */
		if (humansize_parse(tests[i].s, &size)) {
			if (!tests[i].bad) {
				printf(" FAILED!\n");
				failures++;
			} else {
				printf(" PASSED!\n");
			}
		} else {
			if (tests[i].bad || tests[i].val != size) {
				printf(" FAILED!\n");
				failures++;
			} else {
				printf(" PASSED!\n");
			}
		}
	}

	for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
		/* Don't try to do anything with the "bad values". */
		if (tests[i].bad)
			continue;

		/* Notify about what we're trying to do. */
		printf("Generating a string for %" PRIu64 ":", tests[i].val);

		/* Generate (and check) a string. */
		s = humansize(tests[i].val);
		if (strcmp(tests[i].canonical, s) == 0)
			printf(" PASSED!\n");
		else {
			printf(" FAILED!\n");
			failures++;
		}
		free(s);
	}

	if (failures)
		return (1);
	else
		return (0);
}

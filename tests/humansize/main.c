#include <stdint.h>
#include <stdio.h>

#include "humansize.h"
#include "warnp.h"

static struct testcase {
	const char * s;
	int bad;
	uint64_t val;
} tests[] = {
	{ "1", 0, 1ULL },
	{ "2B", 0, 2ULL },
	{ "1234G", 0, 1234000000000ULL },
	{ "9 B", 0, 9ULL },
	{ "0 E", 0, 0ULL },
	{ "321 k", 0, 321000ULL },
	{ "10 EB", 0, 10000000000000000000ULL },
	{ "", 1, 0ULL },
	{ "1 BB", 1, 0ULL },
	{ "1  EB", 1, 0ULL },
	{ "100 EB", 1, 0ULL }
};

int
main(int argc, char * argv[])
{
	size_t i;
	uint64_t size;
	int failures = 0;

	WARNP_INIT;
	(void)argc; /* UNUSED */

	for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
		printf("Parsing \"%s\" as a number of bytes", tests[i].s);
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

	if (failures)
		return (1);
	else
		return (0);
}

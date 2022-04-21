#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

static struct jsontest {
	const char * json;
	const char * tofind;
	const char * answer;
} tests[] = {
	/* Matches. */
	{ "{\"foo\":\"\"}", "foo", "\"\"}"},
	{ "{\"foo\":\"bar\"}", "foo", "\"bar\"}"},
	{ "{\"fo\\to\":\"bar\"}", "fo\to", "\"bar\"}"},
	{ "{\"fo\\/o\":\"bar\"}", "fo/o", "\"bar\"}"},
	{ "{\"fo\\\"o\":\"bar\"}", "fo\"o", "\"bar\"}"},
	/* Matches later in the object. */
	{ "{\"food\":\"barf\",\"foo\":\"bar\"}", "foo", "\"bar\"}"},
	{ "{\"food\":true,\"foo\":\"bar\"}", "foo", "\"bar\"}"},
	{ "{\"food\":123,\"foo\":\"bar\"}", "foo", "\"bar\"}"},
	{ "{\"food\":+123.456E-7890,\"foo\":\"bar\"}", "foo", "\"bar\"}"},
	{ "{\"food\":[123,234,567,\"bar\",{\"foo\":\"barf\"}],\"foo\":\"bar\"}", "foo", "\"bar\"}"},
	/* Random whitespace. */
	{ " {\"food\"\n:\"barf\",\"foo\": \t\"bar\" }", "foo", "\"bar\" }"},
	/* Non-matches. */
	{ "{\"foo\":\"bar\"}", "fo", ""},
	{ "{\"foo\":\"bar\"}", "foo2", ""},
	/* Non-7-bit-clean. */
	{ "{\"tête\":\"oui\"}", "tête", "\"oui\"}"},
	{ "{\"t\u00EAte\":\"oui\"}", "t\u00EAte", "\"oui\"}"},
};

int
main(int argc, char * argv[])
{
	size_t i;
	const uint8_t * found;

	(void)argc; /* UNUSED */
	(void)argv; /* UNUSED */

	/* Iterate through the tests. */
	for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
		found = json_find((const uint8_t *)tests[i].json,
		    (const uint8_t *)tests[i].json + strlen(tests[i].json),
		    tests[i].tofind);
		if (strcmp((const char *)found, tests[i].answer)) {
			printf("Searching for key \"%s\" in object:\n%s\n"
			    "Found: %s\nExpected: %s\n",
			    tests[i].tofind, tests[i].json,
			    found, tests[i].answer);
			printf("FAILURE\n");
			exit(1);
		}
	}

	/* Everything went fine. */
	printf("SUCCESS\n");
	exit(0);
}

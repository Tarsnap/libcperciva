#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sock_util.h"
#include "warnp.h"

static struct testcase_port {
	const char * input;
	const char * output;
} tests_port[] = {
	{ "/tmp/mysock", "/tmp/mysock" },
	{ "localhost", "localhost:0" },
	{ "localhost:80", "localhost:80" },
	{ "[127.0.0.1]", "[127.0.0.1]:0" },
	{ "[127.0.0.1]:80", "[127.0.0.1]:80" },
	{ "[::1]", "[::1]:0" },
	{ "[::1]:80", "[::1]:80" },
	/* Check that even badly-formed strings are handled. */
	{ ":", ":" },
	{ "", ":0" },
};

static int
check_ensure_port(void)
{
	size_t i;
	const char * in;
	const char * expected;
	char * out;

	for (i = 0; i < sizeof(tests_port) / sizeof(tests_port[0]); i++) {
		/* Get test data. */
		in = tests_port[i].input;
		expected = tests_port[i].output;

		/* Run the function and check its output. */
		out = sock_addr_ensure_port(in);
		if (strcmp(out, expected) != 0) {
			warn0("Failed; sock_addr_ensure_port(\"%s\") "
			    "expected \"%s\", got \"%s\"", in, expected, out);
			goto err1;
		}

		/* Clean up. */
		free(out);
	}

	/* Success! */
	return (0);

err1:
	free(out);

	/* Failure! */
	return (-1);
}

int
main(int argc, char * argv[])
{

	WARNP_INIT;
	(void)argc; /* UNUSED */

	if (check_ensure_port())
		goto err0;

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

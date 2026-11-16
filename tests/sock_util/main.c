#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sock.h"
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
	/* Check that even badly-formed strings don't cause a crash. */
	{ ":", ":" },
	{ "", ":0" },
};

/* This checks sock_resolve_one() and sock_addr_prettyprint(). */
static struct testcase_print {
	const char * input;
	const char * output;
} tests_print[] = {
	{ "/tmp/mysock", "/tmp/mysock" },
	{ "[127.0.0.1]", "[127.0.0.1]:0" },
	{ "[127.0.0.1]:80", "[127.0.0.1]:80" },
	{ "[::1]", "[::1]:0" },
	{ "[::1]:80", "[::1]:80" },
	/* Check that badly-formed strings are rejected without a crash. */
	{ ":", NULL },
	{ "", NULL },
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
		if ((out = sock_addr_ensure_port(in)) == NULL) {
			warnp("sock_addr_ensure_port(\"%s\")", in);
			goto err0;
		}
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
err0:
	/* Failure! */
	return (-1);
}

static int
check_prettyprint(void)
{
	size_t i;
	const char * in;
	const char * expected;
	struct sock_addr * sa;
	char * out;

	for (i = 0; i < sizeof(tests_print) / sizeof(tests_print[0]); i++) {
		/* Get test data and clear output. */
		in = tests_print[i].input;
		expected = tests_print[i].output;
		out = NULL;

		/* Resolve the address. */
		sa = sock_resolve_one(in, 1);

		/* Handle "expected to fail" cases. */
		if (expected == NULL) {
			/* Good, it matches; continue to the next test. */
			if (sa == NULL)
				continue;

			/* Bad: we shouldn't have an sa. */
			warn0("sock_resolve_one(%s) failed to fail", in);
			goto err1;
		}

		/* Check that we have an sa. */
		if (sa == NULL) {
			warn0("sock_resolve_one(%s) failed", in);
			goto err1;
		}

		/* Print the address. */
		if ((out = sock_addr_prettyprint(sa)) == NULL) {
			warnp("sock_addr_prettyprint(\"%s\")", in);
			goto err1;
		}

		/* Check the address. */
		if (strcmp(out, expected) != 0) {
			warn0("Failed; sock_addr_prettyprint(\"%s\") "
			    "expected \"%s\", got \"%s\"", in,
			    expected, out);
			goto err1;
		}

		/* Clean up. */
		sock_addr_free(sa);
		free(out);
	}

	/* Success! */
	return (0);

err1:
	sock_addr_free(sa);
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

	if (check_prettyprint())
		goto err0;

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

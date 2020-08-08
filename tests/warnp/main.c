#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "parsenum.h"
#include "warnp.h"

/* Print a 300-character message. */
static void
print_long_message(void)
{
	char message[301];
	int i, j;

	/* Construct message: "a_________b_________c_" up to "_~_________". */
	for (i = 0; i < 30; i++) {
		message[10*i] = 'a' + (char)i;
		for (j = 1; j < 10; j++) {
			message[10*i + j] = '_';
		}
	}
	message[300] = '\0';

	/* Print long message. */
	warn0(message);
}

static void
print_stuff(const char * extra_message)
{

	/* Output the nonce word; also warn0 with a formatted message. */
	warn0("begin output, nonce=%s", extra_message);

	/* Output NULL. */
	warn0(NULL);

	/* Do errno=2 first, so that we don't repeat the same message. */
	errno = 2;
	warnp(NULL);
	errno = 0;
	warnp(NULL);

	/* Output messages involving warnp with a formatted message. */
	errno = 0;
	warnp("warnp errno %i", 0);

	errno = 2;
	warnp("warnp errno %i", 2);

	/* Output long message. */
	print_long_message();
}

static void
check_stderr_syslog(const char * extra_message)
{

	/* Print messages. */
	print_stuff(extra_message);

	/* Switch to syslog. */
	warnp_syslog(1);
	print_stuff(extra_message);

	/* Back to stderr. */
	warnp_syslog(0);
	warn0("back to stderr");
}

int
main(int argc, char * argv[])
{
	const char * extra_message;
	int desired_test;

	WARNP_INIT;

	/* Parse command-line argument. */
	if (argc != 3) {
		fprintf(stderr, "usage: test_warnp STRING NUM\n");
		exit(1);
	}
	extra_message = argv[1];
	if (PARSENUM(&desired_test, argv[2], 1, 1)) {
		warnp("parsenum");
		goto err0;
	}

	/* Run the desired test. */
	switch(desired_test) {
	case 1:
		check_stderr_syslog(extra_message);
		break;
	default:
		warn0("invalid test number");
		goto err0;
	}

	/* Switch to syslog again. */
	warnp_syslog(1);
	warn0("syslog again");

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

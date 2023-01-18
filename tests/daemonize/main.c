#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "daemonize.h"
#include "warnp.h"

int
main(int argc, char * argv[])
{
	const char * pidfile;

	/*
	 * This will allocate some memory for the program name, which will be
	 * freed by an atexit() function.  The non-daemonized fork of this
	 * program will call _exit() to quit without calling that function.
	 * This might cause a memory checker to issue an invalid warning
	 * which should be suppressed.
	 */
	WARNP_INIT;

	/* Parse command-line argument. */
	if (argc != 2) {
		fprintf(stderr, "usage: test_daemonize pidfile\n");
		goto err0;
	}
	pidfile = argv[1];

	/* Print current pid. */
	warn0("before daemonize, pid:\t%jd", (intmax_t)getpid());

	/* Launch daemon. */
	if (daemonize(pidfile))
		goto err0;

	/* Print current pid. */
	warn0("after daemonize, pid:\t%jd", (intmax_t)getpid());

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

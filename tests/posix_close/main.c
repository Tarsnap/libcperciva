#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parsenum.h"
#include "posix_close.h"
#include "warnp.h"

#include "open_close.h"
#include "send_alarms.h"

int
main(int argc, char * argv[])
{
	const char * filename;
	int numbytes;
	volatile int stop_open_close = 0;
	volatile int stop_alarms = 0;
	int rc;
	pthread_t open_close_thread;
	pthread_t send_alarms_thread;

	WARNP_INIT;

	/* Process arguments. */
	if (argc != 3) {
		fprintf(stderr, "usage: test_posix_close filename numbytes\n");
		goto err0;
	}
	filename = argv[1];
	if (PARSENUM(&numbytes, argv[2], 0, 1)) {
		warn0("%s should be 0 or 1", argv[2]);
		goto err0;
	}

	/* Open, read 0 or 1 bytes, then close the file. */
	if (start_repeated_open_close(filename, numbytes, &stop_open_close,
	    &open_close_thread))
		goto err0;

	warn0("sleep for first thread to start");
	fflush(stdout);
	sleep(1);

	/* Send SIGALRM to ${open_close_thread}. */
	if (start_alarms(open_close_thread, &stop_alarms,
	    &send_alarms_thread))
		goto err0;

	warn0("sleep for 5 seconds...");
	sleep(5);
	warn0("... sleep done");

	/* Stop threads in order. */
	stop_alarms = 1;
	if ((rc = pthread_join(send_alarms_thread, NULL)) != 0) {
		warn0("pthread_join: %s", strerror(rc));
		goto err0;
	}
	stop_open_close = 1;
	if ((rc = pthread_join(open_close_thread, NULL)) != 0) {
		warn0("pthread_join: %s", strerror(rc));
		goto err0;
	}

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

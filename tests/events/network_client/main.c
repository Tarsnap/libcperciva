#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <stdio.h>

#include "events.h"
#include "parsenum.h"
#include "sock.h"
#include "warnp.h"

static int done;

static int
poke(struct sock_addr ** sas)
{
	int client_sock;

	/* Attempt to connect, but don't complain about any failures. */
	if ((client_sock = sock_connect(sas)) == -1)
		goto err0;

	/* Don't send anything; just close the connection. */
	if (close(client_sock)) {
		warnp("close");
		goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/* Wait duration can be interrupted by signals. */
static int
wait_ms(size_t msec)
{
	struct timespec ts;

	ts.tv_sec = msec / 1000;
	ts.tv_nsec = (msec % 1000) * 1000000;
	return (nanosleep(&ts, NULL));
}

static int
repeated_poker(struct sock_addr ** sas)
{

	/* "Poke" the server multiple times. */
	do {
		/*
		 * We expect this to work a few times, then stop working
		 * because the server shut itself down.  Verification will be
		 * done via the logfile, so don't warn about errors here.
		 */
		if (poke(sas))
			goto serverdone;

		/* Wait a bit. */
		if (wait_ms(100)) {
			warnp("nanosleep");
			goto err0;
		}
	} while (!done);

serverdone:
	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

static int
long_poke(struct sock_addr ** sas, pid_t server_pid)
{

	/* Poke the server once. */
	if (poke(sas)) {
		warn0("poke");
		goto err0;
	}

	/* Wait for the server to start its poll(). */
	wait_ms(100);

	/* Send a shutdown signal to the server. */
	if (kill(server_pid, SIGUSR1)) {
		warnp("kill");
		goto err0;
	}

	/*
	 * Don't send another poke, as that would activate the server's
	 * poll().  The server should have shut itself down at this point.
	 */

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/* Parse a pid_t; it's a signed integer and there's no PID_MAX. */
static pid_t
str_to_pid(const char * str)
{
	intmax_t pid;

	/* Parse number; a negative pid_t indicates an error so omit that. */
	if (PARSENUM(&pid, str, 0, INTMAX_MAX)) {
		warnp("parsenum");
		goto err0;
	}

	/* Check that it fits into pid_t. */
	if (pid != ((intmax_t)((pid_t)pid))) {
		warn0("%jd does not fit into (pid_t)", pid);
	}

	/* Success! */
	return ((pid_t)pid);

err0:
	/* Failure! */
	return (-1);
}

int
main(int argc, char * argv[])
{
	const char * addr;
	struct sock_addr ** sas;
	pid_t server_pid;
	size_t desired_test;

	WARNP_INIT;

	/* Parse command-line arguments. */
	if (argc != 4) {
		fprintf(stderr, "Usage: %s ADDR SERVER_PID TEST_NUM\n",
		    argv[0]);
		goto err0;
	}
	addr = argv[1];
	if ((server_pid = str_to_pid(argv[2])) == -1) {
		goto err0;
	}
	if (PARSENUM(&desired_test, argv[3], 1, 2)) {
		warnp("parsenum");
		goto err0;
	}

	/* Resolve the address. */
	if ((sas = sock_resolve(addr)) == NULL) {
		warn0("sock_resolve");
		goto err0;
	}

	/* Run desired test. */
	switch(desired_test) {
	case 1:
		if (repeated_poker(sas)) {
			warn0("repeated_poker");
			goto err0;
		}
		break;
	case 2:
		if (long_poke(sas, server_pid)) {
			warn0("long_poker");
			goto err0;
		}
		break;
	default:
		warn0("Invalid test number");
		goto err0;
	}

	/* Clean up.  */
	sock_addr_freelist(sas);

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

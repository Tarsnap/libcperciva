#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "events.h"
#include "sock.h"
#include "warnp.h"

#include "events_counter.h"
#include "events_interrupter.h"

/* "Cookie" from registering a network event. */
static int event_network_id;

/* Server management structure. */
struct server_state {
	int fd;
};

static int
event_network(void * cookie)
{
	struct server_state * S = cookie;
	int s;

	/* Accept and close connection. */
	if ((s = accept(S->fd, NULL, NULL)) == -1) {
		warnp("accept");
		goto err0;
	}
	if (close(s))
		warnp("close");

	/* Start new event. */
	if ((event_network_id = events_network_register(&event_network, S,
	    S->fd, EVENTS_NETWORK_OP_READ)) == -1) {
		warn0("events_network_register");
		goto err0;
	}

	/* Display count, increment, check to see if we should interrupt. */
	events_counter_ping();

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

static int
launch_server(struct sock_addr ** sas)
{
	struct server_state * S;
	int done;
	int ret;

	/* Ensure the counter is 0. */
	events_counter_reset();

	/* Configure SIGUSR1 to cancel the loop. */
	if (events_interrupter_init()) {
		warnp("events_interrupter_init");
		goto err0;
	}

	/* Bake a cookie for the server. */
	if ((S = malloc(sizeof(struct server_state))) == NULL) {
		warnp("malloc");
		goto err0;
	}

	/* Open a socket. */
	if ((S->fd = sock_listener(sas[0])) == -1) {
		warn0("sock_listener");
		goto err1;
	}

	/* Queue an event. */
	if ((event_network_id = events_network_register(&event_network, S,
	    S->fd, EVENTS_NETWORK_OP_READ)) == -1) {
		warn0("events_network_register");
		goto err1;
	}

	/* Run event loop. */
	done = 0;
	while ((ret = events_spin(&done))) {
		if (ret == -1) {
			warnp("error in event loop");
			goto err1;
		}
	}

	/* Clean up. */
	if (events_network_cancel(S->fd, event_network_id)) {
		warnp("events_network_cancel");
		goto err1;
	}
	free(S);

	/* Success! */
	return (0);

err1:
	free(S);
err0:
	/* Failure! */
	return (-1);
}

int
main(int argc, char * argv[])
{
	const char * addr;
	struct sock_addr ** sas;

	WARNP_INIT;

	/* Check arguments. */
	if (argc != 2) {
		fprintf(stderr, "Usage: %s ADDR\n", argv[0]);
		goto err0;
	}
	addr = argv[1];

	/* Resolve the address. */
	if ((sas = sock_resolve(addr)) == NULL) {
		warn0("sock_resolve");
		goto err0;
	}

	/* Launch server; blocks until it's finished. */
	if (launch_server(sas)) {
		warn0("launch_server");
		goto err0;
	}

	/* Clean up. */
	sock_addr_freelist(sas);

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "events.h"
#include "warnp.h"

#define INTERRUPT_AT 4
#define INTERRUPT_BAIL 10

static int event_count = 0;
static void * event_cookie = NULL;

static int
event(void * cookie)
{

	(void)cookie; /* UNUSED */

	/* Display debug info. */
	printf("event %i\n", event_count);

	/* Register new event; infinite loop unless we... */
	event_cookie = events_immediate_register(&event, NULL, 10);
	event_count++;

	/* ... quit via interrupting, or... */
	if (event_count == INTERRUPT_AT)
		raise(SIGUSR1);

	/* ... have an emergency exit. */
	if (event_count > INTERRUPT_BAIL) {
		warn0("events_interrupt() is broken.");
		exit(1);
	}

	/* Success! */
	return (0);
}

static void
interrupt(int sig)
{

	(void)sig; /* UNUSED */

	events_interrupt();
}

int
main(int argc, char * argv[])
{
	struct sigaction sa;
	int ret;
	int done = 0;

	WARNP_INIT;

	(void)argc;
	(void)argv;

	/* Configure SIGUSR1 to cancel the loop. */
	sa.sa_handler = interrupt;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGUSR1, &sa, NULL))
		goto err0;

	/* Queue an event. */
	event_cookie = events_immediate_register(&event, NULL, 10);

	/* Run event loop. */
	while ((ret = events_run())) {
		if (ret == -1) {
			warnp("error in event loop");
			goto err0;
		}
	}

	/* Cancel the left-over event, then reset the loop counter. */
	events_immediate_cancel(event_cookie);
	printf("--- reset event loop ---\n");
	event_count = 0;

	/* Queue an event again and initialize count. */
	event_cookie = events_immediate_register(&event, NULL, 10);

	/* Run event loop again. */
	while ((ret = events_spin(&done))) {
		if (ret == -1) {
			warnp("error in event loop");
			goto err0;
		}
	}

	/* Cancel the left-over event. */
	events_immediate_cancel(event_cookie);

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

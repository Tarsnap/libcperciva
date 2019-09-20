#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "events.h"
#include "warnp.h"

#include "events_counter.h"

/* Variable for event testing in general. */
static void * event_cookie = NULL;

/* Variable specifically for events_run(). */
static int events_run_done = 0;

static int
event(void * cookie)
{

	(void)cookie; /* UNUSED */

	/* Register new event; infinite loop unless it's interrupted. */
	event_cookie = events_immediate_register(&event, NULL, 10);

	/* Display count, increment, check to see if we should interrupt. */
	events_counter_ping();

	/* Success! */
	return (0);
}

static void
interrupt(int sig)
{

	(void)sig; /* UNUSED */

	events_interrupt();
	events_run_done = 1;
}

static int
test_interrupt_run()
{

	/* Reset counter. */
	events_counter_reset();

	/* Queue an event. */
	event_cookie = events_immediate_register(&event, NULL, 10);

	/* Run event loop. */
	do {
		if (events_run()) {
			warnp("error in event loop");
			goto err0;
		}
	} while (!events_run_done);

	/* Cancel the left-over event. */
	events_immediate_cancel(event_cookie);

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

static int
test_interrupt_spin()
{
	int done = 0;
	int ret;

	/* Reset counter. */
	events_counter_reset();

	/* Queue an event. */
	event_cookie = events_immediate_register(&event, NULL, 10);

	/* Run event loop. */
	while ((ret = events_spin(&done))) {
		if (ret == -1) {
			warnp("error in event loop");
			goto err0;
		}
	}

	/* Cancel the left-over event. */
	events_immediate_cancel(event_cookie);

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

static int
event_timer(void * cookie)
{

	(void)cookie; /* UNUSED */

	/* Register new event; infinite loop unless it's interrupted. */
	event_cookie = events_timer_register_double(&event_timer, NULL, 0.01);

	/* Display count, increment, check to see if we should interrupt. */
	events_counter_ping();

	/* Success! */
	return (0);
}

static int
test_timer()
{
	int done = 0;
	int ret;

	/* Reset counter. */
	events_counter_reset();

	/* Queue an event. */
	event_cookie = events_timer_register_double(&event_timer, NULL, 0.01);

	/* Run event loop. */
	while ((ret = events_spin(&done))) {
		if (ret == -1) {
			warnp("error in event loop");
			goto err0;
		}
	}

	/* Cancel the left-over event. */
	events_timer_cancel(event_cookie);

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

int
main(int argc, char * argv[])
{
	struct sigaction sa;

	WARNP_INIT;

	(void)argc;	/* UNUSED */

	/* Configure SIGUSR1 to cancel the loop. */
	sa.sa_handler = interrupt;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGUSR1, &sa, NULL))
		goto err0;

	/* Run tests. */
	if (test_interrupt_run())
		goto err0;
	if (test_interrupt_spin())
		goto err0;
	if (test_timer())
		goto err0;

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

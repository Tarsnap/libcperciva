#include <stdio.h>
#include <stdlib.h>

#include "events.h"
#include "warnp.h"

#include "events_counter.h"
#include "events_interrupter.h"

/* Variable for event testing in general. */
static void * event_cookie = NULL;

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
	} while (!events_interrupter_interrupted());

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

	WARNP_INIT;

	(void)argc;	/* UNUSED */

	/* Set up response to SIGUSR1. */
	if (events_interrupter_init()) {
		warnp("events_interrupter_init()");
		goto err0;
	}

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

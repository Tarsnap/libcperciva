#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "warnp.h"

#include "events_counter.h"

#define INTERRUPT_AT 4
#define INTERRUPT_BAIL 10

static int events_counter = 0;

/**
 * events_counter_reset(void):
 * Reset the event counter and print a message to that effect.
 */
void
events_counter_reset(void)
{

	events_counter = 0;
	printf("--- reset event counter ---\n");
}

/**
 * events_counter_ping(void):
 * Print the current count, increment, and check to see if we should raise
 * SIGUSR1 due to reaching the desired number of events.
 */
void
events_counter_ping(void)
{

	/* Print the current count. */
	printf("event %i\n", events_counter);

	/* Increment. */
	events_counter++;

	/* Check to see if we should interrupt the events. */
	if (events_counter == INTERRUPT_AT) {
		if (raise(SIGUSR1)) {
			warnp("raise(SIGUSR1) failed");
			exit(1);
		}
	}

	/* Emergency exit for runaway loops. */
	if (events_counter > INTERRUPT_BAIL) {
		warn0("events_interrupt() is broken.");
		exit(1);
	}
}

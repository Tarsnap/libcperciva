#include <signal.h>
#include <stddef.h>

#include "events.h"
#include "warnp.h"

#include "events_interrupter.h"

/* This is only used for special testing of events_run. */
static volatile sig_atomic_t interrupted;

static void
interrupt(int sig)
{

	(void)sig; /* UNUSED */

	events_interrupt();

	/* Extra flag for a special testing case. */
	interrupted = 1;
}

/**
 * events_interrupter_init():
 * Set up a signal handler to run events_interrupt() upon receiving SIGUSR1.
 */
int
events_interrupter_init(void)
{
	struct sigaction sa;

	/* Clear the flag. */
	events_interrupter_reset();

	/* Set up the SIGUSR1 handler. */
	sa.sa_handler = interrupt;
	if (sigemptyset(&sa.sa_mask)) {
		warnp("siginfo");
		goto err0;
	}
	sa.sa_flags = 0;
	if (sigaction(SIGUSR1, &sa, NULL)) {
		warnp("sigaction");
		goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

/*
 * events_interrupter_interrupted(void):
 * Return 1 if we received SIGUSR1; else return 0.
 */
int
events_interrupter_interrupted(void)
{

	return ((int)interrupted);
}

/*
 * events_interrupter_reset(void):
 * Clear the "did we receive SIGUSR1" flag.
 */
void
events_interrupter_reset(void)
{

	interrupted = 0;
}

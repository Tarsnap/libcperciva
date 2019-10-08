#ifndef _EVENTS_INTERRUPTER_H_
#define _EVENTS_INTERRUPTER_H_

/**
 * events_interrupter_init():
 * Set up a signal handler to run events_interrupt() upon receiving SIGUSR1.
 */
int events_interrupter_init(void);

/*
 * events_interrupter_interrupted(void):
 * Return 1 if we received SIGUSR1; else return 0.
 */
int events_interrupter_interrupted(void);

/*
 * events_interrupter_reset(void):
 * Clear the "did we receive SIGUSR1" flag.
 */
void events_interrupter_reset(void);

#endif /* !_EVENTS_INTERRUPTER_H_ */

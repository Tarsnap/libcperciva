#ifndef EVENTS_COUNTER_H_
#define EVENTS_COUNTER_H_

/**
 * events_counter_reset(void):
 * Reset the event counter and print a message to that effect.
 */
void events_counter_reset(void);

/**
 * events_counter_ping(void):
 * Print the current count, increment, and check to see if we should raise
 * SIGUSR1 due to reaching the desired number of events.
 */
void events_counter_ping(void);

#endif /* !EVENTS_COUNTER_H_ */

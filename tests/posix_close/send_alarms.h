#ifndef _SEND_ALARMS_H
#define _SEND_ALARMS_H

#include <pthread.h>

/*
 * start_alarms(to_thr, stop, thr):
 * Start a thread, stored in ${thr}, which repeatedly sends SIGALRM to
 * ${to_thr}.  Stop when ${stop} is non-zero.
 */
int start_alarms(pthread_t, volatile int *, pthread_t *);

#endif /* !_SEND_ALARMS_H_ */

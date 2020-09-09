#ifndef _OPEN_CLOSE_H
#define _OPEN_CLOSE_H

#include <pthread.h>

/*
 * start_repeated_open_close(filename, numbytes, stop, thr):
 * Start a thread, stored in ${thr}, to repeatedly open ${filename}, read
 * ${numbytes}, then close it.  Stop when ${*stop} is non-zero.  ${numbytes}
 * must be 0 or 1.  Catch SIGALRM.
 */
int start_repeated_open_close(const char *, int, volatile int * stop,
    pthread_t *);

#endif /* !_OPEN_CLOSE_H_ */

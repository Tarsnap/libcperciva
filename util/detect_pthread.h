#ifndef _DETECT_PTHREAD_H_
#define _DETECT_PTHREAD_H_

extern int detect_pthread_has_pthread;

/**
 * DETECT_PTHREAD_INIT:
 * Initialize detecting whether this binary is linked to pthread or not.
 */
#define DETECT_PTHREAD_INIT	pthread_testcancel()

/*
 * detect_pthread(void):
 * Return non-zero if pthread is linked.
 */
int detect_pthread(void);

/*
 * Internal use: this is an arbitrary pthread function which does not use any
 * special pthread types.
 */
void pthread_testcancel(void);

#endif /* !_DETECT_PTHREAD_H_ */

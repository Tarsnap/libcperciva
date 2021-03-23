#include "detect_pthread.h"

/*
 * Assume that we have pthread.  If we don't have pthread, this value will be
 * modified.
 */
int detect_pthread_has_pthread = 1;

/*
 * detect_pthread(void):
 * Return non-zero if pthread is linked.
 */
int
detect_pthread(void)
{

	return (detect_pthread_has_pthread);
}

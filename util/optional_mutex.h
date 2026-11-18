#ifndef OPTIONAL_MUTEX_H_
#define OPTIONAL_MUTEX_H_

/*
 * This header must be agnostic as to whether the program will be linked to
 * pthread or not, so that code including it compiles identically either way.
 * Regardless of the linking, we need to include <pthread.h> to get the
 * required data type.
 */
#include <pthread.h>

/**
 * optional_mutex_lock(mutex):
 * If the program was linked to pthread, call pthread_mutex_lock(${mutex})
 * and return its result.  Otherwise, do nothing and return 0.
 */
int optional_mutex_lock(pthread_mutex_t *);

/**
 * optional_mutex_unlock(mutex):
 * If the program was linked to pthread, call pthread_mutex_unlock(${mutex})
 * and return its result.  Otherwise, do nothing and return 0.
 */
int optional_mutex_unlock(pthread_mutex_t *);

#endif /* !OPTIONAL_MUTEX_H_ */

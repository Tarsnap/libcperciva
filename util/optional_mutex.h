#ifndef OPTIONAL_MUTEX_H_
#define OPTIONAL_MUTEX_H_

/**
 * optional_mutex_alloc(mutex_p):
 * If OPTIONAL_MUTEX_PTHREAD_YES is defined, allocate memory for a mutex to
 * ${mutex_p}; if OPTIONAL_MUTEX_PTHREAD_NO is defined, do nothing.  If
 * successful, return zero; otherwise, return non-zero and set errno.
 */
int optional_mutex_alloc(void **);

/**
 * optional_mutex_init(mutex):
 * If OPTIONAL_MUTEX_PTHREAD_YES is defined, call pthread_mutex_init() with
 * default attributes; if OPTIONAL_MUTEX_PTHREAD_NO is defined, do nothing.
 * If successful, return zero; otherwise, return an error code.
 */
int optional_mutex_init(void *);

/**
 * optional_mutex_lock(mutex):
 * If OPTIONAL_MUTEX_PTHREAD_YES is defined, call pthread_mutex_lock();
 * if OPTIONAL_MUTEX_PTHREAD_NO is defined, do nothing.  If successful, return
 * zero; otherwise, return an error code.
 */
int optional_mutex_lock(void *);

/**
 * optional_mutex_unlock(mutex):
 * If OPTIONAL_MUTEX_PTHREAD_YES is defined, call pthread_mutex_unlock(); if
 * OPTIONAL_MUTEX_PTHREAD_NO is defined, do nothing.  If successful, return
 * zero; otherwise, return an error code.
 */
int optional_mutex_unlock(void *);

/**
 * optional_mutex_destroy(mutex):
 * If OPTIONAL_MUTEX_PTHREAD_YES is defined, call pthread_mutex_destroy() and
 * then free the ${mutex}; if OPTIONAL_MUTEX_PTHREAD_NO is defined, do
 * nothing.  If successful, return zero; otherwise, return an error code.
 */
int optional_mutex_destroy(void *);

/**
 * optional_mutex_free(mutex):
 * If OPTIONAL_MUTEX_PTHREAD_YES is defined, free the ${mutex}; if
 * OPTIONAL_MUTEX_PTHREAD_NO is defined, do nothing.
 */
void optional_mutex_free(void *);

#endif /* !OPTIONAL_MUTEX_H_ */

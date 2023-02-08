#ifdef OPTIONAL_MUTEX_PTHREAD_YES
#include <pthread.h>
#include <stdlib.h>
#endif

#include "optional_mutex.h"

/* Sanity check OPTIONAL_MUTEX_PTHREAD_* defines. */
#if defined(OPTIONAL_MUTEX_PTHREAD_YES) && defined(OPTIONAL_MUTEX_PTHREAD_NO)
#error "You must not define both OPTIONAL_MUTEX_PTHREAD_YES "	\
    "and OPTIONAL_MUTEX_PTHREAD_NO"
#elif !(defined(OPTIONAL_MUTEX_PTHREAD_YES) ||			\
    defined(OPTIONAL_MUTEX_PTHREAD_NO))
#error "You must define either OPTIONAL_MUTEX_PTHREAD_YES "	\
    "or OPTIONAL_MUTEX_PTHREAD_NO"
#endif

#ifdef OPTIONAL_MUTEX_PTHREAD_YES
/* Clang normally warns if you have a mutex held at the end of a function. */
#ifdef __clang__
_Pragma("clang diagnostic push")
_Pragma("clang diagnostic ignored \"-Wthread-safety-analysis\"")
#endif
#endif

/**
 * optional_mutex_alloc(mutex_p):
 * If OPTIONAL_MUTEX_PTHREAD_YES is defined, allocate memory for a mutex to
 * ${mutex_p}; if OPTIONAL_MUTEX_PTHREAD_NO is defined, do nothing.  If
 * successful, return zero; otherwise, return non-zero and set errno.
 */
int
optional_mutex_alloc(void ** mutex_p)
{

#ifdef OPTIONAL_MUTEX_PTHREAD_YES
	/* Allocate memory for a mutex. */
	if ((*mutex_p = malloc(sizeof(pthread_mutex_t))) == NULL)
		return (-1);

	/* Success! */
	return (0);
#else
	(void)mutex_p; /* UNUSED */
	return (0);
#endif
}

/**
 * optional_mutex_init(mutex):
 * If OPTIONAL_MUTEX_PTHREAD_YES is defined, call pthread_mutex_init() with
 * default attributes; if OPTIONAL_MUTEX_PTHREAD_NO is defined, do nothing.
 * If successful, return zero; otherwise, return an error code.
 */
int
optional_mutex_init(void * mutex)
{

#ifdef OPTIONAL_MUTEX_PTHREAD_YES
	return (pthread_mutex_init(mutex, NULL));
#else
	(void)mutex; /* UNUSED */
	return (0);
#endif
}

/**
 * optional_mutex_lock(mutex):
 * If OPTIONAL_MUTEX_PTHREAD_YES is defined, call pthread_mutex_lock();
 * if OPTIONAL_MUTEX_PTHREAD_NO is defined, do nothing.  If successful, return
 * zero; otherwise, return an error code.
 */
int
optional_mutex_lock(void * mutex)
{

#ifdef OPTIONAL_MUTEX_PTHREAD_YES
	return (pthread_mutex_lock(mutex));
#else
	(void)mutex; /* UNUSED */
	return (0);
#endif
}

/**
 * optional_mutex_unlock(mutex):
 * If OPTIONAL_MUTEX_PTHREAD_YES is defined, call pthread_mutex_unlock(); if
 * OPTIONAL_MUTEX_PTHREAD_NO is defined, do nothing.  If successful, return
 * zero; otherwise, return an error code.
 */
int
optional_mutex_unlock(void * mutex)
{

#ifdef OPTIONAL_MUTEX_PTHREAD_YES
	return (pthread_mutex_unlock(mutex));
#else
	(void)mutex; /* UNUSED */
	return (0);
#endif
}

/**
 * optional_mutex_destroy(mutex):
 * If OPTIONAL_MUTEX_PTHREAD_YES is defined, call pthread_mutex_destroy() and
 * then free the ${mutex}; if OPTIONAL_MUTEX_PTHREAD_NO is defined, do
 * nothing.  If successful, return zero; otherwise, return an error code.
 */
int
optional_mutex_destroy(void * mutex)
{

#ifdef OPTIONAL_MUTEX_PTHREAD_YES
	return (pthread_mutex_destroy(mutex));
#else
	(void)mutex; /* UNUSED */
	return (0);
#endif
}

/**
 * optional_mutex_free(mutex):
 * If OPTIONAL_MUTEX_PTHREAD_YES is defined, free the ${mutex}; if
 * OPTIONAL_MUTEX_PTHREAD_NO is defined, do nothing.
 */
void
optional_mutex_free(void * mutex)
{

#ifdef OPTIONAL_MUTEX_PTHREAD_YES
	free(mutex);
#else
	(void)mutex; /* UNUSED */
#endif
}

#ifdef OPTIONAL_MUTEX_PTHREAD_YES
#ifdef __clang__
_Pragma("clang diagnostic pop")
#endif
#endif

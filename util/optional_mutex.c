#include <pthread.h>

#include "optional_mutex.h"

/* Sanity check OPTIONAL_PTHREAD_* defines. */
#if defined(OPTIONAL_PTHREAD_YES) && defined(OPTIONAL_PTHREAD_NO)
#error "You must not define both OPTIONAL_PTHREAD_YES and OPTIONAL_PTHREAD_NO"
#elif !(defined(OPTIONAL_PTHREAD_YES) || defined(OPTIONAL_PTHREAD_NO))
#error "You must define either OPTIONAL_PTHREAD_YES or OPTIONAL_PTHREAD_NO"
#endif

#ifdef OPTIONAL_PTHREAD_YES
/*
 * Clang warns if you have a mutex held at the end of a function, or if you
 * try to unlock a mutex that it thinks isn't held.
 */
#ifdef __clang__
_Pragma("clang diagnostic push")
_Pragma("clang diagnostic ignored \"-Wthread-safety-analysis\"")
#endif
#endif

/**
 * optional_mutex_lock(mutex):
 * If the program was linked to pthread, call pthread_mutex_lock(${mutex})
 * and return its result.  Otherwise, do nothing and return 0.
 */
int
optional_mutex_lock(pthread_mutex_t * mutex)
{

#ifdef OPTIONAL_PTHREAD_YES
	/* Mutexes are enabled; call the pthread function. */
	return (pthread_mutex_lock(mutex));
#else
	(void)mutex; /* UNUSED */

	/* Do nothing. */
	return (0);
#endif
}

/**
 * optional_mutex_unlock(mutex):
 * If the program was linked to pthread, call pthread_mutex_unlock(${mutex})
 * and return its result.  Otherwise, do nothing and return 0.
 */
int
optional_mutex_unlock(pthread_mutex_t * mutex)
{

#ifdef OPTIONAL_PTHREAD_YES
	/* Mutexes are enabled; call the pthread function. */
	return (pthread_mutex_unlock(mutex));
#else
	(void)mutex; /* UNUSED */

	/* Do nothing. */
	return (0);
#endif
}

#ifdef OPTIONAL_PTHREAD_YES
#ifdef __clang__
_Pragma("clang diagnostic pop")
#endif
#endif

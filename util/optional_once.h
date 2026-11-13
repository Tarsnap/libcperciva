#ifndef OPTIONAL_ONCE_H_
#define OPTIONAL_ONCE_H_

#ifdef OPTIONAL_PTHREAD_YES
#include <pthread.h>
#endif

/* Sanity check OPTIONAL_PTHREAD_* defines. */
#if defined(OPTIONAL_PTHREAD_YES) && defined(OPTIONAL_PTHREAD_NO)
#error You must not define both OPTIONAL_PTHREAD_YES and OPTIONAL_PTHREAD_NO
#elif !(defined(OPTIONAL_PTHREAD_YES) || defined(OPTIONAL_PTHREAD_NO))
#error You must define either OPTIONAL_PTHREAD_YES or OPTIONAL_PTHREAD_NO
#endif

/*
 * pthread_once_t is opaque, so we can't use it ourselves if we're not linking
 * to pthread.  Define a compatibility type and initializer which handles both
 * _YES and _NO.
 */
#ifdef OPTIONAL_PTHREAD_YES
typedef pthread_once_t optional_once_t;
#define OPTIONAL_ONCE_INIT PTHREAD_ONCE_INIT
#else
typedef int optional_once_t;
#define OPTIONAL_ONCE_INIT 0
#endif

/**
 * optional_once(once_control, init_routine):
 * If OPTIONAL_PTHREAD_YES is defined, call pthread_once().
 * If OPTIONAL_PTHREAD_NO is defined, if this is the first call with a given
 * ${once_control}, run the ${init_routine}; on subsequent calls, do nothing.
 * In the latter case, this function is not thread-safe.
 */
int optional_once(optional_once_t *, void (*)(void));

#endif /* !OPTIONAL_ONCE_H_ */

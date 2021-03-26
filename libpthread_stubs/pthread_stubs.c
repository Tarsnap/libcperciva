#include <pthread.h>

/* Work around a safety check in clang -- we're replacing these functions. */
#ifdef __clang__
_Pragma("clang diagnostic push")
_Pragma("clang diagnostic ignored \"-Wthread-safety-analysis\"")
#endif

int
pthread_mutex_lock(pthread_mutex_t *mutex)
{

	(void)mutex; /* UNUSED */

	/* Success! */
	return (0);
}

int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{

	(void)mutex; /* UNUSED */

	/* Success! */
	return (0);
}

/* Work around a safety check in clang. */
#ifdef __clang__
_Pragma("clang diagnostic pop")
#endif

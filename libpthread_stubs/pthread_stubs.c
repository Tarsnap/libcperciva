#include <pthread.h>

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

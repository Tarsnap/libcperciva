#ifdef OPTIONAL_PTHREAD_YES
#include <pthread.h>
#endif

#include "optional_once.h"

/**
 * optional_once(once_control, init_routine):
 * If OPTIONAL_PTHREAD_YES is defined, call pthread_once().
 * If OPTIONAL_PTHREAD_NO is defined, if this is the first call with a given
 * ${once_control}, run the ${init_routine}; on subsequent calls, do nothing.
 * In the latter case, this function is not thread-safe.
 */
int
optional_once(optional_once_t * once_control, void (* init_routine)(void))
{

#ifdef OPTIONAL_PTHREAD_YES
	/* We're using pthread; call pthread_once(). */
	return (pthread_once(once_control, init_routine));
#else
	/* Not using pthread; we can use a non-thread-safe method. */

	/* Bail if we've already run it. */
	if (*once_control != 0)
		return (0);

	/* Run it and record that fact. */
	init_routine();
	*once_control = 1;

	/* Done. */
	return (0);
#endif
}

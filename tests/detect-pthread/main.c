#include <stdio.h>

#include "detect_pthread.h"

int
main()
{

	DETECT_PTHREAD_INIT;

	printf("has_pthread: %i\n", detect_pthread());

	return (0);
}

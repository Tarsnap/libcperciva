#include <time.h>

int
main(void)
{
	struct timespec ts;

	/* Done! */
	return (clock_gettime(CLOCK_REALTIME, &ts));
}

#include <sys/time.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "warnp.h"

#include "monoclock.h"

int
main(int argc, char * argv[])
{
	struct timeval tv_wall, tv_cpu;
	double timer_resolution;

	WARNP_INIT;

	(void)argc; /* UNUSED */
	(void)argv; /* UNUSED */

	/* Get time and process CPU time. */
	if (monoclock_get(&tv_wall)) {
		warnp("monoclock_get()");
		goto err0;
	}
	if (monoclock_get_cputime(&tv_cpu)) {
		warnp("monoclock_get_cputime()");
		goto err0;
	}

	/* Get timer resolution. */
	if (monoclock_getres(&timer_resolution)) {
		warnp("monoclock_getres()");
		goto err0;
	}

	/* Display times. */
	printf("monoclock_get():\t\t%ju seconds,\t%06ji microseconds\n",
	    (uintmax_t)tv_wall.tv_sec, (intmax_t)tv_wall.tv_usec);
	printf("monoclock_get_cputime():\t%ju seconds,\t%06ji microseconds\n",
	    (uintmax_t)tv_cpu.tv_sec, (intmax_t)tv_cpu.tv_usec);
	printf("monoclock_getres():\t\t%g\n", timer_resolution);

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

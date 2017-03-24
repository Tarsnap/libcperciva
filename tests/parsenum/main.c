#include <stdint.h>
#include <stdio.h>

#include "parsenum.h"
#include "warnp.h"

#define PARSENUM_CMP(x, y)						\
	(fabs((double)x - (double)y) < 1 &&				\
	    fabs((double)x - (double)y) <= fabs((double)x + (double)y) * 1e-6)

#define TEST4_DO(str, var, min, max, target)			\
	var parsenum_x;						\
	int parsenum_ret;					\
	fprintf(stderr,						\
	    "Parsing \"%s\" as %s between %s and %s (incl.) yields "	\
	    target "... ", str, #var, #min, #max);		\
	parsenum_ret = PARSENUM(&parsenum_x, str, min, max);

#define CHECK_SUCCESS(target)					\
	if (parsenum_ret == 0) {				\
		if (PARSENUM_CMP(parsenum_x, target)) {		\
			fprintf(stderr, "PASSED!\n");		\
		} else {					\
			fprintf(stderr, "FAILED!\n");		\
			goto err0;				\
		}						\
	}

#define CHECK_FAILURE(target)					\
	if (parsenum_ret != 0) {				\
		if (errno == target)				\
			fprintf(stderr, "PASSED!\n");		\
		else						\
			fprintf(stderr, "FAILED!\n");		\
	}

#define TEST4_SUCCESS(str, var, min, max, target) do {		\
	TEST4_DO(str, var, min, max, #target);			\
	CHECK_SUCCESS(target);					\
} while (0)

#define TEST4_FAILURE(str, var, min, max, target) do {		\
	TEST4_DO(str, var, min, max, #target);			\
	CHECK_FAILURE(target);					\
} while (0)

int
main(int argc, char * argv[])
{
	double d = 0.0;
	size_t s = 0;
	int i = 0;

	(void)argc; /* UNUSED */
	(void)argv; /* UNUSED */

	WARNP_INIT;

	TEST4_SUCCESS("123.456", double, 0, 1000, 123.456);
	TEST4_SUCCESS("1234", size_t, -123, 4000, 1234);
	TEST4_FAILURE("12345", int, -10, 100, ERANGE);

#define TEST2(x, y) do {				\
	fprintf(stderr, "PARSENUM(\"%s\")\n", y);	\
	if (PARSENUM(x, y))				\
		warnp("PARSENUM");			\
	fprintf(stderr, "%f %zu %d\n", d, s, i);	\
} while (0)

	TEST2(&d, "234.567");
	TEST2(&s, "2345");

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

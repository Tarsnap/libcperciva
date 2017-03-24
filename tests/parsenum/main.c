#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "parsenum.h"
#include "warnp.h"

static int
parsenum_equal(double x, double y)
{

	/* Deal with NANs. */
	if (isnan(x) && isnan(y))
		return (1);

	/* If signs do not match, the numbers are not equal. */
	if (signbit(x) != signbit(y))
		return (0);

	/* Deal with infinities. */
	if (isinf(x) && isinf(y))
		return (1);

	/* Deal with zeros. */
	if ((fpclassify(x) == FP_ZERO) && (fpclassify(y) == FP_ZERO))
		return (1);

	/* Deal with real numbers. */
	if ((fabs(x - y) < 1) && (fabs(x - y) <= fabs(x + y) * 1e-6))
		return (1);

	/* Not equal. */
	return (0);
}

#define TEST4_DO(str, var, min, max, target)			\
	var parsenum_x;						\
	int parsenum_ret;					\
	fprintf(stderr,						\
	    "Parsing \"%s\" as %s between %s and %s (incl.) yields "	\
	    target "... ", str, #var, #min, #max);		\
	parsenum_ret = PARSENUM(&parsenum_x, str, min, max);

#define TEST2_DO(str, var, target)				\
	var parsenum_x;						\
	int parsenum_ret;					\
	fprintf(stderr,						\
	    "Parsing \"%s\" as %s yields " target "... ", str, #var);	\
	parsenum_ret = PARSENUM(&parsenum_x, str);

#define CHECK_SUCCESS(target)					\
	if (parsenum_ret == 0) {				\
		if (parsenum_equal((double)parsenum_x, (double)target)) { \
			fprintf(stderr, "PASSED!\n");		\
		} else {					\
			fprintf(stderr, "FAILED!\n");		\
			goto err0;				\
		}						\
	} else {						\
		fprintf(stderr, "FAILED!\n");			\
		goto err0;					\
	}

#define CHECK_FAILURE(target)					\
	if (parsenum_ret != 0) {				\
		if (errno == target)				\
			fprintf(stderr, "PASSED!\n");		\
		else						\
			fprintf(stderr, "FAILED!\n");		\
	} else {						\
		fprintf(stderr, "FAILED!\n");			\
		goto err0;					\
	}

#define TEST4_SUCCESS(str, var, min, max, target) do {		\
	TEST4_DO(str, var, min, max, #target);			\
	CHECK_SUCCESS(target);					\
} while (0)

#define TEST4_FAILURE(str, var, min, max, target) do {		\
	TEST4_DO(str, var, min, max, #target);			\
	CHECK_FAILURE(target);					\
} while (0)

#define TEST2_SUCCESS(str, var, target) do {			\
	TEST2_DO(str, var, #target);				\
	CHECK_SUCCESS(target);					\
} while (0)

#define TEST2_FAILURE(str, var, target) do {			\
	TEST2_DO(str, var, #target);				\
	CHECK_FAILURE(target);					\
} while (0)

int
main(int argc, char * argv[])
{
	(void)argc; /* UNUSED */
	(void)argv; /* UNUSED */

	WARNP_INIT;

	TEST4_SUCCESS("123.456", double, 0, 1000, 123.456);
	TEST4_SUCCESS("1234", size_t, -123, 4000, 1234);
	TEST4_FAILURE("12345", int, -10, 100, ERANGE);

	TEST2_SUCCESS("234.567", double, 234.567);
	TEST2_SUCCESS("2345", size_t, 2345);
	TEST2_FAILURE("abcd", size_t, EINVAL);

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

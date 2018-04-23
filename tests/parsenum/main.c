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
	printf("Parsing \"%s\" as %s between %s and %s (incl.) yields "	\
	    target "... ", str, #var, #min, #max);		\
	parsenum_ret = PARSENUM(&parsenum_x, str, min, max);

#define TEST2_DO(str, var, target)				\
	var parsenum_x;						\
	int parsenum_ret;					\
	printf("Parsing \"%s\" as %s yields " target "... ", str, #var);	\
	parsenum_ret = PARSENUM(&parsenum_x, str);

#define CHECK_SUCCESS(target)					\
	if (parsenum_ret == 0) {				\
		if (parsenum_equal((double)parsenum_x, (double)target)) { \
			printf("PASSED!\n");		\
		} else {					\
			printf("FAILED!\n");		\
			goto err0;				\
		}						\
	} else {						\
		printf("FAILED!\n");			\
		goto err0;					\
	}

#define CHECK_FAILURE(target)					\
	if (parsenum_ret != 0) {				\
		if (errno == target) {				\
			printf("PASSED!\n");		\
		} else {					\
			printf("FAILED!\n");		\
			goto err0;				\
		}						\
	} else {						\
		printf("FAILED!\n");			\
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

	/* Disable warning that 0x100000000 cannot fit into uint32_t. */
#if __clang__
	_Pragma("clang diagnostic ignored \"-Wtautological-constant-out-of-range-compare\"")
#elif __GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
	_Pragma("GCC diagnostic ignored \"-Wtype-limits\"")
#endif

	TEST4_SUCCESS("123.456", double, 0, 1000, 123.456);
	TEST4_SUCCESS("nAn", double, 0, 0, NAN);
	TEST4_SUCCESS("inf", double, 0, INFINITY, INFINITY);
	TEST4_SUCCESS("-InFiNitY", double, -INFINITY, 0, -INFINITY);
	TEST4_SUCCESS("0", double, 0, 0, 0);
	TEST4_SUCCESS("-0", double, 0, 0, -0.0);
	TEST4_FAILURE("7f", double, 0, 1000, EINVAL);
	TEST4_SUCCESS("0X7f", double, 0, 1000, 127);
	TEST4_SUCCESS("-0x7f", double, -1000, 0, -127);
	TEST4_SUCCESS("-1e2", double, -1000, 0, -100);
	TEST4_SUCCESS("1e-2", double, 0, 1000, 0.01);

	TEST4_SUCCESS("123.456", float, 0, 1000, 123.456);
	TEST4_SUCCESS("nAn", float, 0, 0, NAN);
	TEST4_SUCCESS("inf", float, 0, INFINITY, INFINITY);
	TEST4_SUCCESS("-InFiNitY", float, -INFINITY, 0, -INFINITY);
	TEST4_SUCCESS("0", float, 0, 0, 0);
	TEST4_SUCCESS("-0", float, 0, 0, -0.0);
	TEST4_FAILURE("7f", float, 0, 1000, EINVAL);
	TEST4_SUCCESS("0X7f", float, 0, 1000, 127);
	TEST4_SUCCESS("-0x7f", float, -1000, 0, -127);
	TEST4_SUCCESS("-1e2", float, -1000, 0, -100);
	TEST4_SUCCESS("1e-2", float, 0, 1000, 0.01);

	TEST4_SUCCESS("1234", size_t, -123, 4000, 1234);
	TEST4_FAILURE("7f", size_t, 0, 1000, EINVAL);
	TEST4_SUCCESS("0x7f", size_t, 0, 1000, 127);
	TEST4_SUCCESS("0x77", size_t, 0, 1000, 119);
	TEST4_SUCCESS("077", size_t, 0, 1000, 63);

	TEST4_SUCCESS("0xFFFFffff", uint32_t, 0, 0xFFFFffff, UINT32_MAX);
	TEST4_FAILURE("0x100000000", uint32_t, 0, 0xFFFFffff, ERANGE);
	TEST4_FAILURE("0x100000000", uint32_t, 0, 0x100000000, ERANGE);

	TEST4_FAILURE("12345", int, -10, 100, ERANGE);
	TEST4_SUCCESS("0x7fffFFFF", int, 0, INT32_MAX, INT32_MAX);
	TEST4_SUCCESS("-0X7fffFFFF", int, INT32_MIN, 0, INT32_MIN + 1);
	TEST4_SUCCESS("077", int, 0, 100, 63);
	TEST4_SUCCESS("-077", int, -100, 0, -63);

	TEST4_SUCCESS("0x7fffFFFF", int32_t, 0, INT32_MAX, INT32_MAX);
	TEST4_SUCCESS("-0x80000000", int32_t, INT32_MIN, 0, INT32_MIN);

	TEST2_SUCCESS("234.567", double, 234.567);

	TEST2_SUCCESS("2345", size_t, 2345);
	TEST2_FAILURE("abcd", size_t, EINVAL);

	TEST2_SUCCESS("0XffffFFFF", uint32_t, UINT32_MAX);
	TEST2_FAILURE("ffffFFFF", uint32_t, EINVAL);
	TEST2_FAILURE("0XfffffFFFF", uint32_t, ERANGE);
	TEST2_FAILURE("-1", uint32_t, ERANGE);

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

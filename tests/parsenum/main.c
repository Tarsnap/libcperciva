#include <sys/resource.h>

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "parsenum.h"
#include "warnp.h"

static int
parsenum_equal(double x, double y)
{

#if defined(__clang__)
	_Pragma("clang diagnostic push")
	_Pragma("clang diagnostic ignored \"-Wconversion\"")
	_Pragma("clang diagnostic ignored \"-Wdouble-promotion\"")
#endif

	/* Check special status. */
	if (fpclassify(x) != fpclassify(y))
		return (0);

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

#if defined(__clang__)
	_Pragma("clang diagnostic pop")
#endif

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

#define TEST_EX6_DO(str, var, min, max, target, base, trailing)	\
	var parsenum_x;						\
	int parsenum_ret;					\
	printf("Parsing \"%s\" in base %i as %s with trailing"	\
	    " %i between %s and %s (incl.) yields " target	\
	    "... ", str, base, #var, trailing, #min, #max);	\
	parsenum_ret = PARSENUM_EX(&parsenum_x, str, min, max, base, trailing);

#define TEST_EX4_DO(str, var, target, base, trailing)		\
	var parsenum_x;						\
	int parsenum_ret;					\
	printf("Parsing \"%s\" in base %i as %s with trailing"	\
	    " %i yields " target "... ", str, base, #var,	\
	    trailing);						\
	parsenum_ret = PARSENUM_EX(&parsenum_x, str, base, trailing);

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

/* Handle extended PARSENUM */
#define TEST_EX6_SUCCESS(str, var, min, max, target, base, trailing) do { \
	TEST_EX6_DO(str, var, min, max, #target, base, trailing); \
	CHECK_SUCCESS(target);					\
} while (0)

#define TEST_EX6_FAILURE(str, var, min, max, target, base, trailing) do { \
	TEST_EX6_DO(str, var, min, max, #target, base, trailing); \
	CHECK_FAILURE(target);					\
} while (0)

#define TEST_EX4_SUCCESS(str, var, target, base, trailing) do {	\
	TEST_EX4_DO(str, var, #target, base, trailing);		\
	CHECK_SUCCESS(target);					\
} while (0)

#define TEST_EX4_FAILURE(str, var, target, base, trailing) do {	\
	TEST_EX4_DO(str, var, #target, base, trailing);		\
	CHECK_FAILURE(target);					\
} while (0)

static void
test_assert_failure(const char * argv_1)
{
	size_t assert_num;
	int i;
	double f;
	struct rlimit rlp;

	/* Which test should we attempt? */
	if (PARSENUM(&assert_num, argv_1, 1, 4)) {
		warnp("Parameter should be an error case between 1 and 4: %s",
		    argv_1);
		exit(1);
	}

	/* We know that we'll abort(), so disable core files. */
	rlp.rlim_max = 0;
	rlp.rlim_cur = 0;
	if (setrlimit(RLIMIT_CORE, &rlp)) {
		warnp("setrlimit");
		exit(1);
	}

	/*
	 * Each of these commands should end with an abort().  This will
	 * produce a memory leak due to warnp_setprogname, but that's ok
	 * because if we reach an abort() in a normal program then we have
	 * bigger problems.
	 */
	switch(assert_num) {
	case 1:
		/* Signed integer without specified bounds. */
		if (PARSENUM(&i, "1"))
			exit(1);
		break;
	case 2:
		/* Signed integer without specified bounds (with base). */
		if (PARSENUM_EX(&i, "1", 16, 0))
			exit(1);
		break;
	case 3:
		/* Non-zero base applied to float. */
		if (PARSENUM_EX(&f, "1.23", 16, 0))
			exit(1);
		break;
	case 4:
		/* Non-zero base applied to float (with bounds). */
		if (PARSENUM_EX(&f, "1.23", 0, 2, 16, 0))
			exit(1);
		break;
	default:
		fprintf(stderr, "No such error case.\n");
	}

	/*
	 * We should not reach here, but the test suite is expecting a
	 * non-zero value, so exiting with 0 should produce a problem.
	 */
	exit(0);
}

int
main(int argc, char * argv[])
{

	WARNP_INIT;

	/* Test attempting to use PARSENUM improperly. */
	if (argc > 1)
		test_assert_failure(argv[1]);

	/* Disable warning that 0x100000000 cannot fit into uint32_t. */
#if defined(__clang__)
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
	TEST4_FAILURE("-50", size_t, -100, 0, ERANGE);
	TEST4_FAILURE("-50", size_t, 0, -100, ERANGE);
	TEST4_FAILURE("0", size_t, -200, -100, ERANGE);

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

	TEST4_SUCCESS("123", uintmax_t, 0, UINTMAX_MAX, 123);
	TEST4_SUCCESS("123", uintmax_t, -200, 200, 123);
	TEST4_FAILURE("-123", uintmax_t, -200, -100, ERANGE);

	TEST4_SUCCESS("123", intmax_t, 0, INTMAX_MAX, 123);
	TEST4_SUCCESS("-123", intmax_t, -200, -100, -123);
	TEST4_SUCCESS("123", intmax_t, INTMAX_MIN, INTMAX_MAX, 123);

	/* Handle alternate bases */
	TEST_EX4_SUCCESS("11", size_t, 17, 16, 0);
	TEST_EX4_SUCCESS("11", size_t, 11, 0, 0);
	TEST_EX4_FAILURE("122223333", uint32_t, ERANGE, 16, 0);
	TEST_EX4_SUCCESS("122223333", uint32_t, 122223333, 0, 0);
	TEST_EX4_FAILURE("ga", size_t, EINVAL, 16, 0);
	TEST_EX4_SUCCESS("ga", size_t, 282, 17, 0);

	TEST_EX6_SUCCESS("11", size_t, 0, 30, 17, 16, 0);
	TEST_EX6_FAILURE("11", size_t, 0, 10, ERANGE, 16, 0);
	TEST_EX6_FAILURE("ga", size_t, 0, 10, EINVAL, 16, 0);

	TEST_EX4_SUCCESS("11", float, 11, 0, 0);
	TEST_EX6_SUCCESS("11", float, 0, 12, 11, 0, 0);

	TEST_EX4_SUCCESS("12\", 34", size_t, 12, 0, 1);
	TEST_EX4_SUCCESS("12\t, 34", size_t, 12, 0, 1);
	TEST_EX4_FAILURE("12\", 34", size_t, EINVAL, 0, 0);
	TEST_EX4_FAILURE("12\t, 34", size_t, EINVAL, 0, 0);
	TEST_EX6_SUCCESS("-34\", 34", int, -50, 0, -34, 0, 1);
	TEST_EX6_SUCCESS("-34\t, 34", int, -50, 0, -34, 0, 1);
	TEST_EX6_FAILURE("-34\", 34", int, -50, 0, EINVAL, 0, 0);
	TEST_EX6_FAILURE("-34\t, 34", int, -50, 0, EINVAL, 0, 0);
	TEST_EX4_SUCCESS("11  ", float, 11, 0, 1);
	TEST_EX6_SUCCESS("11\t ", float, 0, 12, 11, 0, 1);
	TEST_EX4_FAILURE("11  ", float, EINVAL, 0, 0);
	TEST_EX6_FAILURE("11\t ", float, 0, 12, EINVAL, 0, 0);

	/* Success! */
	exit(0);

err0:
	/* Failure! */
	exit(1);
}

#ifndef _PARSENUM_H_
#define _PARSENUM_H_

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdlib.h>

/* Handle compiler warnings about implicit variable conversion in PARSENUM. */
#ifdef __clang__

/* Disable clang warnings. */
#define PARSENUM_PROLOGUE						\
_Pragma("clang diagnostic push")					\
_Pragma("clang diagnostic ignored \"-Wunknown-pragmas\"")		\
_Pragma("clang diagnostic ignored \"-Wfloat-conversion\"")		\
_Pragma("clang diagnostic ignored \"-Wsign-conversion\"")		\
_Pragma("clang diagnostic ignored \"-Wshorten-64-to-32\"")		\
_Pragma("clang diagnostic ignored \"-Wconversion\"")

/* Enable clang warnings for code outside of PARSENUM. */
#define PARSENUM_EPILOGUE						\
_Pragma("clang diagnostic pop")

/* Other compilers don't need any special handling */
#else
#define PARSENUM_PROLOGUE /* NOTHING */
#define PARSENUM_EPILOGUE /* NOTHING */
#endif /* !__clang__ */

/**
 * PARSENUM(x, s, min, max):
 * Parse the string ${s} according to the type of the unsigned integer, signed
 * integer, or floating-point number variable ${x}.  If the string consists of
 * optional whitespace followed by a number (and nothing else) and the numeric
 * interpretation of the number is between ${min} and ${max} inclusive, store
 * the value into ${x}, set errno to zero, and return zero.  Otherwise, return
 * nonzero with an unspecified value of ${x} and errno set to EINVAL or ERANGE
 * as appropriate.
 * 
 * For floating-point and unsigned integer variables ${x}, this can also be
 * invoked as PARSENUM(x, s), in which case the minimum and maximum values are
 * set to +/- infinity or the limits of the unsigned integer type.
 */
#define PARSENUM2(x, s)							\
	(								\
		PARSENUM_PROLOGUE					\
		errno = 0,						\
		(((*(x)) = 1, (*(x)) /= 2) > 0)	?			\
			((*(x)) = parsenum_float((s),			\
			    (double)-INFINITY, (double)INFINITY)) :	\
		(((*(x)) = -1) > 0) ?					\
			((*(x)) = parsenum_unsigned((s), 0, (*(x)))) :	\
			(assert(0 && "PARSENUM applied to signed integer without specified bounds"), 1),	\
		errno != 0						\
		PARSENUM_EPILOGUE					\
	)
#define PARSENUM4(x, s, min, max)					\
	(								\
		PARSENUM_PROLOGUE					\
		errno = 0,						\
		(((*(x)) = 1, (*(x)) /= 2) > 0)	?			\
			((*(x)) = parsenum_float((s), (min), (max))) :	\
		(((*(x)) = -1) <= 0) ?					\
			((*(x)) = parsenum_signed((s), (min), (max))) :	\
			(assert(((min) >= 0) && "PARSENUM unsigned with negative minimum bound"),			\
			assert(((max) <= *(x)) && "PARSENUM unsigned upper bound does not fit in datatype"),			\
			((*(x)) = parsenum_unsigned((s),		\
			    (min) <= 0 ? 0 : (min),			\
			    (max) >= *(x) ? *(x) : (max))),		\
			((((max) < 0) && (errno == 0)) ?		\
			    (errno = ERANGE) : 0)),			\
		errno != 0						\
		PARSENUM_EPILOGUE					\
	)

/* Magic to select which version of PARSENUM to use. */
#define PARSENUM(...)	PARSENUM_(PARSENUM_COUNT(__VA_ARGS__))(__VA_ARGS__)
#define PARSENUM_(N)	PARSENUM__(N)
#define PARSENUM__(N)	PARSENUM ## N
#define PARSENUM_COUNT(...)	PARSENUM_COUNT_(__VA_ARGS__, 4, 3, 2, 1)
#define PARSENUM_COUNT_(_1, _2, _3, _4, N, ...)	N

/* Functions for performing the parsing and parameter checking. */
static inline double
parsenum_float(const char * s, double min, double max)
{
	char * eptr;
	double val;

	val = strtod(s, &eptr);
	if ((eptr == s) || (*eptr != '\0'))
		errno = EINVAL;
	else if ((val < min) || (val > max))
		errno = ERANGE;
	return (val);
}

static inline intmax_t
parsenum_signed(const char * s, intmax_t min, intmax_t max)
{
	char * eptr;
	intmax_t val;

	val = strtoimax(s, &eptr, 0);
	if ((eptr == s) || (*eptr != '\0'))
		errno = EINVAL;
	else if ((val < min) || (val > max)) {
		errno = ERANGE;
		val = 0;
	}
	return (val);
}

static inline uintmax_t
parsenum_unsigned(const char * s, uintmax_t min, uintmax_t max)
{
	char * eptr;
	uintmax_t val;

	val = strtoumax(s, &eptr, 0);
	if ((eptr == s) || (*eptr != '\0'))
		errno = EINVAL;
	else if ((val < min) || (val > max))
		errno = ERANGE;
	return (val);
}

#endif /* !_PARSENUM_H_ */

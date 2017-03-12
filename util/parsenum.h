#ifndef _PARSENUM_H_
#define _PARSENUM_H_

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>

/**
 * PARSENUM(x, s, min, max):
 * Parse the string ${s} according to the type of the unsigned integer, signed
 * integer, or floating-point number variable ${x}.  If the string consists of
 * optional whitespace followed by a number (and nothing else) and the numeric
 * interpretation of the number is between ${min} and ${max} inclusive, store
 * the value into ${x}, set errno to zero, and return zero.  Otherwise, return
 * nonzero with an unspecified value of ${x} and errno set to EINVAL or ERANGE
 * as appropriate.
 */
#define PARSENUM(x, s, min, max)					\
	(								\
		errno = 0,						\
		(((*(x)) = 1, (*(x)) /= 2) > 0)	?			\
			((*(x)) = parsenum_float((s), (min), (max))) :	\
		(((*(x)) = -1) <= 0) ?					\
			((*(x)) = parsenum_signed((s), (min), (max))) :	\
			(((*(x)) = parsenum_unsigned((s),		\
			    (min) <= 0 ? 0 : (min),			\
			    (max) >= *(x) ? *(x) : (max))),		\
			((((max) < 0) && (errno == 0)) ?		\
			    (errno = ERANGE) : 0)),			\
		errno != 0						\
	)

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

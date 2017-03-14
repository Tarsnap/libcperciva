#include <stdint.h>
#include <stdio.h>

#include "parsenum.h"
#include "warnp.h"

int
main(int argc, char * argv[])
{
	double d = 0.0;
	size_t s = 0;
	int i = 0;

	(void)argc; /* UNUSED */
	(void)argv; /* UNUSED */

	WARNP_INIT;

#define TEST(x, y, z, w) do {						\
	fprintf(stderr, "PARSENUM(\"%s\", %s, %s)\n", y, #z, #w);	\
	if (PARSENUM(x, y, z, w))					\
		warnp("PARSENUM");					\
	fprintf(stderr, "%f %zu %d\n", d, s, i);			\
} while (0)

	TEST(&d, "123.456", 0, 1000);
	TEST(&s, "1234", -123, 4000);
	TEST(&i, "12345", -10, 100);

#define TEST2(x, y) do {				\
	fprintf(stderr, "PARSENUM(\"%s\")\n", y);	\
	if (PARSENUM(x, y))				\
		warnp("PARSENUM");			\
	fprintf(stderr, "%f %zu %d\n", d, s, i);	\
} while (0)

	TEST2(&d, "234.567");
	TEST2(&s, "2345");
}

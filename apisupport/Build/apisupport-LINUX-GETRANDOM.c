#include <sys/random.h>

#include <stdlib.h>

#define BUFLEN 2

int
main(void)
{
	char buf[BUFLEN];

	if (getrandom(buf, BUFLEN, 0) != BUFLEN)
		exit(1);

	exit(0);
}

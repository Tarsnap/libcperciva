#include <sys/auxv.h>

int
main(void)
{
	unsigned long val;

	val = getauxval(AT_HWCAP);

	/* Done! */
	return (val != 0);
}

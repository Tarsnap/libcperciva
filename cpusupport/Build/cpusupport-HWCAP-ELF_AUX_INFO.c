#include <sys/auxv.h>

int
main(void)
{
	int res;
	unsigned long val;

	res = elf_aux_info(AT_HWCAP, &val, sizeof(unsigned long));
	(void)res;

	/* Done! */
	return (val != 0);
}

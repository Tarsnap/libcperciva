static int
foo(const char * restrict x, const char * restrict y)
{

	/* Done! */
	return (x == y);
}

int
main(void)
{
	char x[10];
	char y[10];

	/* Done! */
	return (foo(x, y));
}

static int
foo(const char * restrict x, const char * restrict y)
{

	return (x == y);
}

int
main(void)
{
	const char x[10];
	const char y[10];

	return (foo(x, y));
}

int Main(int *p)
{
	int *a;
	int x;
	a = &x;
	x = 5;
	if(*a > 0) {
		p = a;
	}

	return 0;
}

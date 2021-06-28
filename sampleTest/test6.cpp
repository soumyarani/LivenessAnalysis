int Main(double *p)
{
	double *a;
	double x;
	a = &x;
	x = 5.0;
	if(*a > 0) {
		p = a;
	}

	return 0;
}

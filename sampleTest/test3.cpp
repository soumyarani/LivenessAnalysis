int *a,*y,**x;
int main()
{
	*y=1;
	x=&a;
	if(*y < 0) {
		*x = y;
	}
	y = *x;
	return 0;

}

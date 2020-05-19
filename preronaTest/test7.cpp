int Main(int check)
{
	int x,y;
	
	x = 5.0;
	if(check > 0) {
		x = x*check;
	} else
		x = x+check;

	y = x;

	return 0;
}

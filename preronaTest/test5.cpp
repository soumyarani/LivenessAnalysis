int x,y;
int main() {
	x=10;
	y=x;
	if(y<20) {
		x=12;
		y=x;
	}
	x=30;
	y=x;
	return 0;
} //wierd case

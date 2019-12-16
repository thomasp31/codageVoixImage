#include <stdio.h>
#include <stdlib.h>

static unsigned char R=200, G=255, B=0;

// tracer 1 pixel
static void plot1px( int x, int y, unsigned char * dest, int stride )
{
unsigned int a = x * 3 + y * stride;
dest[a++] = B;
dest[a++] = G;
dest[a]   = R;
}

// algorithme de Bresenham
void draw_vector( int x0, int y0, int x1, int y1, unsigned char * dest, int stride, int w, int h )
{
int tmp, dx, dy, D, x, y, inc;
// printf("draw %d, %d to %d, %d\n", x0, y0, x1, y1 );
dx = x1 - x0;
dy = y1 - y0;
inc = 1;
if	( abs(dx) > abs(dy) )
	{			// "X fast" octants
	if	( x1 < x0 )		// assurer x croissant
		{
		tmp = x0; x0 = x1; x1 = tmp;
		tmp = y0; y0 = y1; y1 = tmp;
		dx = x1 - x0;
		dy = y1 - y0;
		}
	if	( dy < 0 )
		{ dy = -dy; inc = -1; }
	D = 2*dy - dx;
	y = y0;
	for	( x = x0; x <= x1; ++x )
		{
		// printf( "%4d : %4d %4d\n", D, x, y );
		if	( ( x >= 0 ) && ( x < w ) && ( y >= 0 ) && ( y < h ) )
			plot1px( x, y, dest, stride );
		if	( D > 0 )
			{
			y += inc;
			D = D - 2*dx;
			}
		D = D + 2*dy;
		}
	}
else	{			// "Y fast" octants
	if	( y1 < y0 )		// assurer y croissant
		{
		tmp = x0; x0 = x1; x1 = tmp;
		tmp = y0; y0 = y1; y1 = tmp;
		dx = x1 - x0;
		dy = y1 - y0;
		}
	if	( dx < 0 )
		{ dx = -dx; inc = -1; }
	D = 2*dx - dy;
	x = x0;
	for	( y = y0; y <= y1; ++y )
		{
		// printf( "%4d : %4d %4d\n", D, x, y );
		if	( ( x >= 0 ) && ( x < w ) && ( y >= 0 ) && ( y < h ) )
			plot1px( x, y, dest, stride );
		if	( D > 0 )
			{
			x += inc;
			D = D - 2*dy;
			}
		D = D + 2*dx;
		}
	}
}

/*int main()
{
draw_vector( 5, 0, 0, 0, NULL, 0, 10, 10 );
draw_vector( 5, 0, 0, 2, NULL, 0, 10, 10 );
draw_vector( 5, 0, 0, 5, NULL, 0, 10, 10 );
draw_vector( 2, 0, 0, 5, NULL, 0, 10, 10 );
draw_vector( 0, 0, 0, 5, NULL, 0, 10, 10 );
return 0;
}*/
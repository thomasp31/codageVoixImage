#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avi_nc.h"
#include "v42.h"
#include "decode.h"
#include "draw.h"

int over_cnt = 0;


// copie d'un bloc BRG de 8x8 de la trame src a la trame dest
// stride = enjambement (difference d'adresse d'une ligne a l'autre)
void copy_bloc( unsigned char * dest, unsigned char * src, int x, int y, int stride )
{
int iy, a;
a = x * 3 + y * stride;
for	( iy = 0; iy < 8; ++iy )
	{
	memcpy( dest + a, src + a, 8*3 );
	a += stride;
	}
}

// reconstruction d'un bloc a partir d'un predicteur (selon ref, px, py)
// et un residu (selon diff, x, y, iquant)
void reconst_bloc( unsigned char * dest, unsigned char * diff, unsigned char * ref,
		   int x, int y, int px, int py, int stride, int iquant )
{
int iy, ax, ad, ar, v;
ad =  x * 3 +  y * stride;	// offset pour dest et diff
ar = px * 3 + py * stride;	// offset pour predicteur
for	( iy = 0; iy < 8; ++iy )
	{
	for	( ax = 0; ax < (8*3); ++ax )
		{
		v = (int)ref[ar+ax] + ( dequant[iquant][diff[ad+ax]] - 256 );
		if	( v < 0 ) v = 0;
		if	( v > 255 ) v = 255;
		dest[ad+ax] = (unsigned char)v;
		}
	ad += stride;
	ar += stride;
	}
}


// decodage des composantes du vecteur de mouvement et infos annexes
// MSB iXXXXXXXqqYYYYYY LSB
int extract_vect( short vectcode, int * pvx, int * pvy, int * pquant )
{
if	( vectcode & 0x8000 )
	return 1;
int v;
v = ( vectcode >> 8 ) & 0x7F;
if	( v & 0x40 ) v |= 0xFFFFFF80;	// extension de signe 7 bits -> 32 bits
* pvx = v; 
v = vectcode & 0x3F;
if	( v & 0x20 ) v |= 0xFFFFFFC0;	// extension de signe 6 bits -> 32 bits
* pvy = v;
* pquant = ( vectcode >> 6 ) & 0x03;
return 0;
}

int decode_trame( unsigned char * dest, unsigned char * diff, unsigned char * ref,
		  dims * dim, short * vectcode )
{
int x, y;		// origine bloc courant
int ivec = 0;		// indice bloc courant (pour chercher vecteur)
int iquant;		// indice du quantificateur
int px, py;		// origine bloc predicteur
int vx, vy;		// vecteur de compensation de mouvement

for	( y = 0; y < dim->he; y += 8 )
	for	( x = 0; x < dim->wi; x += 8 )
		{
		if	( extract_vect( vectcode[ivec++], &vx, &vy, &iquant ) )
			{
			copy_bloc( dest, diff, x, y, dim->ll );
			}
		else	{
			px = x + vx;
			if	( px < 0 )
				{ if (++over_cnt < 50 ) printf("warn: px < 0\n"); px = 0; }
			if	( px > ( dim->wi - 8 ) )
				{ if (++over_cnt < 50 ) printf("warn: px > max\n"); px = ( dim->wi - 8 ); }
			py = y + vy;
			if	( py < 0 )
				{ if (++over_cnt < 50 ) printf("warn: py < 0\n"); py = 0; }
			if	( py > ( dim->he - 8 ) )
				{ if (++over_cnt < 50 ) printf("warn: py > max\n"); py = ( dim->he - 8 ); }
			reconst_bloc( dest, diff, ref, x, y, px, py, dim->ll, iquant );
			}
		}
return 0;
}

// dans un but d'illustration, trace les vecteurs sur l'image decodee
void vectorise_trame( unsigned char * dest, dims * dim, short * vectcode )
{
int x, y;		// origine bloc courant
int ivec = 0;		// indice bloc courant (pour chercher vecteur)
int vx, vy;		// vecteur de compensation de mouvement
int iquant, x0, y0;
ivec = 0;
for	( y = 0; y < dim->he; y += 8 )
	for	( x = 0; x < dim->wi; x += 8 )
		{
		if	( !extract_vect( vectcode[ivec++], &vx, &vy, &iquant ) )
			{
			x0 = x + 4; y0 = y + 4;	// origine au milieu du bloc
			draw_vector( x0, y0, x0 + vx, y0 + vy, dest, dim->ll, dim->wi, dim->he );
			}
		}
}

// copie d'une trame src a dest
void copy_trame( unsigned char * dest, unsigned char * src, dims * dim )
{
memcpy( dest, src, dim->tot );
}

int decode_avi( char * video_in, char * video_out, int vectorscope )
{
avipars s, d;
unsigned char *buf1, *buf2, *ref;	// buffers de trame
unsigned int ifram;			// index de trame
short * vectcodes;			// table des vecteurs
dims dim;				// dimensions utiles
int qlinaux;				// nombre de lignes auxiliaires
double dqlinaux;			// nombre de lignes auxiliaires (approx)

// ouverture AVI source
s.fnam = video_in;
s.data = NULL;		// memoire pas encore allouee

readAVIheaders( &s );
s.tot = s.ll * s.he;

// calcul du nombre de lignes auxiliaires
// t.q. qlinaux >= ( ( s.wi * ( s.he - qlinaux ) ) / 64 ) * ( 2 / s.ll )
dqlinaux = (double)(s.he * s.wi);
dqlinaux /= ( 32 * s.ll + s.wi );
qlinaux = 1 + (int)(dqlinaux - 1e-14);
printf("retrait de %d lignes auxiliaires\n", qlinaux );

// allocation du buffers pour lecture
s.data = malloc( s.tot );
if	( s.data == NULL )
	gasp("malloc %d", s.tot );

// localisation des vecteurs
vectcodes = (short *)( s.data + s.ll * ( s.he - qlinaux ) );

// ouverture AVI dest
d = s;
d.fnam = video_out;
d.he -= qlinaux;	// enlever les lignes auxiliaires
d.tot = d.ll * d.he;	// encombrement total des pixels d'une trame

if	( ( d.wi % 8 ) || ( d.he % 8 ) )
	gasp("dimensions non multiples de 8 : %dx%d", d.wi, d.he );

writeAVIheaders( &d );

// dimensions 'utiles'
dim.wi = d.wi;
dim.he = d.he;
dim.ll = d.ll;
dim.tot = dim.he * dim.ll;

// allocation des buffers pour sortie et ref
buf1 = malloc( dim.tot );
buf2 = malloc( dim.tot );
if ( ( buf1 == NULL ) || ( buf2 == NULL ) )
   gasp("malloc %d", dim.tot );

if	( vectorscope )			// vectorscope : dessine les vecteurs sur l'image
	{
	ref = buf1;			// buffers fixes car on doit faire une copie de la trame
	d.data = buf2;
	for	( ifram = 0; ifram < s.nframes; ifram++ )
		{
		// traitement d'un trame
		printf("."); fflush(stdout);
		readRGBframe( &s );	// trame lue dans s.data
		decode_trame( d.data, s.data, ref, &dim, vectcodes );
		copy_trame( ref, d.data, &dim );	// copie pour ne pas mettre les vecteurs sur la ref
		vectorise_trame( d.data, &dim, vectcodes );
		writeRGBframe( &d );	// d.data va etre utilise
		}
	}
else	{				// le classique
	for	( ifram = 0; ifram < s.nframes; ifram++ )
		{
		// permutation buffers "ping-pong" pour eviter copies
		if	( ifram & 1 )
			{ ref = buf1; d.data = buf2; }
		else	{ ref = buf2; d.data = buf1; }
		// traitement d'un trame
		// printf("frame %d\n", ifram );
		printf("."); fflush(stdout);
		readRGBframe( &s );	// trame lue dans s.data
		decode_trame( d.data, s.data, ref, &dim, vectcodes );
		writeRGBframe( &d );	// d.data va etre utilise
		}
	}
writeAVIindex( &d );
return 0;
}

/*
int main( int argc, char **argv )
{
printf("programme de codage-decodage R42 sur AVI RGB\n");

if ( argc != 3 )
   { printf("usage : r42 source.avi dest.avi\n");
   exit(1);
   };

decode_avi( argv[1], argv[2] );
return 0;
}
*/
#include <stdio.h>
#include <math.h>
#include "v42.h"

// quantifieurs
// index 0 : perte minimale (quantificateur de reference)
// index 1, 2, 3 : perte augmentees (a completer)

// dans les tables, les valeurs sont translatees pour que les index soient toujours positifs

// index : [0, 511] : dans ce tableau l'index 256 correspond a une difference nulle
// contenu : [0, 255] : le code 128 exprime une difference nulle
unsigned char quant[4][512];

// index  : [0, 255] : dans ce tableau l'index 128 est le code qui exprime une difference nulle
// contenu : [0, 511] : la valeur 256 exprime une difference nulle
int dequant[4][256];

// symetrisation de la table de quantification : cree la zone "negative" a partir de la zone "positive"
// la zone positive va de l'index 257 a l'index 511
static void symetrize( int iquant )
{
int diff;
for	( diff = 1; diff < 256; ++diff )
	quant[iquant][-diff+256] = 256 - quant[iquant][diff+256];
quant[iquant][0] = quant[iquant][1];
quant[iquant][256] = 128;	// obligatoire pour tout quantificateur
}

// obtention d'une table de dequantification par inversion d'une table de quantification
static void reverse( int iquant )
{
unsigned int idiff, first, last, code; double v;
idiff = 0;
while	( idiff < 512 )
	{
	first = idiff; code = quant[iquant][idiff];
	if	( code > 255 )
		{ printf("erreur dans la table de quantification\n"); code = 255; }
	do	{
		last = idiff;
		++idiff;
		} while	( ( idiff < 256 ) && ( code == quant[iquant][idiff] ) );
	v = (double)(first+last) * 0.5;
	dequant[iquant][code] = (int)round(v);
	}
dequant[iquant][128] = 256;	// obligatoire pour tout quantificateur
}

// initialisation du quantificateur par defaut (perte minimale)
// on doit initialiser la zone positive, qui correspond aux differences allant de +1 a +255
// les codes produits doivent etre dans l'intervalle [128:255]
// la fonction doit etre monotone croissante pour que l'inversion fonctionne
static void generate_default_quant( int iquant )
{
int diff, code;
if(iquant<=1){
	for	( diff = 1; diff < 256; ++diff )
	{
	code = diff / 2 + 1;	// +1 -> +1, +255 -> +128
	code += 128;
	if	( code > 255 ) code = 255;
	if	( code < 0   ) code = 0;
	quant[iquant][diff+256] = (unsigned char)code;
	}
}else if (iquant==2){
	for	( diff = 1; diff < 256; ++diff )
	{
	code = log(diff);	// +1 -> +1, +255 -> +128
	//code += 128;
	if	( code > 255 ) code = 255;
	if	( code < 0   ) code = 0;
	quant[iquant][diff+256] = (unsigned char)code;
	}
}else if (iquant==3){
	for	( diff = 1; diff < 256; ++diff )
	{
	code = sqrt(diff);	// +1 -> +1, +255 -> +128
	//code += 128;
	if	( code > 255 ) code = 255;
	if	( code < 0   ) code = 0;
	quant[iquant][diff+256] = (unsigned char)code;
	}
}

}

// initialisation des 4 quantificateurs
void init_quant()
{
int iquant;
// generation de la partie positive
generate_default_quant( 0 );
generate_default_quant( 1 );
generate_default_quant( 2 );
generate_default_quant( 3 );
// symetrisation et inversion
for	( iquant = 0; iquant < 4; ++iquant )
	{
	symetrize( iquant );
	reverse( iquant );
	}
}

#include <stdio.h>

// production d'un fichier javascript pour graphiques 
int plot_js()
{
int iquant;
FILE * fil;
double x;

// initialisation des quantifieurs
init_quant();

fil = fopen("PLOT/plot_v42.js", "w" );

if	( fil == NULL )
	return 1;

for	( iquant = 0; iquant < 4; ++iquant )
	{
	fprintf( fil, "var plot_q%d = [\n", iquant );
	for	( x = 0.0; x <= 511.0; x += 0.2 )
		{
		fprintf( fil, "[%.1f,%d],", x-256.0, quant[iquant][(int)round(x)] );
		}
	fprintf( fil, "];\n");
	fprintf( fil, "var plot_d%d = [\n", iquant );
	for	( x = 0.0; x <= 511.0; x += 0.2 )
		{
		fprintf( fil, "[%.1f,%d],", x-256.0, dequant[iquant][quant[iquant][(int)round(x)]]-256 );
		}
	fprintf( fil, "];\n");
	}
fclose( fil );
return 0;
}

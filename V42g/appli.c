/* demo AVI --> AVI */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "v42.h"
#include "decode.h"
#include "encode.h"

void usage()
{ printf("usage : v42 source.avi dest.avi c|d|D|j <iq> <br>\n");
exit(1);
};

int main( int argc, char **argv )
{
int iquant, breadth;
printf("programme de codage-decodage v42 version g sur AVI BGR\n");

if	( argc < 4 )
	usage();

char opt = *argv[3];

if	( argc >= 5 )
	iquant = atoi( argv[4] );
else	iquant = 0;
if	( ( iquant < 0 ) || ( iquant > 3 ) )
	iquant = 0;

if	( argc >= 6 )
	breadth = atoi( argv[5] );
else	breadth = 30;
if	( ( breadth < 0 ) || ( breadth > 63 ) )
	breadth = 0;

init_quant();

if	( opt == 'd' )
	{
	decode_avi( argv[1], argv[2], 0 );
	}
else if	( opt == 'D' )
	{
	printf("decodage avec trace des vecteurs d'estimation de mouvement\n");
	decode_avi( argv[1], argv[2], 1 );
	}
else if	( opt == 'c' )
	{
	printf("quantificateur %d, ampleur estim. mouvt.+-%d x +-%d\n", iquant, breadth, breadth/2 );
	encode_avi( argv[1], argv[2], iquant, breadth );
	}
else if	( opt == 'j' )
	{
	printf("production du fichier \"./PLOT/plot_v42.js\" pour graphique des quantificateurs\n");
	if	( plot_js() )
		printf("erreur creation fichier\n");
	}
else	usage();

return 0;
}

// decodage des composantes du vecteur de mouvement et infos annexes
// MSB iXXXXXXXqqYYYYYY LSB
int extract_vect( short vectcode, int * pvx, int * pvy, int * pquant );

// copie d'un bloc BRG de 8x8 de la trame src a la trame dest
// stride = enjambement (difference d'adresse d'une ligne a l'autre)
void copy_bloc( unsigned char * dest, unsigned char * src, int x, int y, int stride );

// reconstruction d'un bloc a partir d'un predicteur (selon ref, px, py)
// et un residu (selon diff, x, y, iquant)
void reconst_bloc( unsigned char * dest, unsigned char * diff, unsigned char * ref,
		   int x, int y, int px, int py, int stride, int iquant );

// decodage d'une trame, de diff vers dest en cherchant des predicteurs dans ref
// en utilisant les vecteurs de mouvement et infos annexes lus dans vectcode 
int decode_trame( unsigned char * dest, unsigned char * diff, unsigned char * ref,
		  dims * dim, short * vectcode );

int decode_avi( char * video_in, char * video_out, int vectorscope );

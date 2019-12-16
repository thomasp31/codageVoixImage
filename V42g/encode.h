// encodage des composantes du vecteur de mouvement et infos annexes
// MSB iXXXXXXXqqYYYYYY LSB
short int encode_vect( int vx, int vy, int iquant, int intra );

// encodage d'une trame, de src vers diff en cherchant des predicteurs dans ref
// les vecteurs de mouvement et infos annexes sont mis dans vectcode 
// si force_keyframe == 1, tous les blocs sont simplement copies
int encode_trame( unsigned char * diff, unsigned char * src, unsigned char * ref,
		  dims * dim, short * vectcode, int force_keyframe, int iquant, int breadth );


int encode_avi( char * video_in, char * video_out, int iquant, int breadth );


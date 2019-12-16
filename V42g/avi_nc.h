/* Fonctions pour lire ou ecrire les frames d'un fichier AVI non comprime
   Chaque image RGB dans s->data contient :
   - les lignes en partant du bas
   - dans chaque ligne les pixels en partant de gauche,
     plus eventuellement des octets nuls pour arriver a un
     multiple de 4 bytes (s->ll)
   - pour chaque pixel trois octets B,G,R
 */

#ifndef O_BINARY
#define O_BINARY 0
#endif

typedef struct /* structure representant les parametres d'un fichier avi */
{
unsigned int wi, he; /* dimensions utiles en pixels */
unsigned int ll;     /* encombrement d'une ligne dans la base de donnees, en bytes */
unsigned int tot;   /* encombrement total des pixels d'une trame */
unsigned char *data; /* donnees brutes de la trame courante */
int hand;   /* descripteur de fichier */
char *fnam; /* nom de fichier */	
unsigned int usecPerFrame; /* duree frame */
unsigned int nframes;  /* nombre de frames du premier stream video */
int ivstream;  /* indice du premier stream video */
} avipars;



/* cette fonction assure l'ouverture du fichier AVI en lecture */
void readAVIheaders( avipars *s );

/* cette fonction lit la prochaine trame RGB entiere en memoire
   elle prend en charge l'allocation si s->data est NULL,
   sinon elle copie les donnees dans s->data qui doit
   pointer sur un espace dont la capacite est s->tot
*/
void readRGBframe( avipars *s );


/* cette fonction assure l'ouverture du fichier AVI en ecriture */
void writeAVIheaders( avipars *d );

/* cette fonction ecrit une trame RGB entiere */
void writeRGBframe( avipars *d );

/* cette fonction cree l'index a la fin du fichier avi puis
   le ferme */
void writeAVIindex( avipars *d );

void gasp( char *fmt, ... );  /* traitement erreur fatale */

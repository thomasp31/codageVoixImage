/* Fonctions pour lire ou ecrire les frames d'un fichier AVI non comprime
   Chaque image RGB dans s->data contient :
   - les lignes en partant du bas
   - dans chaque ligne les pixels en partant de gauche,
     plus eventuellement des octets nuls pour arriver a un
     multiple de 4 bytes (s->ll)
   - pour chaque pixel trois octets B,G,R
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include "avi_nc.h"

/* macro pour convertir une chaine 4 char en int  */

#define FCCI(ch4) *( (unsigned int*)ch4 )

/* ces fonctions ne sont pas portables sur les architectures 
   big-endian */

/* macro pour convertir une chaine 4 char en int  */

#define FCCI(ch4) *( (unsigned int*)ch4 )

/* fonction inverse */
char * IFCC( int i )
{
static char lbuf[5];
lbuf[0] = (char)(i);
lbuf[1] = (char)(i>>8);
lbuf[2] = (char)(i>>16);
lbuf[3] = (char)(i>>24);
lbuf[4] = 0;
return(lbuf);
}

/* macro pour arrondir les tailles de chunks */
#define RIFFROUND(cb) ((cb) + ((cb)&1))

/* cette fonction cherche a lire un chunk de type déterminé fcc.
   cas islist = 0 :
     si c'est le bon chunk et si le buffer est suffisant il est lu
        et sa taille est retournee.
     sinon il est saute et sa taille en negatif est retournee, sauf si isfatal (-->gasp)
   cas islist = 1
   si c'est la bonne liste sa taille est retournee mais elle n'est pas lue
   sinon il est sautee et sa taille en negatif est retournee, sauf si isfatal (-->gasp)
 */

int getChunk( int hand, char *fcc, void *buf, unsigned int bufsize,
              int islist, int isfatal )
{
unsigned int head[3];
unsigned int size;
if ( read( hand, head, 8 ) != 8 )
   gasp("fin de fichier atteinte sur recherche chunk %s", fcc );
size = RIFFROUND(head[1]);
// d'abord sauter les JUNK
if   ( head[0] == FCCI("JUNK") )
     {
     printf("on saute chunk %s de %d bytes\n", IFCC(head[0]), head[1] );
     lseek( hand, size, SEEK_CUR );
     return(-(int)size);
     }
if   ( islist )
     {
     if ( head[0] != FCCI("LIST") )
        gasp("liste n'a pas le fcc LIST mais %s", IFCC(head[0]) );
     if ( read( hand, head+2, 4 ) != 4 )
        gasp("fin de fichier atteinte sur recherche chunk %s", fcc );
     if   ( head[2]  !=  FCCI(fcc)  )
	  {
          if   (!isfatal)
               {
               printf("on saute chunk %s de %d bytes\n", IFCC(head[0]), head[1] );
               lseek( hand, (size-4), SEEK_CUR );
               return(-(int)size);
               }
          else gasp("echec sur recherche liste %s : vu %s", fcc, IFCC(head[2]) );
	  }
     return(size);
     }
else {
     // hack pour contourner l'ambiguite 00dc vs 00db : on masque le dernier caractere
     // si le premier est un zero
     if   (  ( ( head[0] & 0xFF ) == '0' )?
				( ( head[0] & 0x00FFFFFF ) != FCCI(fcc) ):
				( head[0] != FCCI(fcc) )
	  )
          {
          if   ( !isfatal )
               {
               printf("on saute chunk %s de %d bytes\n", IFCC(head[0]), head[1] );
               lseek( hand, size, SEEK_CUR );
	       return(-(int)size);
               }
          else gasp("echec sur recherche chunk %s : vu %s", fcc, IFCC(head[0]) );
          }
     if ( bufsize < size )
        gasp("memoire insufficante pour chunk %s de %d bytes", fcc, head[1] );
     if ( read( hand, buf, size ) < (int)size )
        gasp("fin de fichier atteinte sur contenu chunk %s", fcc );
     return(size);
     }
}

void readAVIheaders( avipars *s )
{
unsigned int hbuf[16];
int cnt, strl_size, strf_size, movi_size;	// hdrl_size
int nstreams, istream;

s->hand = open( s->fnam, O_RDONLY | O_BINARY );
if ( s->hand <= 0 )
   gasp("pb ouverture fichier %s en lecture", s->fnam );

/* lecture header RIFF */
cnt = read( s->hand, hbuf, 12 );
if ( cnt != 12 )
   gasp("pb lecture header fichier %s", s->fnam );

if ( hbuf[0] != FCCI("RIFF") )
   gasp("pb lecture fcc RIFF fichier %s", s->fnam );

/* printf("taille donnees RIFF : %d\n", hbuf[1] ); */

if ( hbuf[2] != FCCI("AVI ") )
   gasp("pb lecture fcc AVI fichier %s", s->fnam );

// hdrl_size = getChunk( s->hand, "hdrl", NULL, 0, 1, 1 ); 
/* printf("taille liste hdrl : %d\n", hdrl_size ); */
getChunk( s->hand, "hdrl", NULL, 0, 1, 1 ); 

/* lecture AVI main header */
getChunk( s->hand, "avih", hbuf, 64, 0, 1 );
s->usecPerFrame = hbuf[0];
s->wi = hbuf[8];
s->he = hbuf[9];
nstreams = hbuf[6];
printf("%d stream(s), flags = %08x, suggested buffer=%d\n",
       nstreams, hbuf[3], hbuf[7] );
printf("format %d x %d, %d microsecondes par trame\n",
        s->wi, s->he, s->usecPerFrame );

/* lecture stream headers */
s->ivstream = -1;
for ( istream = 0; istream < nstreams; istream++ )
    {
    strl_size = getChunk( s->hand, "strl", NULL, 0, 1, 1 ); 
    /* printf("taille liste strl : %d\n", strl_size ); */
    strl_size -= 4;
    /* lecture stream header */
    strl_size -= ( 8 + getChunk( s->hand, "strh", hbuf, 64, 0, 1 ) );
    if ( hbuf[0] == FCCI("vids") )
       printf("stream %d : video, codec \"%s\"\n", istream, IFCC(hbuf[1]) );
    if ( hbuf[0] == FCCI("auds") )
       printf("stream %d : audio, codec \"%s\"\n", istream, IFCC(hbuf[1]) );
    printf("  rate/scale = %d/%d, %d samples, suggested buffer %d, sample size=%d\n",
            hbuf[6], hbuf[5], hbuf[8], hbuf[9], hbuf[11] );
    if	(  ( hbuf[0] == FCCI("vids") ) &&
	   ( ( hbuf[1] == FCCI("DIB ") ) || ( hbuf[1] == 0 ) )	// fourcc = 0 <==> "DIB "
	)
       {
       int i;
       printf("  [left=%d, top=%d, right=%d, bottom=%d]\n",
               hbuf[12] & 0xFFFF, (hbuf[12]>>16) & 0xFFFF,
               hbuf[13] & 0xFFFF, (hbuf[13]>>16) & 0xFFFF );
       /* lecture stream format */
       if ( s->ivstream < 0 )
          {
          s->nframes = hbuf[8];
          s->ivstream = istream; /* premier stream video */
          strf_size = getChunk( s->hand, "strf", hbuf, 64, 0, 1 );
          strl_size -= ( 8 + strf_size );
          if ( ( strf_size != 40 ) ||
               ( hbuf[1] != s->wi ) ||
               ( hbuf[2] != s->he ) ||
               ( hbuf[4] != 0 )
             )
          gasp("video stream header mismatch");
          if ( hbuf[3] != 0x00180001 )
	     gasp("video stream not RGB");
          s->ll = s->wi * 3;
          i =  ( s->ll & 3 );        /* residu division par 4 */
          if (i) s->ll += ( 4 - i ); /* maintenant ll est divisible par 4 */
          if ( hbuf[5] != s->ll * s->he )
             gasp("desaccord sur encombrement image");
          printf("  encombrement image %d * %d = %d\n", s->he, s->ll, hbuf[5] );
          } 
       }
    /* printf( "reste %d a lire\n", strl_size ); */
    lseek( s->hand, strl_size, SEEK_CUR );
    } 
while ( ( movi_size = getChunk( s->hand, "movi", NULL, 0, 1, 0 ) ) < 0 )
      { }
/* printf("taille liste movi : %d\n", movi_size ); */
if ( s->ivstream < 0 )
   gasp("pas de stream video non compresse");
printf("stream %02d contenant %d trames de video non comprimee\n", s->ivstream, s->nframes );
}

/* cette fonction lit la prochaine trame RGB entiere en memoire
   elle prend en charge l'allocation si s->data est NULL,
   sinon elle copie les donnees dans s->data qui doit
   pointer sur un espace dont la capacite est s->tot.
*/

void readRGBframe( avipars *s )
{
char fcc[5];
/* on cherche des chunks type 00dc ou 00db, on compte sur getChunk pour ignorer le dernier char */
snprintf( fcc, sizeof(fcc), "%02dd", s->ivstream );

if   ( s->data == NULL )
     {
     s->data = malloc( s->tot );
     if ( s->data == NULL )
        gasp("pb alloc memoire %d bytes", s->tot );
     }

while ( ( getChunk( s->hand, fcc, s->data, s->tot, 0, 0 ) ) < 0 )
      { }
}

void writeAVIheaders( avipars *d )
{
unsigned int hbuf[16];
unsigned int avih_size, strh_size, strf_size, strl_size,
             hdrl_size, movi_size, riff_size, idx1_size;
int cnt;

// umask(0);
d->hand = open( d->fnam, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, 0666 );
if ( d->hand <= 0 )
   gasp("pb ouverture fichier %s en ecriture", d->fnam );

/* calculs de taille 
RIFF AVI
  LIST hdrl
     avih
     LIST strl
       strh
       strf
  LIST movi
    00dc
    00dc
  idx1
les tailles ci-dessous sont "tout compris"
*/
avih_size = 8 + 56;
strh_size = 8 + 56;
strf_size = 8 + 40;
strl_size = 12 + strh_size + strf_size; 
hdrl_size = 12 + avih_size + strl_size;
movi_size = 12 + d->nframes * ( 8 + d->tot );
idx1_size = 8 + 16 * d->nframes;
riff_size = 12 + hdrl_size + movi_size + idx1_size;

hbuf[0] = FCCI("RIFF");
hbuf[1] = riff_size - 8;
hbuf[2] = FCCI("AVI ");
hbuf[3] = FCCI("LIST");
hbuf[4] = hdrl_size - 8;
hbuf[5] = FCCI("hdrl");
cnt = 4 * 6;
if ( cnt != write( d->hand, hbuf, cnt )  )
   gasp("pb ecriture %s (disque plein ?)", d->fnam );

/* AVI main header */
#define AVIF_HASINDEX        0x00000010 // Index at end of file?
#define AVIF_ISINTERLEAVED   0x00000100
memset( hbuf, 0, sizeof(hbuf) );
hbuf[0] = FCCI("avih");
hbuf[1] = avih_size - 8;
hbuf[2] = d->usecPerFrame;
hbuf[5] = AVIF_HASINDEX + AVIF_ISINTERLEAVED;
hbuf[6] = d->nframes;
hbuf[8] = 1;  /* nstreams */
hbuf[10] = d->wi;
hbuf[11] = d->he;
cnt = avih_size;
if ( cnt != write( d->hand, hbuf, cnt )  )
   gasp("pb ecriture %s (disque plein ?)", d->fnam );

hbuf[0] = FCCI("LIST");
hbuf[1] = strl_size - 8;
hbuf[2] = FCCI("strl");
cnt = 4 * 3;
if ( cnt != write( d->hand, hbuf, cnt )  )
   gasp("pb ecriture %s (disque plein ?)", d->fnam );

/* stream header for one video stream */
memset( hbuf, 0, sizeof(hbuf) );
hbuf[0] = FCCI("strh");
hbuf[1] = strh_size - 8;
hbuf[2] = FCCI("vids");
hbuf[3] = FCCI("DIB ");
hbuf[8] = 1000000;
hbuf[7] = d->usecPerFrame;
hbuf[10] = d->nframes;
hbuf[11] = d->tot;  /* suggested buffer */
hbuf[13] = 0;  /* sample size, but 0 means "1 frame per chunk" */
hbuf[14] = 0; /* left, top */
hbuf[15] = d->wi + ( d->he << 16 ); /* right, bottom */

cnt = strh_size;
if ( cnt != write( d->hand, hbuf, cnt )  )
   gasp("pb ecriture %s (disque plein ?)", d->fnam );

/* stream format header genre BITMAPINFOHEADER du format bmp */
memset( hbuf, 0, sizeof(hbuf) );
hbuf[0] = FCCI("strf");
hbuf[1] = strf_size - 8;
hbuf[2] = strf_size - 8;  /* GAG */
hbuf[3] = d->wi;
hbuf[4] = d->he;
hbuf[5] = 0x00180001; /* 24 bits/pixel, 1 plane */
hbuf[6] = 0;  /* no compression */
hbuf[7] = d->tot;

cnt = strf_size;
if ( cnt != write( d->hand, hbuf, cnt )  )
   gasp("pb ecriture %s (disque plein ?)", d->fnam );

hbuf[0] = FCCI("LIST");
hbuf[1] = movi_size - 8;
hbuf[2] = FCCI("movi");
cnt = 4 * 3;
if ( cnt != write( d->hand, hbuf, cnt )  )
   gasp("pb ecriture %s (disque plein ?)", d->fnam );
}

void writeRGBframe( avipars *d )
{
unsigned int hbuf[2];
int cnt;
hbuf[0] = FCCI("00dc");
hbuf[1] = d->tot;
cnt = 8;
if ( cnt != write( d->hand, hbuf, cnt )  )
   gasp("pb ecriture %s (disque plein ?)", d->fnam );
cnt = d->tot;
if ( cnt != write( d->hand, d->data, cnt )  )
   gasp("pb ecriture %s (disque plein ?)", d->fnam );
}

/* cette fonction cree l'index a la fin du fichier avi puis
   le ferme */

#define AVIIF_KEYFRAME   0x00000010

void writeAVIindex( avipars *d )
{
unsigned int hbuf[4];
int cnt, ifram;
unsigned int idx1_size;
idx1_size = 8 + 16 * d->nframes;
hbuf[0] = FCCI("idx1");
hbuf[1] = idx1_size - 8;
cnt = 8;
if ( cnt != write( d->hand, hbuf, cnt )  )
   gasp("pb ecriture %s (disque plein ?)", d->fnam );
hbuf[0] = FCCI("00dc");
hbuf[3] = d->tot;
for ( ifram = 0; ifram < (int)d->nframes; ifram++ )
    {
    hbuf[1] = AVIIF_KEYFRAME; /* flags */
    hbuf[2] = ifram * ( d->tot + 8 ) + 4; /* offset */
    cnt = 16;
    if ( cnt != write( d->hand, hbuf, cnt )  )
       gasp("pb ecriture %s (disque plein ?)", d->fnam );
    }
close( d->hand );
}

/* --------------------------------------- traitement erreur fatale */

void gasp( char *fmt, ... )
{
  char lbuf[2048];
  va_list  argptr;
  va_start( argptr, fmt );
  vsprintf( lbuf, fmt, argptr );
  va_end( argptr );
  printf("STOP : %s\n", lbuf ); exit(1);
}

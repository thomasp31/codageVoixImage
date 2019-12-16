typedef struct
{
unsigned int wi, he; /* dimensions utiles en pixels */
unsigned int ll;     /* encombrement d'une ligne dans la base de donnees, en bytes */
unsigned int tot;   /* encombrement total des pixels d'une trame */
} dims;

extern int dequant[4][256];
extern unsigned char quant[4][512];

// initialisation des quantifieurs (appeler 1 fois)
void init_quant();
int plot_js();

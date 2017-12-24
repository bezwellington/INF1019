

int shms[4];
int semId;
int semId2;
int semId3;
int semId4;
int semSignalId;
int semContador;
int segmento_contadores;
//posicao   -   contador
//   0      -   pageFaults
//   1      -   modificacao
//   2      -   acesso
int *contadores;
int pageFaults[4];

#define TRUE 1
#define FALSE 0

#ifndef TP

  typedef struct pagina {

  char r_w;
	int id_processo;
	int contador_acesso; //contador de acesso
        char r; //reference byte
        char m; //modified byte
        char p; //see if page is present
	int end_fisico;
  }Pagina;

  #define TP

#endif

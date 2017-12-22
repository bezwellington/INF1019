
/****************************************************************************************************
 *    PUCRIO
 *    TRABALHO 1 - Sistemas de Computação
 *    Professor: Luiz Fernando Seibel
 *    25/10/2017
 *
 *    Aluno:    Wellington Bezerra da Silva - 1413383
 *    Aluno:    Guilherme de Azevedo Pereira Marques - 1220676
 *
 ****************************************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define MAX_PROG 5 /* num max de programas */
#define MAX_TAM 10
#define MAX_PRIO 15
#define MAX_NOME 20
#define N 100
#define EVER ;;
int pedi_IO = 0;
int terminei = 0;




/****************************************************************************************************
 *
 *    PROCESSO
 *
 ****************************************************************************************************/

//Estruturas de dados auxiliares.
typedef struct infoProcesso {
    char nome[MAX_NOME];                                          /* Nome do programa referente ao processo */
    int pid;                                                     /* PID do processo */
    int rajada[MAX_PRIO];                                       /* rajada do processo */
    int index_rajada;											                     /* index da rajada que esta sendo utilizada */
    int total_rajadas;											                  /* total de rajadas lidas */
    int fila_atual;                                          /* fila que o processo está */
    int tempo_io;                                        /* tempo que o processo fica em io */
    int to_em_io;
} InfoProcesso;

/***************************************************************************************************/

/****************************************************************************************************
 *
 *    FILA
 *
 ****************************************************************************************************/

typedef struct fila {
    int n; /* numero de elementos armazenados na fila */
    int ini; /* indice para o inicio da fila */
    InfoProcesso **vet;
    int prio;
    int deltaT;
} Fila;


/****************************************************************************************************
 *
 *    FUNCAO - FILA CRIA
 *
 ****************************************************************************************************/
/* Essa funcao para criar a fila aloca dinamicamente essa estrutura e inicializa a fila como sendo vazia. */
Fila* fila_cria(int prioridade, int deltaT) {

    Fila* f = (Fila*) malloc(sizeof(Fila));
    f->n = 0; /* inicializa fila vazia */
    f->ini = 0; /* escolhe uma posicao inicial */
    f->prio = prioridade;
    f->deltaT = deltaT ;
    f->vet = (InfoProcesso**) malloc(N*sizeof(InfoProcesso*));
    return f;
}


/****************************************************************************************************
 *
 *    FUNCAO - FILA VAZIA
 *
 ****************************************************************************************************/
/* Verifica se a fila esta vazia. */
int fila_vazia(Fila *f) {
    return (f->n == 0);
}


/****************************************************************************************************
 *
 *    FUNCAO - FILA INSERE
 *
 ****************************************************************************************************/
/* Para inserir um elemento na fila, usamos a prox posicao livre do vetor, indicada por fim. */
void fila_insere(Fila* f, InfoProcesso *processo) {
    // printf("FILA INSERE\n");
    int fim;
    if (f->n == N) { /* fila cheia: capacidade esgotada */
        printf("Capacidade da fila estourou. \n");
        exit(1);
    }
    /* insere elemento na prox posicao livre */
    fim = (f->ini + f->n) % N;
    f->vet[fim] = processo;
    f->n++;
}


/****************************************************************************************************
 *
 *    FUNCAO - FILA RETIRA
 *
 ****************************************************************************************************/
/* A funcao para retirar o elemento do inicio da fila fornece o valor do elemento retirado como retorno. */
InfoProcesso * fila_retira(Fila* f) {
    InfoProcesso *processo = (InfoProcesso *) malloc(sizeof(InfoProcesso));
    // printf("ANTES DE RETIRAR - Fila de prioridade %d possui %d elementos\n", f->prio, f->n);
    if (fila_vazia(f)) {
        printf("Fila vazia. \n");
        exit(1);
    }
    processo = f->vet[f->ini];
    f->ini = (f->ini + 1) % N;
    f->n--;
    // printf("DPS DE RETIRAR - Fila de prioridade %d possui %d elementos\n", f->prio, f->n);
    return processo;
}


/****************************************************************************************************
 *
 *    FUNCAO - FILA LIBERA
 *
 ****************************************************************************************************/
/* Funcao para liberar a memoria alocada pela fila. */
void fila_libera(Fila* f) {
  for(int i = 0; i< N; i++) {
    free(f->vet[i]);
  }
  free(f->vet);
  free(f);
}


/****************************************************************************************************
 *
 *    VARIAVEIS GLOBAIS
 *
 ****************************************************************************************************/
Fila *f1, *f2, *f3, *fProcessosIO;
Fila *filas[3];
InfoProcesso *current_proccess = NULL;

/****************************************************************************************************
 *
 *    MAIN
 *
 ****************************************************************************************************/

void interpretador(InfoProcesso **vetorProcesso, int* numProgramas);
int escalonador (InfoProcesso **vetorProcesso, int * numProgramas);
int criaPrograma (int n, InfoProcesso *processo);
void executando(int time_slice, InfoProcesso *processo, Fila *filas[], Fila *fProcessosIO);
void iniciaIO();
void terminaIO();
void finalizacaoPrograma(int signal);

/***************************************************************************************************/

int main (void) {

  f1 = fila_cria(0, 1);
  f2 = fila_cria(1, 2);
  f3 = fila_cria(2, 4);
  fProcessosIO = fila_cria(3, 0);

  filas[0] = f1;
  filas[1] = f2;
  filas[2] = f3;

  InfoProcesso **vetorProcesso = (InfoProcesso**) malloc(MAX_PROG*sizeof(InfoProcesso*));
  int * numProgramas = malloc(sizeof(*numProgramas));
  int pidInterpretador;
  int i;
  int fd[2];

  //numero do sinal a ser tratado + endereço da funcao
  if(signal(SIGUSR1, iniciaIO) == SIG_ERR || signal(SIGUSR2, terminaIO) == SIG_ERR) {
    printf("ERRO AO INSTALAR ROTINA DE ATENDIMENTO AO SINAL\n");
    exit(-1);
  } else {
    printf("ROTINAS DE ATENDIMENTO AOS SINAIS USR1 e USR2 OK\n");
  }

  if(signal(SIGCHLD, finalizacaoPrograma) == SIG_ERR) {
    printf("ERRO AO INSTALAR ROTINA DE ATENDIMENTO AO SINAL\n");
    exit(-1);
  } else {
    printf("ROTINAS DE ATENDIMENTO AOS SINAL SIGCHLD OK\n");
  }

  InfoProcesso vetor[MAX_PROG];

  // ponteiro que aponte para cada um dos arquivos txt
  //FILE * fp1 = fopen ("entrada.txt", "r") ; /* Abertura de arquivo de entrada */
  //FILE * fp2 = fopen ("saida.txt", "w") ;   /* Abertura de arquivo de saida  */

  // cria um canal de comunicacao entre os processos
  // fd[0]: leitura
  // fd[1]: escrita
  if(pipe(fd) < 0) {
    printf("Erro no pipe\n");
    exit(-1);
  }

  *numProgramas = 0;



  // cria um processo filho identico ao pai
  pidInterpretador = fork();

  // pipe do filho para o pai (do interpretador para o escalonador) - filho fecha fd[0]
  if(pidInterpretador == 0) { // Filho - Interpretador
    close(fd[0]);
    interpretador(vetorProcesso, numProgramas);
    // interpretador (fp1, vetorProcesso, numProgramas, statusInterp) ;   /* abre o araquivo entrada.txt e preenche o vetorProcesso */
                                            /* Nome do programa referente ao processo */

    for(int i = 0; i < *numProgramas; i++) {
      strcpy(vetor[i].nome, vetorProcesso[i]->nome);
      vetor[i].pid = vetorProcesso[i]->pid;
      for(int j=0; j < vetorProcesso[i]->total_rajadas; j++) {
        vetor[i].rajada[j] = vetorProcesso[i]->rajada[j];                                       /* rajada do processo */
      }                                                     /* PID do processo */
      vetor[i].index_rajada = vetorProcesso[i]->index_rajada;                                          /* index da rajada que esta sendo utilizada */
      vetor[i].total_rajadas = vetorProcesso[i]->total_rajadas;                                        /* total de rajadas lidas */
      vetor[i].fila_atual = vetorProcesso[i]->fila_atual;                                          /* fila que o processo está */
      vetor[i].tempo_io= vetorProcesso[i]->tempo_io;
    }
    write(fd[1], numProgramas, sizeof(int));                              /* escreve na pipe o numProgramas */
    write(fd[1], vetor, *numProgramas*sizeof(InfoProcesso));        /* escreve na pipe o vetorProcesso */

  } else { // Pai - Escalonador
    waitpid(-1,0,0);
    close(fd[1]);
    read(fd[0], numProgramas, sizeof(int));                              /* le na pipe o numProgramas */
    read(fd[0], vetor, *numProgramas*sizeof(InfoProcesso));         /* le na pipe o vetorProcesso */


    for(int i = 0; i < *numProgramas; i++) {
      vetorProcesso[i] = (InfoProcesso*) malloc(sizeof(InfoProcesso));
      strcpy(vetorProcesso[i]->nome, vetor[i].nome);
      vetorProcesso[i]->pid = vetor[i].pid;
      for(int j=0; j < vetor[i].total_rajadas; j++) {
        vetorProcesso[i]->rajada[j] = vetor[i].rajada[j];                                    /* rajada do processo */
      }                                                     /* PID do processo */
      vetorProcesso[i]->index_rajada = vetor[i].index_rajada;                                          /* index da rajada que esta sendo utilizada */
      vetorProcesso[i]->total_rajadas = vetor[i].total_rajadas;                                        /* total de rajadas lidas */
      vetorProcesso[i]->fila_atual = vetor[i].fila_atual;                                          /* fila que o processo está */
      vetorProcesso[i]->tempo_io= vetor[i].tempo_io;
    }


    while(*numProgramas == 0); //Espera ler ao menos um programa para iniciar escalonamento

    // printf("ESCALONADOR");

    escalonador(vetorProcesso, numProgramas) ;
                               /* Chama funcao do escalonador */
    printf("Escalonamento chegou ao fim! \n");
    kill(pidInterpretador, SIGKILL);
  }



  return 0;
}


/****************************************************************************************************
 *
 *    FUNCAO - INTERPRETADOR
 *
 ****************************************************************************************************/
void interpretador(InfoProcesso **vetorProcesso, int* numProgramas) {
  int j = 0;
  *numProgramas = 0;
  // *statusInterp = 0;
  char exec[MAX_NOME];
  int total_rajadas;
  printf("Entre com um número de 1 a 15 de programas: \n");
  scanf("%ds", numProgramas);

  printf("Entre com o número positivo de rajadas que todos os programas terão: \n");
  scanf("%d", &total_rajadas);

  for(int i = 0; i < *numProgramas; i++) { /* inicia a leitura do arquivo entrada */
    vetorProcesso[i] = (InfoProcesso*) malloc(sizeof(InfoProcesso));
    printf("Digite o nome do programa %d:\n", i+1);
    scanf("%s", &(vetorProcesso[i]->nome));

    for(j = 0; j < total_rajadas; j++) {
      printf("Digite o tempo da %d rajada do programa %s \n", j+1, vetorProcesso[i]->nome);
      scanf("%d", &vetorProcesso[i]->rajada[j]);
      if(vetorProcesso[i]->rajada[j] < 0) {
        vetorProcesso[i]->rajada[j] = 0;
      }
    }

    vetorProcesso[i]->total_rajadas = total_rajadas;
    vetorProcesso[i]->fila_atual = 0;
    vetorProcesso[i]->to_em_io = 0;
  }

}

/****************************************************************************************************
 *
 *    FUNCAO - CRIA PROGRAMA
 *
 ****************************************************************************************************/

int criaPrograma (int n, InfoProcesso *processo) {
  int pid;
  char nomePrograma[10] = "programa";
  sprintf(nomePrograma, "programa%d", n);
  pid = fork(); /* Executa o fork */

  if (pid < 0) { /* Com problemas no fork retorna erro */
    return -1;

  } else if ( pid != 0 ) { /* PAI */
    kill(pid, SIGSTOP);                     /* Pausa processo filho no início da execução */
    // printf("PROCESSO %d PAUSADO!\n", pid);
    return pid;
  } else { /* FILHO */
    // printf("nome do programa: %s \n", nomePrograma );
    char ** strs = (char **) malloc((processo->total_rajadas+2)*sizeof(char*));
    strs[0] = nomePrograma;
    for(int i = 1; i < processo->total_rajadas+1; i++) {
      strs[i] = (char *) malloc(sizeof(char));
      sprintf(strs[i], "%d", processo->rajada[i-1]);
      if(i == processo->total_rajadas+1) {
        strs[i] = NULL;
      }
    }
    execv(nomePrograma, strs);
  }
  return 0;
}

/****************************************************************************************************
 *
 *    FUNCAO  - ESCALONADOR
 *
 ****************************************************************************************************/

int escalonador (InfoProcesso **vetorProcesso, int * numProgramas) {

  int i;
  if( (f1 == NULL) || (f2 == NULL) || (f3 == NULL) ) {                        /* verifica se a criacao de alguma fila deu errado */
    printf("Erro ao criar uma das filas");
    exit(-1);
  }
  else{
    printf("fila 0 criada com sucesso \n");
    printf("fila 1 criada com sucesso \n");
    printf("fila 2 criada com sucesso \n");
  }

  for (i=0; i < *numProgramas; i++) { /* insere os programas na 1a fila */

    vetorProcesso[i]->pid = criaPrograma(i, vetorProcesso[i]);
    vetorProcesso[i]->tempo_io = 0;
    // printf("VAI ENTRAR NA FILA INSERE\n");
    fila_insere(filas[0], vetorProcesso[i]);
    // printf("SAI DA FILA INSERE\n");
    // printf("NOME: %s \n", vetorProcesso[i]->nome );
    // printf("PID: %d\n",vetorProcesso[i]->pid);
    // printf("TOTAL RAJADAS: %d\n",vetorProcesso[i]->total_rajadas);
    // printf("Inseriu na FILA 1");
  }
  // printf("***** TERMINOU DE INSERIR OS ELEMENTOS NA FILA 0 ***** \n");

  struct timespec time1, time2;
  time1.tv_sec = 0;
  time2.tv_nsec = 1000000000;

  /************************ INÍCIO DO PROCESSO DE ESCALONAMENTO  **********************/

  while(!fila_vazia(f1) || !fila_vazia(f2) || !fila_vazia(f3) || !fila_vazia(fProcessosIO)) {

    if (fila_vazia(f1) != 1) {
      printf("FILA 1\n");
      fflush(stdout);
      // printf("TIME SLICE: %d \n", filas[0]->deltaT);

      current_proccess = fila_retira(filas[0]);
      // printf("nome: %s - PID: %d\n", current_proccess->nome, current_proccess->pid);
      executando(filas[0]->deltaT, current_proccess, filas, fProcessosIO);

    } else if (fila_vazia(f2) != 1) {
      printf("FILA 2\n");
      fflush(stdout);
      // printf("TIME SLICE: %d \n", filas[1]->deltaT);

      current_proccess = fila_retira(filas[1]);
      // printf("nome: %s - PID: %d\n", current_proccess->nome, current_proccess->pid);
      executando(filas[1]->deltaT, current_proccess, filas, fProcessosIO);

    } else if (fila_vazia(f3) != 1) {
      printf("FILA 3\n");
      fflush(stdout);
      // printf("TIME SLICE: %d \n", filas[2]->deltaT);

      current_proccess = fila_retira(filas[2]);
      // printf("nome: %s - PID: %d\n", current_proccess->nome, current_proccess->pid);
      executando(filas[2]->deltaT, current_proccess, filas, fProcessosIO);
	   } else if (fila_vazia(fProcessosIO) != 1) {

     }
    //  printf("==========================================\n");
    //  printf("Fila de prioridade %d possui %d elementos\n", f1->prio, f1->n);
    //  printf("Fila de prioridade %d possui %d elementos\n", f2->prio, f2->n);
    //  printf("Fila de prioridade %d possui %d elementos\n", f3->prio, f3->n);
    //  printf("Fila de IO possui %d elementos\n", fProcessosIO->n);
    //  printf("==========================================\n");

     if(fila_vazia(f1) && fila_vazia(f2) && fila_vazia(f3) && fila_vazia(fProcessosIO)) {
       printf("NÃO HÁ MAIS PROCESSOS PARA SEREM ESCALONADOS\n");
       exit(1);
     }

  }
  return 0;
}


/****************************************************************************************************
 *
 *    FUNCAO - MUDA PROCESSO DE FILA
 *
 ****************************************************************************************************/
// sobe_ou_desce = 1 : processo desce
// sobe_ou_desce = -1 : processo sobe
void  mudaProcessoDeFila(InfoProcesso *processo, int sobe_ou_desce) {
  if(processo->fila_atual == 0) {
    if(sobe_ou_desce == 1) {
      processo->fila_atual += sobe_ou_desce;
    }
    return;
  }
  if(processo->fila_atual == 2) {
    if(sobe_ou_desce == -1) {
      processo->fila_atual += sobe_ou_desce;
    }
    return;
  }
  processo->fila_atual += sobe_ou_desce;
}


/****************************************************************************************************
 *
 *    FUNCAO - INSERE NA FILA
 *
 ****************************************************************************************************/
void insere_na_fila(Fila *filas[], InfoProcesso *processo) {
  // printf("%d\n", processo->fila_atual);
  Fila *fila_a_receber_processo = filas[processo->fila_atual];
  // printf("+++++++++++++++++++++++++++++++");
  // printf("Inserindo processo de pid %d - na fila de prioridade %d\n", processo->pid, fila_a_receber_processo->prio);
  // printf("+++++++++++++++++++++++++++++++");
  fila_insere(fila_a_receber_processo, processo);

}

/****************************************************************************************************
 *
 *    FUNCAO - EXECUTANDO
 *
 ****************************************************************************************************/
void executando(int time_slice, InfoProcesso *processo, Fila *filas[], Fila *fProcessosIO) {
  int index = processo->index_rajada;
  if(processo->rajada[index] == 0){
  	processo->total_rajadas -= 1;
  	processo->index_rajada += 1;
  }

  index = processo->index_rajada;

  // struct timespec time1, time2;
  // time1.tv_sec = 0;
  // time2.tv_nsec = 1000000000;
  // INSTANTE DE INICIO DA EXECUCAO DO PROCESSO ATUAL
  // printf("**** TO EXECUTANDO **** ");
  // printf("PID ATIVO = %d\n", processo->pid);
  // printf("burst = %d\n", processo->rajada[index]);
  kill(processo->pid, SIGCONT);
  int start = time(NULL);
  int tempo_passado = time(NULL) - start;

  while((time_slice > tempo_passado) && !pedi_IO && !terminei) {
    usleep(1000000);
    // nanosleep(&time1, &time2);
    // printf("Passei 1s\n");
    tempo_passado = time(NULL) - start;
  }
  // printf("**** TEMPO PASSADO %d ****\n", tempo_passado);

  if(terminei || (index == processo->total_rajadas && processo->rajada[processo->total_rajadas] <= time_slice)) {
    // printf("TEEEEEEERMINEEEEEI ALELUIA\n");
    //printf("total rajadas %d - rajada %d - index %d - timeslice %d\n", processo->total_rajadas, processo->rajada[processo->total_rajadas-1], index, time_slice);
    fila_retira(filas[processo->fila_atual]);
    // if (fila_vazia(fProcessosIO) != 1) {
    //   fila_retira(fProcessosIO);
    // }
    terminei = 0;

  } else if(!terminei && (pedi_IO || processo->rajada[index] <= time_slice)) {
    // printf("ENTRA EM I/O");
    processo->rajada[index] = 0;
    mudaProcessoDeFila(processo, -1);
    fila_insere(fProcessosIO, processo);
    pedi_IO = 0;
  } else {
    // printf("NAO ENTRA EM I/O");
    kill(processo->pid, SIGSTOP);
    mudaProcessoDeFila(processo, 1);
    insere_na_fila(filas, processo);
    // printf("PROCESSO FOI PARA FILA %d\n", processo->fila_atual);
    processo->rajada[index] -= time_slice;
  }
  // printf("burst = %d\n", processo->rajada[index]);
}


/****************************************************************************************************
 *
 *    FUNCAO - INICIA I/O
 *
 ****************************************************************************************************/
void iniciaIO() {
	pedi_IO = 1;
  current_proccess->to_em_io = 1;
}


/****************************************************************************************************
 *
 *    FUNCAO TERMINA I/O
 *
 ****************************************************************************************************/
void terminaIO() {

// tira da fila de IO
// checar a prioridade do processo retirado da fila
// e inseri-lo na fila de sua prioridade
  // printf("CHAMEI TERMINA IO\n");
  InfoProcesso *aux = fila_retira(fProcessosIO);
  aux->to_em_io = 0;
  kill(aux->pid, SIGSTOP);
  // printf("**** PROCESSO %d SAIU DE IO ****\n", aux->pid);
  insere_na_fila(filas, aux);

}

void finalizacaoPrograma(int signal) {
  int status;
  if(current_proccess != NULL) {
    pid_t pid = waitpid(current_proccess->pid, &status, WNOHANG);
    // printf("MOLEQUE DOIDO Q TA TERMINANDO = %d sera?\n", pid);
    // printf("%d\n", status);
    if (pid == current_proccess->pid) {
      terminei = 1;
      printf("Programa terminou\n");
    }
  }

}

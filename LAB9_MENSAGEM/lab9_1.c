#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>

/* 
Tamanho máximo da fila é 8.
Tamanho máximo da mensagem é 100.
Número de mensagens 64.

Estrutura da mensagem é definida como "message_buf"

Nesse programa, os processos irão se comunicar por meio da troca de mensagens.
O envio e recebimento de mensagens é feito utilizando as funções msgsnd() e msgrcv, respectivamente. 
IPC_CREAT é utilizado para especificar a chave da mensagem
A função msgget() retorna o identificador da fila de mensagem
O produtor é executado em um processo de 1 em 1 segundo colocando um elemendo na lista de mensagem, 
enquanto existe dois consumidores, cada um sendo executado em um processo de 2 em 2 segundos e consumindo
um elemento do buffer de mensagem.

No final da execução do programa, todos os 64 itens serão produzidos pelo produtor 
e consumidos pelos consumidores 

*/

#define TAMFILA 8   /* Tamanho da fila */
#define MSGMAX 100  /* tamanho maximo da mensagem */
#define MSGNUM 64   /* Numero de mensagens */

/* Estrutura do buffer da mensagem */
typedef struct msgbuf {
  long    mtype;                /* tipo da mensagem */
  char    mtext[MSGMAX];        /* texto da mensagem */
} message_buf;

/* Variaveis globais */
int message_count = 0;
int msgid;

void  consumidor(int id) {

  int i = 0;
  message_buf rbuf;

  while (i < MSGNUM/2) {
    sleep(2);
    if (msgrcv(msgid, &rbuf, MSGMAX, 1, 0) < 0) {
      printf("Erro - consumidor %d. \n", id);
    }

    printf("Consumidor %d: %s \n", id, rbuf.mtext);
    i++;
  }
}

void produtor() {

  int i;
  message_buf sbuffer;
  size_t buf_size;

  for (i = 0; i < MSGNUM; i++){
    sbuffer.mtype = 1;
    sprintf(sbuffer.mtext, "Mensagem: %d", i);
    buf_size = strlen(sbuffer.mtext) + 1 ;

    if (msgsnd(msgid, &sbuffer, buf_size, IPC_NOWAIT) < 0) {
      printf("Erro  - produtor. \n");
    }

    printf("Produtor: %s\n", sbuffer.mtext);

    sleep(1);
  }
}


int main (void) {
  key_t chave = 9876;

  if ((msgid = msgget(chave, IPC_CREAT | 0666 )) < 0) 
    exit(1);

  if (fork() == 0) {            /* Processo - Consumidor 0 */
    consumidor(0);

  } else if(fork() == 0) {      /* Processo - Consumidor 1 */
    consumidor(1);

  } else {
    produtor();                 /* Processo - Produtor */

  }

  return 0;
}
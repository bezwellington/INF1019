#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>

/* DEFINE */
#define MSGMAX 100
#define TAMFILA 8
#define MSGNUM 64 /* Numero de mensagens */

/* Estrutura do buffer da mensagem */
typedef struct msgbuf {
	long    mtype;						/* tipo da mensagem */
	char    mtext[MSGMAX];				/* texto da mensagem */
} message_buf;

/* Variavies globais */
int idMSG;
int idMSG2;

void  consumidor(int id) {

	message_buf rbuffer;
	int i = 0;

	while (i < MSGNUM/2) {
		sleep(2);

		if((id % 2) != 0) {											/* id = 1 */
			if (msgrcv(idMSG2, &rbuffer, MSGMAX, 1, 0) < 0) {
				printf("Erro - consumidor %d. \n", id);
			}

		} else {													/* id = 0 */
			if (msgrcv(idMSG, &rbuffer, MSGMAX, 1, 0) < 0) {
				printf("Erro - consumidor %d. \n", id);
			}

		}

		printf("Consumidor %d: %s \n", id, rbuffer.mtext);

		i++;
	}
}

void produtor() {
	int i;
	message_buf sbuf;
	size_t buf_size;

	for (i = 0; i < MSGNUM; i++){
		sbuf.mtype = 1;
		sprintf(sbuf.mtext, "Msg %d", i);
		buf_size = strlen(sbuf.mtext) + 1 ;

		if((i % 2) != 0) {													/* id = 1 */
			if (msgsnd(idMSG2, &sbuf, buf_size, IPC_NOWAIT) < 0) {
				printf("Erro - produtor. \n");
			}

		} else {															/* id = 0 */
			if (msgsnd(idMSG, &sbuf, buf_size, IPC_NOWAIT) < 0) {
				printf("Erro - produtor. \n");
			}

		}
		int aux = ((i % 2) == 0)? 0 : 1;
		printf("Produtor para consumidor %d: %s\n", aux, sbuf.mtext);

		sleep(1);
	}
}

int main (void) {

  	key_t chave1 = 1235;
	key_t chave2 = 1234;

	if ((idMSG = msgget(chave1, IPC_CREAT | 0666 )) < 0) 
		exit(1);

	if ((idMSG2 = msgget(chave2, IPC_CREAT | 0666 )) < 0) 
		exit(1);

	if (fork() == 0) {
		consumidor(0);				/* Processo - Consumidor 0 */

	} else if(fork() == 0) {
		consumidor(1);				/* Processo - Consumidor 1 */

	} else {
		produtor();					/* Processo - Produtor */

	}

	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <signal.h>
#include "semaphore.h"
#include "gm.h"
#include "vm.h"
#include <sys/sem.h>
#include <errno.h>
#include <time.h>

#define MAXTABELA 65536

int lfu();
int criaTabelaPaginas(int paginaTam, int id);
unsigned retIndPag(unsigned addr);
void alocaPag(int pid,int num_proc, int index);
void perdaPagHandler(int signal);
void sigHandler(int signal, siginfo_t *siginfo, void *context);

int P1,P2,P3,P4;
int memoria_principal[10];
int memoria_principal_proc[10];
int inicio_sim;

unsigned retIndPag(unsigned addr){
	unsigned page = addr >> 16;
	return page;
}

int verificaMemPrinc(int *posDisponivel) {
	int i = 0;
	while( i < 256 ) {
		if(memoria_principal[i] == -1) {
			*posDisponivel = i;
			return FALSE;
		}
		i++;
	}
	return TRUE;
}

int lfu(){
	int i = 0;
	int menor = 0;
	Pagina *t;

	while(i < 10){

		if(memoria_principal_proc[i] == 1) {
			t = (Pagina *)shmat(shms[0], NULL, 0);
		} else if(memoria_principal_proc[i] == 2) {
			t = (Pagina *)shmat(shms[1], NULL, 0);
		} else if(memoria_principal_proc[i] == 3) {
			t = (Pagina *)shmat(shms[2], NULL, 0);
		} else if(memoria_principal_proc[i] == 4) {
			t = (Pagina *)shmat(shms[3], NULL, 0);
		}

		if((t[memoria_principal[menor]].contador_acesso > t[memoria_principal[i]].contador_acesso)){
			menor = i;
		} else if((t[memoria_principal[menor]].contador_acesso == t[memoria_principal[i]].contador_acesso)){
			if(t[memoria_principal[menor]].m == 0 && t[memoria_principal[i]].m == 1)	menor = i;
			shmdt(t);
		}
		i++;

	}

	return menor;
}

void alocaPag(int pid,int num_proc, int index) {
	//tabela e a shm do processo que chamou o allocate page por meio do sinal usr1
	//tabela2 e a shm do processo que vai perder um frame
	Pagina *tabela,*tabela2;
	int ramCheia, posicaoDisp;

	tabela = (Pagina *) shmat(shms[num_proc - 1], NULL, 0);
	ramCheia = verificaMemPrinc(&posicaoDisp);
	tabela[index].p = 1;
	tabela[index].r = 1;
	tabela[index].contador_acesso++;

	if(ramCheia) {

		//printf("RAM cheia - Rodar LFU \n");
		int indPagPerdida = lfu();

		tabela2 = (Pagina *) shmat(shms[memoria_principal_proc[indPagPerdida] - 1],NULL,0);

		int procPag = memoria_principal_proc[indPagPerdida];
		printf("Processo dono da pagina a ser perdida %d\n", procPag);

		int procId;
		if(procPag == 1) {
			procId = P1;
		} else if(procPag == 2) {
			procId = P2;
		} else if(procPag == 3) {
			procId = P3;
		} else if(procPag == 4) {
			procId = P4;
		}

		tabela[index].end_fisico = indPagPerdida;
		tabela2[memoria_principal[indPagPerdida]].p = 0;
		tabela2[memoria_principal[indPagPerdida]].end_fisico = -1;
		memoria_principal[tabela[index].end_fisico] = index;
		memoria_principal_proc[tabela[index].end_fisico] = tabela[index].id_processo;
		printf("endereco fisico do frame perdido = %x\n", tabela[index].end_fisico);
		shmdt(tabela2);
		kill(procId, SIGUSR2);

	} else {

		//printf("mitico num proc %d\n", num_proc);
		//printf("mitico posicao Disp %d\n", posicaoDisp);
		memoria_principal[posicaoDisp] = index;
		memoria_principal_proc[posicaoDisp] = num_proc;
		tabela[index].end_fisico = posicaoDisp;
		//		printf("Indice na tabela virtual:%d, Endereco fisico %d\n",index,tabela[index].end_fisico);

	}


	int inicio = time(NULL);
	if(tabela[index].m == 1) {
		//bit M setado, logo sleep por 2s
		while(1) {
			if((int)(time(NULL) - inicio) >= 2) {
				//printf("DORMI POR 2 SEGUNDOS\n");
				kill(pid, SIGCONT);
				break;
			}

		}
	} else {
		while(1) {
			if((int)(time(NULL) - inicio) >= 1) {
				//printf("DORMI POR 1 SEGUNDO\n");
				kill(pid, SIGCONT);
				break;
			}
		}
	}
	shmdt(tabela);

	int semAtual;
	if(num_proc == 1) {
		semAtual = semId;
	} else if(num_proc == 2) {
		semAtual = semId2;
	} else if(num_proc == 3) {
		semAtual = semId3;
	} else if(num_proc == 4) {
		semAtual = semId4;
	}
	up(semAtual);
}

int criaTabelaPaginas(int paginaTam, int id) {
	Pagina *vetorTabelaPaginas;
	int segmento;

	segmento = shmget (id, (paginaTam + 1) *  sizeof(Pagina), IPC_CREAT |  S_IRUSR | S_IWUSR);
	vetorTabelaPaginas = (Pagina *) shmat(segmento, NULL, 0);

	if(!vetorTabelaPaginas) {
		printf("Faltou memoria\n");
		exit(1);
	}

	for(int i=0;i < paginaTam; i++){
		Pagina tp = { .r_w = -1, .id_processo = -1, .contador_acesso = 0, .r = -1, .m = -1, .p = 0, .end_fisico = -1 };
		vetorTabelaPaginas[i] = tp;
	}

	return segmento;
}

void auxHandler(int pid, int i) {
	Pagina *tabela = shmat(shms[i], NULL, 0);
	alocaPag(pid,i+1, tabela[MAXTABELA].contador_acesso);
	shmdt(tabela);
	up(semSignalId);
}

void sigHandler(int signal, siginfo_t *siginfo, void *context) {

	if(signal == SIGUSR1) {
		//stop no processo requisitante
		kill(siginfo->si_pid, SIGSTOP);
		if(P1 == siginfo->si_pid) {
			auxHandler(siginfo->si_pid, 0);
		} else if(P2 == siginfo->si_pid) {
			auxHandler(siginfo->si_pid, 1);
		} else if(P3 == siginfo->si_pid) {
			auxHandler(siginfo->si_pid, 2);
		} else if(P4 == siginfo->si_pid) {
			auxHandler(siginfo->si_pid, 3);
		}
	}

}

void perdaPagHandler(int signal) {
	int processPid = getpid();
	int indice;
	if(processPid == P1) {
		indice = 0;
	} else if(processPid == P2) {
		indice = 1;
	} else if(processPid == P3) {
		indice = 2;
	} else if(processPid == P4) {
		indice = 3;
	}
	printf("Processo %d perdeu uma pagina\n", indice+1);
}

void inicializa_contadores() {

	segmento_contadores = shmget(4444444, (3) *  sizeof(int), IPC_CREAT |  S_IRUSR | S_IWUSR);
	contadores = (int *) shmat(segmento_contadores, NULL, 0);

	for(int i = 0; i < 3; i++) {
		contadores[i] = 0;
	}

}

void checa_contador_acesso() {
	Pagina *t;
	//int semAtual;
	contadores = (int *) shmat(segmento_contadores, NULL, 0);
	down(semContador);
	int total_pf = contadores[0];
	int total_modif = contadores[1];
	//printf("NUM DE ACESSOS %d\n", contadores[2]);
	if(contadores[2] > 5) {
		//printf("ENTREI NO IF PORRA\n");
		long tempo_total = time(NULL) - inicio_sim;
		for(int i = 0; i < 10; i++) {
			//printf("vou selecionar porra!! %d e o fucking i %d\n", memoria_principal_proc[i],i);
			if(memoria_principal_proc[i] == 1) {
				t = (Pagina *)shmat(shms[0], NULL, 0);
				//printf("acessei 1\n");
				//semAtual = semId;
				//down(semId);
			} else if(memoria_principal_proc[i] == 2) {
				t = (Pagina *)shmat(shms[1], NULL, 0);
				//printf("acessei 2\n");
				//down(semId2);
				//semAtual = semId2;
			} else if(memoria_principal_proc[i] == 3) {
				t = (Pagina *)shmat(shms[2], NULL, 0);
				//printf("acessei 3\n");
				//down(semId2);
				//semAtual = semId2;
			} else if(memoria_principal_proc[i] == 4) {
				t = (Pagina *)shmat(shms[3], NULL, 0);
				//printf("acessei 4\n");
				//down(semId4);
				//semAtual = semId4;
			}
			if(memoria_principal[i] != -1) {
					t[memoria_principal[i]].contador_acesso = 0;
			}
			//printf("ZEREI FIOTE\n");
			//up(semAtual);
			shmdt(t);
		}
		contadores[2] = 0;
		printf("==============================================\n");
		printf("Contador de acessos zerado\n");
		printf("*** Numero de Page Faults = %d ***\n", total_pf);
		printf("*** Numero de escritas = %d ***\n", total_modif);
		printf("*** Tempo de execucao da simulacao = %lu s ***\n", tempo_total);
		printf("==============================================\n");
	}
	//printf("trava\n");
	up(semContador);
}

int main(void){

	/* cria as 4 memorias compartilhadas para as tabelas de pagina virtual de cada processo*/

	inicio_sim = time(NULL);

	shms[0] = criaTabelaPaginas(MAXTABELA,55555);
	shms[1] = criaTabelaPaginas(MAXTABELA,66666);
	shms[2] = criaTabelaPaginas(MAXTABELA,77777);
	shms[3] = criaTabelaPaginas(MAXTABELA,88888);

	for(int i=0;i<4;i++) {
		pageFaults[i] = 0;
	}

	inicializa_contadores();

	//Inicializa os semaforos

	semId = semget(10888, 1, 0666 | IPC_CREAT);
	setSemValue(semId);

	semId2 = semget(10889, 1, 0666 | IPC_CREAT);
	setSemValue(semId2);

	semId3 = semget(10890, 1, 0666 | IPC_CREAT);
	setSemValue(semId3);

	semId4 = semget(10891, 1, 0666 | IPC_CREAT);
	setSemValue(semId4);

	semSignalId = semget(10892, 1, 0666 | IPC_CREAT);
	setSemValue(semSignalId);

	semContador = semget(10893, 1, 0666 | IPC_CREAT);
	setSemValue(semContador);

	struct sigaction act;
	memset(&act, '\0', sizeof(act));

	act.sa_sigaction = &sigHandler;
	act.sa_flags = SA_SIGINFO;

	if (sigaction(SIGUSR1, &act, NULL) == -1) {
		perror("sigaction");
	}

	signal(SIGUSR2, perdaPagHandler);

	for(int i = 0; i < 10; i++) {
		memoria_principal[i] = -1;
		memoria_principal_proc[i] = -1;
	}

	P1 = fork();
	if(P1 != 0){
		P2 = fork();

		if(P2 != 0){
			P3 = fork();

			if(P3 != 0){
				P4 = fork();

				if(P4 != 0){
					while(1) {
						checa_contador_acesso();
					}
				}
				else{
					//simulador.log
					unsigned addr4;
					char rw4;
					FILE *arq4 = fopen("simulador.log", "r");

					while( fscanf(arq4, "%x %c", &addr4, &rw4) == 2 ) {
						int indicePagina = retIndPag(addr4);
						int offsetePagina = addr4 & 0x0000FFFF;
						//checa_contador_acesso();
						trans(4, indicePagina, offsetePagina, rw4);
					}
					Pagina *tp = shmat(shms[3], NULL, 0);
					shmdt(tp);
				}
			}else{
				//compressor.log
				unsigned addr3;
				char rw3;
				FILE *arq3 = fopen("compressor.log", "r");

				while( fscanf(arq3, "%x %c", &addr3, &rw3) == 2 ) {
					int indicePagina = retIndPag(addr3);
					int offsetePagina = addr3 & 0x0000FFFF;
					//checa_contador_acesso();
					trans(3, indicePagina, offsetePagina, rw3);

				}

				Pagina *tp = shmat(shms[2], NULL, 0);
				shmdt(tp);
			}
		}else{
			//matriz.log
			unsigned addr2;
			char rw2;
			FILE *arq2 = fopen("matriz.log", "r");

			while( fscanf(arq2, "%x %c", &addr2, &rw2) == 2 ) {
				int indicePagina = retIndPag(addr2);
				int offsetePagina = addr2 & 0x0000FFFF;
				//checa_contador_acesso();
				trans(2, indicePagina, offsetePagina, rw2);
			}

		}
		Pagina *tp = shmat(shms[1], NULL, 0);
		shmdt(tp);

	}else{
		//compilador.log
		unsigned addr1;
		char rw1;
		FILE *arq1 = fopen("compilador.log", "r");
		while( fscanf(arq1, "%x %c", &addr1, &rw1) == 2 ) {
			int indicePagina = retIndPag(addr1);
			int offsetePagina = addr1 & 0x0000FFFF;
			//checa_contador_acesso();
			trans(1, indicePagina, offsetePagina, rw1);
		}
		Pagina *tp = shmat(shms[0], NULL, 0);
		shmdt(tp);
	}
	return 0;
}

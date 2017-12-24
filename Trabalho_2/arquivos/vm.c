#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <signal.h>
#include "vm.h"
#include "gm.h"
#include "semaphore.h"
#include <sys/sem.h>


void trans(int num_proc, int pag, int offset, char read_write) {

	int segmento_processo = shms[num_proc - 1];
	int semAtual;

	contadores = (int *) shmat(segmento_contadores, NULL, 0);
	// contando os acessos
	down(semContador);
	contadores[2]++;

	Pagina *tabela;
	tabela = shmat(segmento_processo, NULL, 0);

	if(num_proc == 1) {
		semAtual = semId;
	} else if(num_proc == 2) {
		semAtual = semId2;
	} else if(num_proc == 3) {
		semAtual = semId3;
	} else if(num_proc == 4) {
		semAtual = semId4;
	}

	down(semAtual);

	if(tabela[pag].p == 0) {
		int rw;
		if(read_write == 'R') {
			rw = 0;
		} else if(read_write == 'W') {
			rw = 1;
			//conta modificacao
			contadores[1]++;
		}
		// conta pageFaults
		contadores[0]++;
		up(semContador);

		Pagina paginaAcessada = { .r_w = rw, .id_processo = num_proc, .contador_acesso = 0, .r = 1, .m = rw, .p = 0, .end_fisico = -1 };
		pageFaults[num_proc - 1]++;
		Pagina paginaFinal = { .r_w = rw, .id_processo = num_proc, .contador_acesso = pag, .r = 1, .m = rw, .p = 0, .end_fisico = -1 };

		tabela[pag] = paginaAcessada;
		tabela[65536] = paginaFinal;

		down(semSignalId);
		printf("Page fault tabela do processo = %d - do frame de endereco fisico = %d - e offset = %x \n", num_proc, pag, offset);
		kill(getppid(), SIGUSR1);
		//printf("Page fault na tabela do processo: %d\n", num_proc);
		usleep(10000);

	} else {
		up(semAtual);
		printf("Pagina %d ja esta em um frame de memoria\n", pag);
		printf("Px %d, F-number+o %x %x, r_w %c \n",num_proc-1,tabela[pag].end_fisico, offset, read_write);
		up(semContador);

	}
}

#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>




int main (int argc, char *argv[]) {

	int fd[2]; //descritor dos pipes
	int retorno2;


	//funcao pipe cria um canal de comunicacao entre os processos

	if( pipe(fd) < 0 ) {
		printf("erro ao abrir canais de leitura");
		exit(-1);
	}

	if( fork() > 0 ) { // pai
		// fecha a entrada
		if ( (retorno2 = dup2(fd[0],0)) == -1 ) { /* duplicacao de stdout */
			perror("Error dup2()");
			return -3;
		}
		close(fd[1]);
		argv[0] = "grep";
		argv[1] = "e";
    	argv[2] = NULL;
		execvp(argv[0], argv);

	}
	else {
		//close(1) - já é feito pelo 
		if ( (retorno2 = dup2(fd[1],1)) == -1 ) { /* duplicacao de stdout */
			perror("Error dup2()");
			return -3;
		}
		argv[0] = "ls";
    	argv[1] = NULL;
		execvp(argv[0], argv);

	}
	 
	//close(fd[1]); 

	return 0;

}

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

	int fd; /* descritor a ser duplicado */
	int retorno; /* valor de retorno de dup */
	int retorno2; /* valor de retorno de dup2 */

	if ( (fd = open("teste.txt",O_RDWR|O_CREAT|O_TRUNC,0666)) == -1 ) {
		perror("Error open()");
		return -1;
	}

	close(0); /* fechamento da entrada stdin */

	if ( (retorno = dup(fd)) == -1 ) { /* duplicacao de stdin (menor descritor fechado) */
		perror("Error dup()");
		return -2;
	}

	if ( (retorno2 = dup2(fd,1) ) == -1) { /* duplicacao de stdout */
		perror("Error dup2()");
		return -3;
	}
	
	printf ("valor de retorno de dup(): %d \n",retorno);
	printf ("valor de retorno de dup2(): %d \n",retorno2);

	return 0;
}

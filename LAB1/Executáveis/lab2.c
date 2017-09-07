#include  <stdio.h>
#include  <string.h>
#include  <sys/types.h>
#include  <sys/wait.h>
#include <unistd.h>

int main(void){

	pid_t pidfilho, pidpai;
	int status;
	int var = 0;
	int i;

	pidfilho = fork(); //retorna o pid do filho
	pidpai = getpid(); //retorna o pid do pai

	if( pidfilho != 0 ){
		printf("\n Processo do pai iniciado \n");
		for(i=0; i<100; i++){
			var += 5;
            printf("\n PAI %d - %d \n", pidpai, var);
		}
		waitpid(-1, &status, 0);
		printf("\n Processo do pai finalizado \n");
	}
	else {
		printf("\n Processo do filho iniciado \n");
		for(i=0; i<100; i++){
			var += 10;
            printf("\nFILHO %d - %d", pidpai, var);
		}
		printf("\n Processo do filho finalizado \n");
	}
	return 0;
}

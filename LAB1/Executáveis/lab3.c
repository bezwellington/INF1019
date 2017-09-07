#include  <stdio.h>
#include  <string.h>
#include  <sys/types.h>
#include  <sys/wait.h>
#include <unistd.h>

int main(void){

	pid_t pidfilho, pidpai, pidneto;
	int status;
	int i;

	pidfilho = fork(); //retorna o pid do filho
	pidpai = getpid(); //retorna o pid do pai

	//processo do pai
	if( pidfilho != 0 ){
		printf("\n Processo do pai iniciado \n");
		for(i=0; i<100; i++){
			printf("\nPAI - numero: %d \n", i);
			sleep(1);
		}
		waitpid(-1, &status, 0);
		printf("\n Processo do pai finalizado \n");
	}
	else {
		pidneto = fork();
		//processo do filho
		if( pidneto != 0 ){
			printf("\n Processo do filho iniciado \n");
			for(i=100; i<200; i++){
				printf("\nFILHO - numero: %d \n", i);
				sleep(2);
			}
			waitpid(-1, &status, 0);
			printf("\n Processo do filho finalizado \n");
		}
		// processo do neto
		else {
			printf("\n Processo do NETO iniciado \n");
			for(i=200; i<300; i++){
				printf("\nNETO - numero: %d \n", i);
				sleep(3);
			}
			printf("\n Processo do NETO finalizado \n");
		}
	}
	return 0;
}

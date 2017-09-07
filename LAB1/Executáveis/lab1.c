#include  <stdio.h>
#include  <string.h>
#include  <sys/types.h>
#include  <sys/wait.h>
#include <unistd.h>

int main(void){
    
    //variaveis
	pid_t pidfilho, pidpai;
	int status;
	
	pidfilho = fork(); //retorna o pid do filho
	pidpai = getpid(); //retorna o pid do pai

	if( pidfilho != 0 ){
		printf("\n Processo pai iniciado \n");
		printf("\n pid do pai %d \n", pidpai);
		printf("\n pid do filho %d \n", pidfilho);
		waitpid(-1, &status, 0);
		printf("\n Processo do pai finalizado \n");
	}
	else {
		printf("\n Processo filho iniciado \n");
		printf("\n pid do pai %d \n", pidpai);
		printf("\n pid do filho %d \n", pidfilho);
		printf("\n Processo do filho finalizado \n");
	}
	return 0;
}

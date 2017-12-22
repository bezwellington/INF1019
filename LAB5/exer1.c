#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void){
    int pidPai, pidFilho, pidNeto, status, i, aux;

  
    pidFilho = fork();

    if( pidFilho == 0 ){ //processo filho1
        printf("Processo FILHO 1 iniciado\n");
        while(1);

    }else {
        pidNeto = fork();
        if(pidNeto == 0){ //processo filho2
            printf("Processo FILHO 2 iniciado\n");
            while(1);
            //waitpid(-1, &status, 0);

        }else { //Processo PAI
            printf("Processo PAI iniciado\n");
            //waitpid(-1, &status, 0);
            
            //stop nos processos filho
            kill(pidNeto, SIGSTOP);
            kill(pidFilho, SIGSTOP);

            printf("processo filho %d\n", pidFilho);
            printf("processo neto %d\n", pidNeto);

            for(i=0 ; i<10 ; i++){
                aux = i%2;
                if(aux==0){
                    kill(pidFilho, SIGCONT);
                    printf("processo filho");
                    sleep(1);
                    kill(pidFilho, SIGSTOP);
                }else {
                    kill(pidNeto, SIGCONT);
                    printf("processo neto");
                    sleep(1);
                    kill(pidNeto, SIGSTOP);
                }

            }

            printf("Processo PAI finalizado\n");
        }
    }

    return 0;

}
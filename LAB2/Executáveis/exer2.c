#include  <stdio.h>
#include  <string.h>
#include  <sys/types.h>
#include  <sys/wait.h>
#include <unistd.h>




void childFunction(int pos, int matriz1[3][3], int matriz2[3][3]){
    printf("Child : %d\n", getpid());
	int j;
	int aux[3] = {0, 0, 0};
	for(j=0; j<3 ; j++){
		aux[j] = matriz1[pos][j] + matriz2[pos][j];
	}
	printf("linha %d: %d %d %d \n  ------ \n", pos, aux[0], aux[1], aux[2]);
	sleep(pos);
}

int main(void){

    int matriz1[3][3] = {{1,2,3}, {4,5,6}, {7,8,9}};
    int matriz2[3][3] = {{1,2,3}, {4,5,6}, {7,8,9}};

    int childLimit = 3; /* numero de processos filho */
    int childrenPids[childLimit]; /* vetor que armazena PIDs dos processos filho */
    int currentPid, i;

    for(i=0; i<childLimit; i++){
        switch(currentPid = fork()){
            case 0:
                /* chama a função do processo filho */
                childFunction(i, matriz1, matriz2);
                /* fim do processo filho */
                return 0;
            case -1:
                printf("Error when forking\n");
                break;
            default:
                childrenPids[i] = currentPid; // store current child pid
                break;
        }

    }

    printf("Processo Pai : %d filhos criados\n", i);
    
    waitpid(-1, NULL, 0);
}


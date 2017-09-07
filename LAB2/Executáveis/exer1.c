#include  <stdio.h>
#include  <string.h>
#include  <sys/types.h>
#include  <sys/wait.h>
#include <unistd.h>


void busca(int ini, int fim, int *vet, int elem){
	int i;
	for(i=ini; i<=fim; i++){
        if(vet[i] == elem){
			printf("achei no indice %d\n", i);
            printf("-------------\n");
            return;
        }
	}
	printf("nao achei\n");
    printf("-------------\n");
}

void childFunction(int i, int *vet, int elem){
    printf("Child : %d\n", getpid());
    // do stuff
	if (i == 0){
		busca(0, 2, vet, elem);
	}
	else {
		if(i == 1){
			busca(3, 5, vet, elem);
		}
		else {
			if (i == 2){
				busca(6, 8, vet, elem);
			}
			else {
				busca(9, 10, vet, elem);
			}
		}
	}
	

}

int main(){


    int vetor[10] = {3,5,7,2,8,4,1,2,6,5};
    int elem = 1;
    int childLimit = 4; // number of children wanted
    int childrenPids[childLimit]; // array to store children's PIDs if needed
    int currentPid, i;

    for(i=0; i<childLimit; i++){
        switch(currentPid = fork()){
            case 0:
                // in the child
                childFunction(i, vetor, elem);
                // exit the child normally and prevent the child
                // from iterating again
                return 0;
            case -1:
                printf("Error when forking\n");
                break;
            default:
                // in the father
                childrenPids[i] = currentPid; // store current child pid
                break;
        }

    }

    //wait for all child created to die
    waitpid(-1, NULL, 0);
}


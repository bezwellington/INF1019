#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

void transformaTransposta(int *m1, int *m2, int i){
    int cont;
    for(cont=0; cont<4; cont++) {
        m2[i+cont*4] = m1[4*i+cont];
    }
}

int main (int argc, char *argv[]) {
    
    int segmento1, segmento2, *m1, *m2, id, pid, status, i;
    
    // aloca espaço de memória compartilhada
    segmento1 = shmget (IPC_PRIVATE, 16 * sizeof (int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    segmento2 = shmget (IPC_PRIVATE, 16 * sizeof (int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if(segmento1 == -1 || segmento2 == -1 ){
        printf("Erro ao alocar espaço para segmento 1 ou 2");
        exit(1);
    }
    
    // o ponteiro é associado a memória compartilhada
    m1 = (int *) shmat (segmento1, 0, 0);
    if(segmento1 == -1){
        printf("Erro 1");
        exit(1);
    }
    
    m2 = (int *) shmat (segmento2, 0, 0);
    if(segmento2 == -1){
        printf("Erro 2");
        exit(1);
    }
    
    //Pede para o usuário preencher a matriz
    for(i=0;i<4;i++){
        printf("Digite os valores da linha %d \n", i+1);
        scanf("%d %d %d %d", &m1[(4*i)], &m1[4*i+1], &m1[4*i+2], &m1[4*i+3]);
    }
    
    for(i=0;i<4;i++){
        
        if ((id = fork()) < 0) {
            printf("Erro na criação do novo processo");
            exit (-2);
        }
        else if (id == 0) {
            
            //printf("Processo filho = %d\n", i);
            transformaTransposta(m1, m2, i);
            exit(-1); // finaliza o processo filho
        }
        else {
            waitpid (-1, 0, 0);
        }
    }
    printf("MATRIZ ---- TRANSPOSTA\n");
    //processo pai imprime as duas matrizes
    for(i=0;i<4;i++) {
        printf("%d %d %d %d ", m1[4*i], m1[4*i+1], m1[4*i+2], m1[4*i+3]);
        printf("---- %d %d %d %d \n", m2[i*4], m2[4*i+1], m2[4*i+2], m2[4*i+3] );
    }
    
    // libera a memória compartilhada do processo
    shmdt (m1);
    
    // libera a memória compartilhada
    shmctl (segmento1, IPC_RMID, 0);
    
    // libera a memória compartilhada do processo
    shmdt (m2);
    
    // libera a memória compartilhada
    shmctl (segmento2, IPC_RMID, 0);
    
    
    return 0;
}

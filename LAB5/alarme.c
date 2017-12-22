#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

void somaLinhas(int ini, int fim, int *p1, int *p2, int *p3){
    int j;
    for(j=ini;j<fim;j++){
        p3[j] = p1[j] + p2[j];
    }
}

int main (int argc, char *argv[]) {
    
    int segmento1, segmento2, segmento3, *p1, *p2, *p3, id, pid, status, i;
    
    // aloca espaço de memória compartilhada
    segmento1 = shmget (IPC_PRIVATE, 9 * sizeof (int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    segmento2 = shmget (IPC_PRIVATE, 9 * sizeof (int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    segmento3 = shmget (IPC_PRIVATE, 9 * sizeof (int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if(segmento1 == -1 || segmento2 == -1 || segmento3 == -1){
        printf("Erro ao alocar espaço");
        exit(1);
    }
    
    // associa a memória compartilhada ao processo
    p1 = (int *) shmat (segmento1, 0, 0);
    if(segmento1 == -1){
        printf("Erro 1");
        exit(1);
    }
    p2 = (int *) shmat (segmento2, 0, 0);
    if(segmento2 == -1){
        printf("Erro 2");
        exit(1);
    }
    p3 = (int *) shmat (segmento3, 0, 0);
    if(segmento3 == -1){
        printf("Erro 3");
        exit(1);
    }
    
    //vetores preenchidos na área de memória alocada
    for(i=0;i<9;i++){
        p1[i] = i;
        p2[i] = i;
        p3[i] = 0;
    }
    
    for(i=0;i<3;i++){
        
        if ((id = fork()) < 0) {
            printf("Erro na criação do novo processo");
            exit (-2);
        }
        else if (id == 0) {
        
            //processo filho
            somaLinhas(i*3,(i+1)*3, p1, p2, p3);
            sleep(1);
            return 0; // finaliza o processo filho
        }
        else {
            pid = wait (&status);
        }
    }
    
    printf("MATRIZ\n");
    //processo pai imprime as duas matrizes
    for(i=0;i<3;i++) {
        printf("%d %d %d \n", p3[3*i], p3[3*i+1], p3[3*i+2] );
    }
    
    // libera a memória compartilhada do processo
    shmdt (p1);
    shmdt (p2);
    shmdt (p3);
    
    // libera a memória compartilhada
    shmctl (segmento1, IPC_RMID, 0);
    shmctl (segmento2, IPC_RMID, 0);
    shmctl (segmento3, IPC_RMID, 0);
    
    
    return 0; 
}

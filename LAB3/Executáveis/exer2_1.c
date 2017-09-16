#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    int seg;
    char * msg;
    
    // aloca espaço de memória compartilhada
    seg = shmget(8180, 100*sizeof (char), IPC_CREAT | S_IRUSR |  S_IWUSR);
    msg = (char*) shmat(seg, 0, 0);
    
    printf("Mensagem do dia: \n");
    scanf(" %100[^\n]", msg);
    
    return 0;
}

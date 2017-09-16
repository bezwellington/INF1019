#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int seg;
    char * msg;
    
    // aloca espaço de memória compartilhada
    seg = shmget (8180, 100 * sizeof(char), IPC_EXCL | S_IRUSR |  S_IWUSR);
    msg = (char *) shmat(seg, 0, 0);
    
    printf("A mensagem do dia é: %s\n", msg);
    
    shmdt(msg);
    shmctl(seg, IPC_RMID, 0);
    
    return 0;
}

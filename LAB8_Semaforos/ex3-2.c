#include <sys/sem.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> 
#include <sys/shm.h>
#include <sys/stat.h>
#define BUFFER_SIZE 16
#define MY_IPC 1212

// inicializa o valor do semáforo
int setSemValue(int semId);

// remove o semáforo
void delSemValue(int semId);

// operação P
int semaforoP(int semId);

//operação V
int semaforoV(int semId);

int main(int argc, char const *argv[]) {
    int semId;
    int i;
    int segmento;
    int * var;

    /* Aloca a memória compartilhada */
    segmento = shmget(MY_IPC, sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR );
    if (segmento == -1) {
        puts ("Erro na criação de segmento de memoria compartilhada."); 
        exit(1);
    }

    /* Associa a memória compartilhada ao processo */
    var = (int *) shmat(segmento, 0, 0);
    
    //*var = 0;

    /* cria um novo semáforo ou obtem a chave do que já existe */
    semId = semget (8765, 1, 0666 | IPC_CREAT);
    
    /* inicializa o valor do semáforo */
    setSemValue(semId);

    while (1) {
        semaforoP(semId);
        *var += 5;
        printf("%d\n", *var);
        sleep(rand()%3);    
        semaforoV(semId);
        sleep(rand()%2);
    }

    shmdt (var); 
    shmctl (segmento, IPC_RMID, 0); 

    return 0;
}

int setSemValue( int semId) {
    union semun semUnion;
    semUnion.val = 1;
    return semctl(semId, 0, SETVAL, semUnion);
}

void delSemValue(int semId) {
    union semun semUnion;
    semctl(semId, 0, IPC_RMID, semUnion);
}

int semaforoP(int semId) {
    struct sembuf semB;

    semB.sem_num = 0;
    semB.sem_op = -1;
    semB.sem_flg = SEM_UNDO;

    semop(semId, &semB, 1);

    return 0;
}

int semaforoV(int semId) {
    struct sembuf semB;

    semB.sem_num = 0;
    semB.sem_op = 1;
    semB.sem_flg = SEM_UNDO;

    semop(semId, &semB, 1);

    return 0;
}

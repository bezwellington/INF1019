#include <sys/sem.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define TRUE 1

// inicializa o valor do semáforo
int setSemValue(int semId);

// remove o semáforo
void delSemValue(int semId);

// operação P
int semaforoP(int semId);

//operação V
int semaforoV(int semId);

int main(int argc, char const *argv[]) {
    int qtd = 0, i;
    int segmento, semId;
    char * p, c;
    int pid = 0;

    // Aloca a memória compartilhada
    segmento = shmget(IPC_PRIVATE, sizeof(char) * 500, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    semId = semget (8752, 1, 0666 | IPC_CREAT);
    setSemValue(semId);

    // Associa a memória compartilhada ao processo
    p = (char *) shmat(segmento, 0,0);

    pid = fork();

    if (pid < 0) /* Erro ao criar novo processo */
    {
        printf("Erro ao criar novo processo\n");
        exit(1);
    }
    else if (pid == 0) /* Processo filho */
    {
        printf("Entrando no processo filho\n");
        while (TRUE)
        {
            semaforoP(semId); /* Entra na area critica */
            printf("Executando o processo filho (Coletar mensagem)\n");
            printf("Escreva a mensagem: \n");
            scanf("%s", p);
            printf("Fim da execucao do processo filho (incrementando o semaforo...)\n");
            semaforoV(semId); /* Sai da area critica */
        }
    }
    else { // Processo pai
        printf("Processo pai\n");

        while (TRUE) {
            sleep(1);

            semaforoP(semId);

            printf("Executando o processo pai (Printar mensagem)\n");
            printf("%s\n",p);
            printf("Fim da execucao do processo filho (incrementando o semaforo...)\n");

            semaforoV(semId); /* Sai da area critica */
        }
    }

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

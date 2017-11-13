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

int main(int argc, char const * argv[]) {
    int semId, pid, segmento, cont = 0;
    char * mensagem, leitura, lixo;

    // Aloca a memória compartilhada
    segmento = shmget(IPC_PRIVATE, sizeof(char) * 17, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR );
    semId = semget (8752, 1, 0666 | IPC_CREAT);
    setSemValue(semId);

    // Associa a memória compartilhada ao processo
    mensagem = (char *) shmat(segmento, 0, 0);

    // Fazer consumidor entender quando o buffer esta cheio
    mensagem[16] = '\0';
    mensagem[15] = '\0';

    if((pid = fork()) == 0) { //Produtor
        while(TRUE) {
            semaforoP(semId);

            printf("Digite um caracter: ");
            scanf("%c%c", &leitura, &lixo);
            mensagem[cont] = leitura;
            cont++;

            if (cont == 16) cont = 0;

            semaforoV(semId);
            sleep(1);
        }
    }
    else { //Consumidor
        while(TRUE) {
            semaforoP(semId);
            fflush(stdout);

            if(mensagem[15] != '\0') {
                printf("\n-------------------\n");
                printf("%s\n", mensagem);
                printf("-------------------\n\n");
                mensagem[15] = '\0';
            }

            semaforoV(semId);
            sleep(2);
        }
    }

    shmdt(mensagem);
    shmctl(segmento, IPC_RMID, 0);
    delSemValue(semId);

    return 0;
}

int setSemValue(int semId) {
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

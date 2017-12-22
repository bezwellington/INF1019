#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 2

static int vet[10] = {0,1,2,3,4,5,6,7,8,9};


void* incrementa(void* t0);
void* decrementa(void* t1);

int main()
{
    int t0 = 1, t1 = 2, t, i;
    pthread_t threads[NUM_THREADS];
    printf("Inicio \n");


    pthread_create(&threads[0], NULL, incrementa, (void *)t0);
    pthread_create(&threads[1], NULL, decrementa, (void *)t1);


    for(t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    for(i=0 ; i<10 ; i++){
        printf("%d \n", vet[i]);
    }

    printf("\nAcabou!!\n");

    pthread_exit(NULL);

    return 0;
}

void* incrementa(void* t0){
    int i;
    for(i=0 ; i<10 ; i++){
        printf("thread %d\n", (int)t0);
        if(i%2 != 0){           //impar
            vet[i] -= 1;
        }
        sleep(1);
    }
    pthread_exit(NULL);
}

void* decrementa(void* t1){
    int i;
    for(i=0 ; i<10 ; i++){
        printf("thread %d\n", (int)t1);
        if(i%2 == 0){           //par
            vet[i] += 1;
        }
        sleep(1);
    }
    pthread_exit(NULL);
}




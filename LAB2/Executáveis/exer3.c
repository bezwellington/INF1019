#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int main(void){
    
    char *argv[100];
    char entrada[100];
    char raiz[100];
    
    int status;
    int index = 0;
    
    while(1){ /* loop da "shell" */
        
        strcpy(entrada, ""); /* limpa entrada */
        strcpy(raiz, ""); /* limpa a raiz */
        
        printf("Shell >> ");
        scanf("%[^\n]%*c", entrada); /* pega a entrada */
        
        if ( strcmp(entrada,"exit") == 0 ) { /* aborta o programa */
            return 0;
        }
        
        char *token;
        token = strtok(entrada," \""); /* pega a primeira parte da string */
        
        while(token != NULL) { /* preenche as posiçoes de argv */
            argv[index] = token;
            index = index + 1;
            token = strtok(NULL," \"");
        }
        
        argv[index] = NULL; /* NULL no final do vetor argv */
                        /* argv precisa ter NULL no final */
        
        
        if (fork() != 0){
            
            printf("\nProcesso do pai iniciado \n");
            waitpid(-1,&status,0);
            index = 0; /* reset do indice de argumentos */
            printf("Processo do pai finalizado \n");
        }
        else {
            printf("Processo do filho iniciado \n");
            printf("Raiz: %s\n", raiz);
            execvp(argv[0], argv); /* recebe um comando e os parametros desse comando  */
                                    /* argv[0] -> comando */
            printf("Processo do filho finalizado \n");
        }
        
    }
    
    return 0;
}

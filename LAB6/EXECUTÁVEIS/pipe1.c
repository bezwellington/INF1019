#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char *argv[]) {

	int fd[2]; //descritor dos pipes
	int nDadosTx, nDadosRx; // quantidade de dados transmitidos/recebidos
	const char textoTX[] = "uma mensagem"; //mensagem enviada pelo filho
	char textoRX[sizeof(textoTX)];

	//funcao pipe cria um canal de comunicacao entre os processos

	if( pipe(fd) < 0 ) {
		printf("erro ao abrir canais de leitura");
		exit(-1);
	}

	if( fork() > 0 ) { // pai

		nDadosRx = read(fd[0], textoRX, sizeof textoRX); /* lê do filho */
		printf("%d dados lidos pelo pai do filho: %s\n", nDadosRx, textoRX);

	}
	else {
		//dados escritos pelo filho
		nDadosTx = write(fd[1], textoTX, strlen(textoTX)+1);
		printf("%d dados escritos pelo filho \n", nDadosTx);

	}
	close(fd[0]); 
	close(fd[1]); 

	return 0;
}
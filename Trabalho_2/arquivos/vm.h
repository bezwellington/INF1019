

//A Função trans deve imprimir o endereço físico do processo quando ele está com um PageFrame e quando ele não está deve emitir o sinal SIGUSR1 para o GM avisando que falta uma página.

void trans(int num_proc, int pag, int offset, char read_write);

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/shm.h>

int main (int argc, char *argv[]) {
	int i, j;
	int p_pid = getppid();
	for(i=1; i < argc; i++) {
		for(j=0; j < atoi(argv[i]); j++) {
			printf("PID: %d\n", getpid());
			fflush(stdout);
			usleep(1000000);
		}
		// printf("Filho %d acabou a rajada de %s\n", getpid(), argv[i] );
		if(i != argc-1) {
			kill(p_pid, SIGUSR1);
			usleep(3000000);
			kill(p_pid, SIGUSR2);
		}
	}
	return 0;
}

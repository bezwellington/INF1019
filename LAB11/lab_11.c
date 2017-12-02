#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    char *str = "P";
    
    mkdir("./sc", 0700);
    mkdir("./sc/a", 0700);
    mkdir("./sc/b", 0700);
    mkdir("./sc/c", 0700);
    
    int fd2 = open("./sc/a/arq.txt", O_CREAT | O_WRONLY, 0777);
    write(fd2, str, sizeof(char));
    int fd3 = open("./sc/b/arq.txt", O_CREAT | O_WRONLY, 0777);
    write(fd3, str, sizeof(char));
    int fd4 = open("./sc/c/arq.txt", O_CREAT | O_WRONLY, 0777);
    write(fd4, str, sizeof(char));
    
    close(fd2);
    close(fd3);
    close(fd4);
    
    return 0;
}

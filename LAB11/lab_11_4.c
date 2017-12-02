#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    struct dirent **namelist;
    int n;
    
    n = scandir(".", &namelist, NULL, alphasort);
    
    if (n < 0) {
        perror("scandir");
        
    } else {
        while (n--) {
            if(strcmp("arq.txt", namelist[n]->d_name) == 0) {
                char string[1] = "P";
                char read_file[1];
                int fd2 = open("./sc/a/arq.txt", O_RDONLY, 0777);
                int fd3 = open("./sc/a/arq.txt", O_WRONLY, 0777);
                lseek(fd3, 1, SEEK_SET);
                write(fd3, string, sizeof(char));
                read(fd2, read_file, 2);
                printf("%s\n",read_file);
                exit(1);
                
            }
            
            free(namelist[n]);
        }
        
        puts("Aquivo não encontrado!");
        
        free(namelist);
    }
}

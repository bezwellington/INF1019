#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
  char mode[] = "0777";
  char buf[100] = "./sc/a/arq.txt";
  int i;

  i = strtol(mode, 0, 8);

  if (chmod (buf,i) < 0) {
    puts("Erro!");
    exit(1);
  }

  return(0);
}

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

int main(void) {
  struct stat fileAt;

  if (stat("./sc/a/arq.txt", &fileAt) < 0) {
    printf("File Error Message = %s\n", strerror(errno));

  } else {
    printf( "Permi: %d\n", fileAt.st_mode );
    printf( "Size: %lld\n", fileAt.st_size );
    printf( "Access Time: %ld\n", fileAt.st_atime );
    printf( "Device: %d\n", fileAt.st_uid );
    printf( "Permi Changed: %ld\n", fileAt.st_ctime );

  }

  return 0;
}




Under gdb, I get:

Program received signal SIGSEGV, Segmentation fault.
0x00000000004007a8 in arguments (str=0x7fffffffe010 "", args=0x7fffffffe000) at u.c:20
20      **args=strtok(s," ");

i.e, you use a pointer that points to a pointer that points to nothing usable: this is undefined behaviour, and it may be why you get strange things like ""Undefined symbol 'printf' version ABC.", or segmentation fault under gdb.

I suggest you rewrite the fonction arguments() with less indirection levels (and a better use of strtok() =), but for now this code may do what you want:

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX 32

int arguments(char str[],char **args)
{
    char *s;
    int num;
    int i;
    s=malloc(strlen(str)+1);
    strcpy(s,str);
    if(strtok(s," ")==NULL)
        num=0;
    else {
        num=1;
        while(strtok(NULL," ")!=NULL)
            num++;
    }
    strcpy(s,str);
    //  **args=strtok(s," ");
    *args=strtok(s," ");
    if(num)
    {
        for(i=1;i<num;i++)
            //         *((*args)+i)=strtok(NULL," ");
            args[i] = strtok(NULL," ");
    }
    return num;
    // note. 's' is not free()'d ..
}

int main()
{
    char buffer[256];
    char *arg[MAX];
    int args;
    int i;
    
    while(1)
    {
        gets(buffer);
        if(!strcmp(buffer,"STOP"))
            break;
        args=arguments(buffer, arg);
        printf("%d\n",args);
        for(i=0;i<args;i++)
            printf("%s\n",arg[i]);
        
    }
    return 0;
}


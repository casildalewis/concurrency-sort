#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

/**
 * Error action
 * 
 */
void error(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

int main(int argc, char *argv[]){
// wrong invokation
    if(argc!=3){
        error();
        exit(1);
    }

    const char *input = argv[1];
    const char *output = argv[2];

    return 0;
}
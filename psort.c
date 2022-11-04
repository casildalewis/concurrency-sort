#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef struct rec {
    int key;
    int value[24]; // 96 bytes
} rec_t;

void* sort(rec_t records[]) {
    // merge sort

    return ;
}

int main(int argc, char *argv[]){
// wrong invokation
    if(argc!=3){
        exit(1);
    }

    const char *input = argv[1];
    const char *output = argv[2];
    int nprocs = get_nprocs();
    pthread_t cid[nprocs];
    rec_t *records[];
    // get file, divide into chunks;
    for (int i = 0; i < nprocs; i++)
        pthread_create(&cid[i], NULL, sort, &records[i]);
    
    return 0;
}
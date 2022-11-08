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
    char *value; // 100 bytes
} rec_t;



int main(int argc, char *argv[]){
// wrong invokation
    if(argc!=3){
        printf("Wrong number of args\n");
        exit(1);
    }

    const char *input = argv[1];
    //const char *output = argv[2];

    int fd = open(input, O_RDONLY);
    if(fd < 0){
        printf("\n\"%s \" could not open\n", input);
        exit(1);
    }

    struct stat statbuf;
    int err = fstat(fd, &statbuf);
    if(err < 0){
        printf("\n\"%s \" could not open\n", input);
        exit(1);
    }

    char *ptr = mmap(NULL,statbuf.st_size,PROT_READ,MAP_SHARED,fd,0);
    if(ptr == MAP_FAILED){
        printf("Mapping Failed\n");
        return 1;
    }

    // printf("file size (bytes): %li\n", statbuf.st_size);

    close(fd);

    int nthreads = get_nprocs();
    // printf("nthreads: %i\n", nthreads);

    pthread_t threads[nthreads];

    int arrsze = statbuf.st_size/nthreads;
    rec_t arr [arrsze];
    char *r = ptr;
    for(int i=0; i<nthreads-1; i++){
        int idx = 0;
        for (; r < ptr + arrsze * 100, r<ptr+statbuf.st_size*100; r += 100) {
            arr[idx].key = *(int *)r;
            arr[idx].value = r;
            idx++;
        }

        pthread_create(&threads[i], NULL, mergeSort, )
    }


    err = munmap(ptr, statbuf.st_size);
    if(err != 0){
        printf("UnMapping Failed\n");
        return 1;
    }
    
    return 0;
}
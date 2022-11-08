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


// Merges two subarrays of arr[].
// First subarray is arr[l..m]
// Second subarray is arr[m+1..r]
void merge(rec_t records[], int left, int mid, int right)
{
    int i, j, k;
    int size_left = mid - left + 1;
    int size_right = right - mid;
  
    rec_t L[size_left], R[size_right];
  
    for (i = 0; i < size_left; i++)
        L[i] = records[left + i];
    for (j = 0; j < size_right; j++)
        R[j] = records[mid + 1 + j];
  
    // Merge the temp arrays back 
    // into records[l..r]
    // Initial index of first subarray
    i = 0; 
  
    // Initial index of second subarray
    j = 0; 
  
    // Initial index of merged subarray
    k = left; 
    while (i < size_left && j < size_right) 
    {
        if (L[i].key <= R[j].key) 
        {
            records[k] = L[i];
            i++;
        }
        else 
        {
            records[k] = R[j];
            j++;
        }
        k++;
    }
  
    // Copy the remaining elements 
    // of L[], if there are any
    while (i < size_left) {
        records[k] = L[i];
        i++;
        k++;
    }
  
    // Copy the remaining elements of 
    // R[], if there are any 
    while (j < size_right) 
    {
        records[k] = R[j];
        j++;
        k++;
    }
}

void *mergeSort(void *record){
    rec_t *records = (rec_t *)record;
    int r_index = sizeof(records)/sizeof(records[0]);

    mergeSortHelper(records, 0, r_index);
}
  
void mergeSortHelper (rec_t records[], int l_index, int r_index)
{
    if (l_index < r_index) 
    {
        int mid_index = (l_index + r_index) / 2;
  
        // Sort first and second halves
        mergeSortHelper(records, l_index, mid_index);
        mergeSortHelper(records, mid_index + 1, r_index);
  
        merge(records, l_index, mid_index, r_index);
    }
}

int main(int argc, char *argv[]){
// wrong invokation
    if(argc!=3){
        printf("Wrong number of args\n");
        exit(1);
    }

    // input/output files
    const char *input = argv[1];
    //const char *output = argv[2];

    // open input file
    int fd = open(input, O_RDONLY);
    if(fd < 0){
        printf("\n\"%s \" could not open\n", input);
        exit(1);
    }

    // store input data in buffer
    struct stat statbuf;
    int err = fstat(fd, &statbuf);
    if(err < 0){
        printf("\n\"%s \" could not open\n", input);
        exit(1);
    }

    // map input data to address space
    char *ptr = mmap(NULL,statbuf.st_size,PROT_READ,MAP_SHARED,fd,0);
    if(ptr == MAP_FAILED){
        printf("Mapping Failed\n");
        return 1;
    }

    // printf("file size (bytes): %li\n", statbuf.st_size);

    // close input file
    close(fd);

    // number of threads
    int nthreads = get_nprocs();
    // printf("nthreads: %i\n", nthreads);
    // thread array
    pthread_t threads[nthreads];

    // array sent to each thread
    int arrsze = statbuf.st_size/nthreads;
    rec_t arr [arrsze];

    // iterate through input data
    char *r = ptr;
    // for each thread,
    for(int i=0; i<nthreads-1; i++){
        int idx = 0;
        // populating input array
        for (; r < ptr + arrsze * 100, r<ptr+statbuf.st_size*100; r += 100) {

            // finding keys
            arr[idx].key = *(int *)r;

            // finding values
            int value_idx = 0;
            for(char *v = r+4; v<r+100; v+=4, value_idx++){
                arr[idx].value[value_idx] = *(int *)v;
            }
            idx++;
        }

        // creating threads
        pthread_create(&threads[i], NULL, mergeSort, &arr);
    }

    // unmap input data to address space
    err = munmap(ptr, statbuf.st_size);
    if(err != 0){
        printf("UnMapping Failed\n");
        return 1;
    }
    
    return 0;
}
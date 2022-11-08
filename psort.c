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


// Merges two subarrays of arr[].
// First subarray is arr[l..m]
// Second subarray is arr[m+1..r]
void merge(rec_t records[], int left, 
           int mid, int right)
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
    while (i < left_size && j < right_size) 
    {
        if (L[i]->key <= R[j]->key) 
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
    while (i < n1) {
        records[k] = L[i];
        i++;
        k++;
    }
  
    // Copy the remaining elements of 
    // R[], if there are any 
    while (j < n2) 
    {
        records[k] = R[j];
        j++;
        k++;
    }
}
  
void mergeSort (rec_t records[], 
               int l_index, int r_index)
{
    if (l_index < r_index) 
    {
        int mid_index = (l_index + r_index) / 2;
  
        // Sort first and second halves
        mergeSort(records, l_index, mid_index);
        mergeSort(records, mid_index + 1, r_index);
  
        merge(records, l_index, mid_index, r_index);
    }
}

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
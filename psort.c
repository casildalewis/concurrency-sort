#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/mman.h>

// TODO MS r_index-> r_index-1
typedef struct rec {
    int key;
    int value[24]; // 96 bytes
} rec_t;

int nperthread, nlastthread, nthreads, nrecords;
rec_t *records;

// Merges two subarrays of r[].
// First subarray is r[l..m]
// Second subarray is r[m+1..r]
void merge(int left, int mid, int right)
{
    int i, j, k;
    int size_left = mid - left + 1;
    int size_right = right - mid;
  
    rec_t *L = (rec_t *) malloc(size_left*sizeof(rec_t)); 
    rec_t *R = (rec_t *) malloc(size_right*sizeof(rec_t)); ;
  
    for (i = 0; i < size_left; i++)
        // L[i] = records[left + i];
        memcpy(&L[i], &(records[left + i]), sizeof(rec_t));
    for (j = 0; j < size_right; j++)
        // R[j] = records[mid + 1 + j];
        memcpy(&R[j], &(records[mid + 1 + j]), sizeof(rec_t));
  
    // Merge the temp arrays back into records[l..r]

    // Initial index of first subarray
    i = 0; 
  
    // Initial index of second subarray
    j = 0; 
  
    // Initial index of merged subarray
    k = left; 
    while (i < size_left && j < size_right) {
        if (L[i].key < R[j].key) {
            records[k] = L[i];
            i++;
        }
        else if (L[i].key > R[j].key){
            records[k] = R[j];
            j++;
        } else{
            records[k] = L[i];
            i++;

            k++;

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
    while (j < size_right) {
        records[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}
  
void mergeSort (int l_index, int r_index){
    if (l_index < r_index) {
        int mid_index = l_index + (r_index - l_index) / 2;
  
        // Sort first and second halves
        mergeSort(l_index, mid_index);
        mergeSort(mid_index + 1, r_index);
  
        merge(l_index, mid_index, r_index);
    }
}

void * mergeSortHelper(void *arg){
    long tnum = (long)arg;                    // thread number
    int l_index = tnum*nperthread;
    int r_index = (tnum+1)*nperthread - 1;

    // if last thread, add leftover records to it
    if(tnum==nthreads-1){
        r_index+=nlastthread;
    }

    //merge sort only a given subset of the array
    mergeSort(l_index, r_index);

    return NULL;
}

void mergeSortedThreads(int ntosort, int nsorted){
    
    // pairwise sorting for the subsets of the array
    for(int i = 0; i < ntosort; i = i + 2) {
        int l_index = i * (nperthread * nsorted);              // start of subset 1
        int r_index = ((i + 2) * nperthread * nsorted) - 1;   // end of subset 2
        int mid_index = l_index + (nperthread * nsorted) - 1;

        // if subset 2 is the last one and smaller than the others
        if (r_index >= nrecords) {
            // correct r_index
            r_index = nrecords - 1;
        }

        merge(l_index, mid_index, r_index);
    }

    if (ntosort / 2 >= 1) {
        mergeSortedThreads(ntosort / 2, nsorted * 2);
    }
}

int main(int argc, char *argv[]){
// wrong invokation
    if(argc!=3){
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    // input/output files
    const char *input = argv[1];
    const char *output = argv[2];

    // open input file
    int ifd = open(input, O_RDONLY);
    if(ifd < 0){
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    // store input data in buffer
    struct stat statbuf;
    int err = fstat(ifd, &statbuf);
    if(err < 0){
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    // map input data to address space
    char *ptr = mmap(NULL,statbuf.st_size,PROT_READ,MAP_SHARED,ifd,0);
    if(ptr == MAP_FAILED){
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    // close input file
    close(ifd);

    // number of records in input file
    nrecords = statbuf.st_size/100;

    // populate input array and send address to global pointer
    rec_t *inputdata = (rec_t *)malloc(statbuf.st_size);
    int i = 0;
    for (char *r=ptr; r<(ptr + nrecords*100); r+=100, i++) {
        // finding key
        memcpy(&(inputdata[i].key), (int *)r, 4);
        // finding values
        int value_idx = 0;
        for(char *v = r+4; v<r+100; v+=4, value_idx++){
            memcpy(&(inputdata[i].value[value_idx]), (int *)v, 4);
        }
    }
    records = inputdata;

    // unmap input data from address space
    err = munmap(ptr, statbuf.st_size);
    if(err != 0){
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    // number of threads
    nthreads = get_nprocs();
    // thread array
    pthread_t threads[nthreads];

    // number of records sent to each thread
    nperthread = nrecords/nthreads;
    nlastthread = nrecords%nthreads;

    // create each thread
    for(long i=0; i<nthreads; i++){
        pthread_create(&threads[i], NULL, mergeSortHelper, (void *)i);
    }
   
    // wait for them to be over
    for (int i = 0; i < nthreads; i++) {       
        pthread_join(threads[i], NULL);
    }
    
    // merge their sorted subsections
    mergeSortedThreads(nthreads, 1);

    // open output file
    FILE *fd = fopen(output, "w");
    if(!fd){
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    // write to output file
    for(int i=0; i<nrecords; i++){
        fwrite(&(records[i].key), 4, 1, fd);
        fwrite(records[i].value, 4, 24, fd);
    }

    // close output file
    fclose(fd);

    free(inputdata);

    return 0;
}

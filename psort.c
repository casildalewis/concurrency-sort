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
rec_t (*records)[];

// Merges two subarrays of r[].
// First subarray is r[l..m]
// Second subarray is r[m+1..r]
void merge(int left, int mid, int right)
{
    int i, j, k;
    int size_left = mid - left + 1;
    int size_right = right - mid;
  
    rec_t L[size_left], R[size_right];
  
    for (i = 0; i < size_left; i++)
        L[i] = (*records)[left + i];
    for (j = 0; j < size_right; j++)
        R[j] = (*records)[mid + 1 + j];
  
    // Merge the temp arrays back into records[l..r]

    // Initial index of first subarray
    i = 0; 
  
    // Initial index of second subarray
    j = 0; 
  
    // Initial index of merged subarray
    k = left; 
    while (i < size_left && j < size_right) {
        if (L[i].key < R[j].key) {
            (*records)[k] = L[i];
            i++;
        }
        else if (L[i].key > R[j].key){
            (*records)[k] = R[j];
            j++;
        } else{
            (*records)[k] = L[i];
            i++;

            k++;

            (*records)[k] = R[j];
            j++;
        }
        k++;
    }
  
    // Copy the remaining elements 
    // of L[], if there are any
    while (i < size_left) {
        (*records)[k] = L[i];
        i++;
        k++;
    }
  
    // Copy the remaining elements of 
    // R[], if there are any 
    while (j < size_right) {
        (*records)[k] = R[j];
        j++;
        k++;
    }
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

    // printf("\n unsorted thread %li: ", tnum);
    // for(int i=l_index; i<r_index; i++){
    //     printf("%i ", (*records)[i].key);
    // }
    // printf("\n");

    //merge sort only a given subset of the array
    mergeSort(l_index, r_index);

    // printf("\nsorted thread %li: ", tnum);
    // for(int i=l_index; i<r_index; i++){
    //     printf("%i ", (*records)[i].key);
    // }
    // printf("\n");
    
    return NULL;
}

void mergeSortedThreads(int ntosort, int nsorted){

    // pairwise sorting for the subsets of the array
    for(int i = 0; i < ntosort; i = i + 2) {
        int l_index = i * (nperthread * nsorted);              // start of subset 1
        int r_index = ((i + 2) * nperthread * nsorted) - 1;   // end of subset 2
        int mid_index = l_index + (nperthread * nsorted) - 1;

        // printf("\nsorted subset: ");
        // for(int i=l_index; i<mid_index; i++){
        //     printf("%i ", (*records)[i].key);
        // }
        // printf("\n");
        // printf("\nsorted subset: ");
        // for(int i=mid_index; i<r_index; i++){
        //     printf("%i ", (*records)[i].key);
        // }
        // printf("\n");

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
        // printf("Wrong number of args\n");
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    // input/output files
    const char *input = argv[1];
    const char *output = argv[2];

    // open input file
    int ifd = open(input, O_RDONLY);
    if(ifd < 0){
        // printf("\n\"%s \" could not open\n", input);
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    // store input data in buffer
    struct stat statbuf;
    int err = fstat(ifd, &statbuf);
    if(err < 0){
        // printf("\n\"%s \" could not open\n", input);
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    // map input data to address space
    char *ptr = mmap(NULL,statbuf.st_size,PROT_READ,MAP_SHARED,ifd,0);
    if(ptr == MAP_FAILED){
        // printf("Mapping Input Failed\n");
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    // close input file
    close(ifd);

    // number of records in input file
    nrecords = statbuf.st_size/100;

    // populate input array and send address to global pointer
    rec_t inputdata[nrecords];
    int i = 0;
    // printf("input keys:");
    for (char *r=ptr; r<(ptr + nrecords*100); r+=100, i++) {
        inputdata[i].key = *(int *)r;
        // finding values
        int value_idx = 0;
        for(char *v = r+4; v<r+100; v+=4, value_idx++){
            inputdata[i].value[value_idx] = *(int *)v;
        }

        // printf("%i ", inputdata[i].key);
    }
    // printf("\n");
    records = &inputdata;

    // unmap input data from address space
    err = munmap(ptr, statbuf.st_size);
    if(err != 0){
        // printf("UnMapping Failed\n");
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

    // printf("\nsorted: ");
    // for(int i=0; i<nrecords; i++){
    //     printf("%i ", (*records)[i].key);
    // }
    // printf("\n");

    // open output file
    int ofd = open(output, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if(ofd < 0){
        // printf("\n\"%s \" could not open\n", output);
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    // Stretch the file size
    if (lseek(ofd, nrecords*100 + 1, SEEK_SET) == -1){
        close(ofd);
        // printf("Error calling lseek() to 'stretch' the output file\n");
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }
    if (write(ofd, "", 1) == -1){
        close(ofd);
        // printf("Error writing last byte of the output file");
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    // map output file
    ptr = mmap(NULL, nrecords*100+1, PROT_READ | PROT_WRITE, MAP_SHARED, ofd, 0);
    if(ptr == MAP_FAILED){
        // printf("Mapping Output Failed\n");
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    // write to file
    int ptridx = 0;
    for(int i=0; i<nrecords; i++){
        ptr[ptridx] = (*records)[i].key;
        ptridx+=4;
        for(int j=0; j<24; j++, ptridx+=4){
           ptr[ptridx] = (*records)[i].value[j];
        }
    }

    // sync file to disk
    if(fsync(ofd)!=0){
        close(ofd);
        // printf("Output Sync Failed\n");
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    // unmap output data to address space
    err = munmap(ptr, nrecords*100+1);
    if(err != 0){
        // printf("UnMapping Failed\n");
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    // close output file
    close(ofd);

    return 0;
}

// COMP3230 Programming Assignment Two
// The sequential version of the sorting using qsort

/*
# Filename: psort_3035553020.c
# Student name and No.: WAN, Ho Yin 3035553020
# Development platform: WSL2
# Remark: Completed all features
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>

int checking(unsigned int *, long);
int compare(const void *, const void *);

// locks
pthread_mutex_t bucket_locks = PTHREAD_MUTEX_INITIALIZER; // for blocking buckets
pthread_cond_t myturn = PTHREAD_COND_INITIALIZER; // condition variable for threads to wait for their turn
int thread_count = 0; // thread count to check whether equal to thread_id
int thread_count1 = 0; // thread count 1 to check whether equal to thread_id in phrase 1s

// global variables
long size;  // size of the array
unsigned int * intarr; // array of random integers
unsigned int * intarr3; // array intarr3 for each worker threads selects p sample from it local sequence at indices.
unsigned int * intarr4; // array intarr4 to store p-1 pivot value from intarr3
//unsigned int * intarr5; // array intarr5 to store the ith partition and collects from other threads thier 1 partitions
unsigned int * intarr6; // array intarr6 to store all sorted element.
unsigned int * count; // store the value on or before ith partition
int thread_num; // thread number 
long intarr3_index = 0;
long temp2 = 0; // store the index of intarr6[] when put intarr5[] into intarr6[] 

void * buckets (void *thread_ids){

  unsigned int * intarr2; // array for storing intarr partition. 
  unsigned int * intarr5; // array intarr5 to store the ith partition and collects from other threads thier 1 partitions

  int * thread_id = (int *) thread_ids;

  double range = (double)size/ (double)thread_num;
 
  // get the up, down, temp and intarr5_size, base, partition, and intarr5_index, count[]
  long down = (*thread_id) * range;
  long up = (*thread_id + 1) * range - 1;
  long temp = down;
  long intarr5_size = 0;
  long base = 0;
  int partition = 0;
  int intarr5_index = 0;
  // store the size into count[]
  count[*thread_id] = up - down + 1; 

  // setup the size of intarr2
  intarr2 = (unsigned int *)malloc((up - down + 1)*sizeof(unsigned int)); 

  // sub intarr[] elements into intarr2[]
  for (long i = 0; i < (up - down + 1); i++){

    if(temp <= up)
    {
      intarr2[i] = intarr[temp];
      temp++;
      
    }
  }
  temp = down;

  // using quick sort to sort intarr2
  qsort(intarr2, (up-down+1), sizeof(unsigned int), compare);

  // sub intarr2[] elements into intarr[]
  for (long i = 0; i < (up - down + 1); i++){

    if(temp <= up)
    {
      intarr[temp] = intarr2[i];
      temp++;
      
    }
       
  }

  pthread_mutex_lock(&bucket_locks);


  // each worker select thread_num samples from its 'local' sequence (intarr2[]) at index, and sub it to intarr3[]
  for (int i = 0; i < thread_num; i++){

    int index = i*size/(thread_num * thread_num);
    intarr3[intarr3_index] = intarr2[index];
    
    intarr3_index++;
  }


  // do broadcast
  pthread_cond_broadcast(&myturn);
  thread_count++;

  pthread_mutex_unlock(&bucket_locks);


  //step into phrase 3 and 4
  pthread_mutex_lock(&bucket_locks);

  while(thread_count != thread_num + 1){
    pthread_cond_wait(&myturn, &bucket_locks);
  }

  pthread_mutex_unlock(&bucket_locks);


  partition = *thread_id; // understand which partition 

  // find out the elements number (intarr5_size) of each partition in different threads
  for (int i = 0; i < thread_num; i++){ // know the thread number

    if (i == 0){ // when thread is 0

      for (long a = 0; a < count[i]; a++){ // loop in thread 0 from 0 
        
        if (partition == 0){ // if partition == 0;

          if (intarr[a] <= intarr4[partition]){ 
            intarr5_size++;
          }
        }
        else if (partition > 0){

          if (partition == thread_num - 1){
            if (intarr[a] > intarr4[partition - 1]){
              intarr5_size++;
            }
          }
          else {
            if (intarr[a] > intarr4[partition - 1] && intarr[a] <= intarr4[partition]){
              intarr5_size++;
            }
          }
        }
      }

      base = base + count[i];

    }
    else if (i > 0){
      
      for (long a = base; a < base + count[i]; a++){ //loop in thread 1 or more

        if (partition == 0){

          if (intarr[a] <= intarr4[partition]){ 
            intarr5_size++;
          }
        }
        else if (partition > 0){

          if (partition == thread_num - 1){
            
            if (intarr[a] > intarr4[partition - 1]){
              intarr5_size++;
            }
          }
          else {
            if (intarr[a] > intarr4[partition - 1] && intarr[a] <= intarr4[partition]){
              intarr5_size++;
            }
          }
        }
      }
      base = base + count[i];
    }
  }

 
  // create intarr5
  intarr5 = (unsigned int *)malloc((intarr5_size)*sizeof(unsigned int));

  //------------------------------------------------------------------------------------------
  // sub the elements of each partition in different threads to intarr5[]

  base = 0; 
  for (int i = 0; i < thread_num; i++){ // know the thread number

    if (i == 0){ // when thread is 0

      for (long a = 0; a < count[i]; a++){ // loop in thread 0 from 0 
        
        if (partition == 0){ // if partition == 0;

          if (intarr[a] <= intarr4[partition]){ 
            intarr5[intarr5_index] = intarr[a];
            intarr5_index++;
          }
        }
        else if (partition > 0){

          if (partition == thread_num - 1){
            if (intarr[a] > intarr4[partition - 1]){
              intarr5[intarr5_index] = intarr[a];
              intarr5_index++;
            }
          }
          else {
            if (intarr[a] > intarr4[partition - 1] && intarr[a] <= intarr4[partition]){
              intarr5[intarr5_index] = intarr[a];
              intarr5_index++;
            }
          }
        }
      }

      base = base + count[i];

    }
    else if (i > 0){
      
      for (long a = base; a < base + count[i]; a++){ //loop in thread 1 or above

        if (partition == 0){

          if (intarr[a] <= intarr4[partition]){ 
            intarr5[intarr5_index] = intarr[a];
            intarr5_index++;
          }
        }
        else if (partition > 0){

          if (partition == thread_num - 1){
            
            if (intarr[a] > intarr4[partition - 1]){
              intarr5[intarr5_index] = intarr[a];
              intarr5_index++;
            }
          }
          else {
            if (intarr[a] > intarr4[partition - 1] && intarr[a] <= intarr4[partition]){
              intarr5[intarr5_index] = intarr[a];
              intarr5_index++;
            }
          }
        }
      }
      base = base + count[i];
    }
  }

  qsort(intarr5, intarr5_size, sizeof(unsigned int), compare);

  pthread_mutex_lock(&bucket_locks);
  // make sure the thread sub element into intarr6[] according its thread_id in asc order 
  while(thread_count1 != *thread_id){
    pthread_cond_wait(&myturn, &bucket_locks);
  }

  // sub intarr5[] elements into intarr6[]
  for (long i = 0; i < intarr5_size; i++){

      intarr6[temp2] = intarr5[i];
      temp2++;  
  }

  // do broadcast
  pthread_cond_broadcast(&myturn);
  thread_count1++;

  pthread_mutex_unlock(&bucket_locks);

  free(intarr2);
  free(intarr5);

  pthread_exit(NULL);

}

int main (int argc, char **argv)
{
  long i;
  struct timeval start, end;

  // check how many input
  if ((argc < 2))
  {
    printf("Usage: seq_sort <number>\n");
    exit(0);
  }

  // check thread_num when user input three arguments
  if (argc == 3){

    thread_num = atol(argv[2]);
  
    if(thread_num > 1){
      //printf("thread_num: %d\n", thread_num);
    }
    else{
      printf("Please input thread number greater than 1.\n");
      exit(0);
    }
  }
  else{
    thread_num = 4;
  }
    
  // get the size of instarr
  size = atol(argv[1]);

  // setup the thread id
  int thread_ids[thread_num];
  // setup the threads
  pthread_t threads[thread_num];

  // setup the thread id
  for(int i = 0.0; i < thread_num; i++){
    thread_ids[i] = i;
    //printf("thread id: %d\n", thread_ids[i]);
  }

  // set the size of for instarr, intarr2 and intarr3
  intarr = (unsigned int *)malloc(size*sizeof(unsigned int));
  intarr3 = (unsigned int *)malloc((thread_num * thread_num)*sizeof(unsigned int));
  intarr4 = (unsigned int *)malloc((thread_num - 1)*sizeof(unsigned int));
  intarr6 = (unsigned int *)malloc(size*sizeof(unsigned int));
  count = (unsigned int *)malloc(thread_num*sizeof(unsigned int));

  // check whether malloc is used correctly
  if (intarr == NULL) {perror("malloc"); exit(0); }
  
  // set the random seed for generating a fixed random
  // sequence across different runs
  char * env = getenv("RANNUM");  //get the env variable
  if (!env)                       //if not exists
    srandom(3230);
  else
    srandom(atol(env));
  
  // put number to intarr randomly
  for (i=0; i<size; i++) {
    intarr[i] = random();
  }


  // measure the start time
  gettimeofday(&start, NULL);

  // create threads base on thread_num
  for (int i = 0; i < thread_num; i++){
    pthread_create(&threads[i], NULL, buckets, (void *)&thread_ids[i]);
  }

  pthread_mutex_lock(&bucket_locks);

  //ensure phrase one finish before entering phrase two
  while(thread_count != thread_num){
    pthread_cond_wait(&myturn, &bucket_locks);
  }

  qsort(intarr3, thread_num*thread_num, sizeof(unsigned int), compare);

  // select the pivot values in intarr3[] and sub it to intarr4[]
  for (int i = 1; i < thread_num; i++){

    int t = i*thread_num + (thread_num/2) - 1;
    intarr4[i-1] = intarr3[t];
  }

  thread_count++;

  pthread_cond_broadcast(&myturn);

  pthread_mutex_unlock(&bucket_locks);


  // join threads
  for (int i = 0; i < thread_num; i++){
    pthread_join(threads[i], NULL);
  }

  // measure the end time
  gettimeofday(&end, NULL);


  intarr = intarr6;
  pthread_mutex_destroy(&bucket_locks);

  
  // check whether intar6 is in sorted order
  if (!checking(intarr, size)) {
    printf("The array is not in sorted order!!\n");
  }
  //else printf("array is sorted --------\n");
  
  printf("Total elapsed time: %.4f s\n\n", (end.tv_sec - start.tv_sec)*1.0 + (end.tv_usec - start.tv_usec)/1000000.0);
  
  /*free(intarr6);
  free(intarr);*/
  free(intarr3);
  free(intarr4);
  pthread_cond_destroy(&myturn);
  return 0;
}

int compare(const void * a, const void * b) {
  return (*(unsigned int *)a>*(unsigned int *)b) ? 1 : ((*(unsigned int *)a==*(unsigned int *)b) ? 0 : -1);
}

int checking(unsigned int * list, long size) {
  long i;
  printf("First : %d\n", list[0]);
  printf("At 25%%: %d\n", list[size/4]);
  printf("At 50%%: %d\n", list[size/2]);
  printf("At 75%%: %d\n", list[3*size/4]);
  printf("Last  : %d\n", list[size-1]);
  for (i=0; i<size-1; i++) {

    //printf("list[i]: %d ; list[i+1]: %d\n", list[i], list[i+1]);
    if (list[i] > list[i+1]) {
      return 0;
    }
  }
  return 1;
}
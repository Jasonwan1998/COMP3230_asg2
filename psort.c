// COMP3230 Programming Assignment Two
// The sequential version of the sorting using qsort

/*
# Filename: 
# Student name and No.:
# Development platform:
# Remark:
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

// global variables
long size;  // size of the array
unsigned int * intarr; // array of random integers
unsigned int * intarr3; // array intarr3 for each worker threads selects p sample from it local sequence at indices.
int thread_num; // thread number 
long intarr3_index = 0;

void * buckets (void *thread_ids){

  unsigned int * intarr2; // array for storing intarr partition. 

  int * thread_id = (int *) thread_ids;

  double range = (double)size/ (double)thread_num;
 
  // get the up and down
  long down = (*thread_id) * range;
  long up = (*thread_id + 1) * range - 1;
  long temp = down;

  // setup the size of intarr2
  intarr2 = (unsigned int *)malloc((up - down + 1)*sizeof(unsigned int)); 


  // sub intarr[] elements into intarr2[]
  for (long i = 0; i < (up - down + 1); i++){

    if(temp <= up)
    {
      intarr2[i] = intarr[temp];
      //printf("intarr2[%ld]: %d     temp: %ld\n", i, intarr2[i], temp);
      temp++;
      
    }
    else
      break;   
  }

  // using quick sort to sort intarr2
  qsort(intarr2, (up-down+1), sizeof(unsigned int), compare);

  for (long i=0; i<(up-down+1); i++) {

    printf("intarr2[%ld]: %d ; thread_id: %d\n", i, intarr2[i], *thread_id);
  }

  printf("\n\n");

  if (!checking(intarr2, up-down+1)) {
    printf("The array is not in sorted order!!\n");
  }
  else printf("The array is sorted!! \n\n");

  pthread_mutex_lock(&bucket_locks);

  printf("thread_count: %d ; thread_id: %d\n\n", thread_count, *thread_id);

  while(thread_count != *thread_id){
    printf("waiting\n\n");
    pthread_cond_wait(&myturn, &bucket_locks);
  }
    

  // each worker select thread_num samples from its 'local' sequence at indices
  for (int i = 0; i < thread_num; i++){

    int index = i*size/(thread_num * thread_num);
    intarr3[intarr3_index] = intarr2[index];
    
    //printf("Thread number: %d ; i: %d ; size: %ld  ", thread_num, i, size);
    printf("intarr3[%ld]: %d ; (i*size)/thread_num^2: %d\n", intarr3_index, intarr2[index], index);
    intarr3_index++;
  }

  


  printf("I am thread %d. Range is %f, Down is %ld, Up is %ld\n", *thread_id, range, down, up);
  //printf("I am thread %d.\n", *thread_id);

  // do broadcast
  pthread_cond_broadcast(&myturn);
  thread_count++;

  pthread_mutex_unlock(&bucket_locks);

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
      printf("thread_num: %d\n", thread_num);
    }
    else{
      printf("Please input thread number greater than 1.\n");
      exit(0);
    }
  }
  else{
    thread_num = 4;
      printf("thread_num: %d\n", thread_num);
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

  // set the size of for instarr and intarr2
  intarr = (unsigned int *)malloc(size*sizeof(unsigned int));
  intarr3 = (unsigned int *)malloc((thread_num * thread_num)*sizeof(unsigned int));

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
    printf("intarr[%ld]: %d\n", i, intarr[i]);
  }

  printf("----------------------------------------\n\n");


  // measure the start time
  gettimeofday(&start, NULL);
  
  // just call the qsort library
  // replace qsort by your parallel sorting algorithm using pthread

  //qsort(intarr, size, sizeof(unsigned int), compare);

  // create threads
  for (int i = 0; i < thread_num; i++){
    pthread_create(&threads[i], NULL, buckets, (void *)&thread_ids[i]);
  }

  // measure the end time
  gettimeofday(&end, NULL);

  // join threads
  for (int i = 0; i < thread_num; i++){
    pthread_join(threads[i], NULL);
  }

  pthread_mutex_destroy(&bucket_locks);

  
  // check whether intarr is in sorted order
  if (!checking(intarr, size)) {
    printf("The array is not in sorted order!!\n");
  }
  
  printf("Total elapsed time: %.4f s\n\n", (end.tv_sec - start.tv_sec)*1.0 + (end.tv_usec - start.tv_usec)/1000000.0);
    
  free(intarr);
  return 0;
}

int compare(const void * a, const void * b) {
  return (*(unsigned int *)a>*(unsigned int *)b) ? 1 : ((*(unsigned int *)a==*(unsigned int *)b) ? 0 : -1);
}

int checking(unsigned int * list, long size) {
  long i;
  /*printf("First : %d\n", list[0]);
  printf("At 25%%: %d\n", list[size/4]);
  printf("At 50%%: %d\n", list[size/2]);
  printf("At 75%%: %d\n", list[3*size/4]);
  printf("Last  : %d\n", list[size-1]);*/
  for (i=0; i<size-1; i++) {

    //printf("list[i]: %d ; list[i+1]: %d\n", list[i], list[i+1]);
    if (list[i] > list[i+1]) {
      return 0;
    }
  }
  return 1;
}
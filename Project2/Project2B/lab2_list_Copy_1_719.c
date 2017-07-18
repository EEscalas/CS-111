//Elena Escalas
//704560697
//elenaescalas@g.ucla.edu

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include "SortedList.h"

/*
timespec structure definted in time.h:
struct timespec {
time_t tv_sec; //seconds
long tv_nsec;  //nanoseconds
};*/

//flags
int opt_threads = 0;
int opt_iterations = 0;
int opt_lists = 0;
int opt_yield = 0;
int opt_yield_insert = 0;
int opt_yield_delete = 0;
int opt_yield_lookup = 0;
int opt_debug = 0;
int opt_debug_general = 0;
int opt_sync = 0;
int yielding = 0;

//arg variables
long long num_threads = 1;
long long num_iterations = 1;
long long num_lists = 1;
char sync_type;

//list variables
SortedList_t *head;
SortedListElement_t *elements;

//sync variables
pthread_mutex_t *mutex_locks;
volatile int* locks;

//sorted list variables
char keyString[11] = "";

//wait-for-lock time variables
long long * timeArr;
long long * lockArr;

//hash elements into buckets
int hash(char* key)
{
  int *hashPointer = (int *) key;
  int hashValue = *key;
  int ret = hashValue % num_lists;
  if (opt_debug) fprintf(stdout, "Hash for key: %s is %i\n", key, ret);
  return ret;
}

//calculate elapsed time
long long getElapsedTime(struct timespec start_time, struct timespec end_time)
{
  return (1000000000L * (end_time.tv_sec - start_time.tv_sec)) + (end_time.tv_nsec - start_time.tv_nsec);
}

void *threadTask(void *thread_arg)
{
  struct timespec waiting_for_lock, obtained_lock;
  long long waitlock_total = 0;
  long long total_num_locks = 0;

  /*// apply locks: either mutex or spin
  if (sync_type == 'm')
    {
      clock_gettime(CLOCK_MONOTONIC, &waiting_for_lock);
      if (opt_debug) fprintf(stdout, "locking mutex lock\n");
      pthread_mutex_lock(&mutex_lock);
      clock_gettime(CLOCK_MONOTONIC, &obtained_lock);
      waitlock_total += getElapsedTime(waiting_for_lock, obtained_lock);
      total_num_locks++;
      if (opt_debug) fprintf(stdout, "Total waitlock time (with mutex): %llu\n", waitlock_total);
    }
  else if (sync_type == 's')
    {
      clock_gettime(CLOCK_MONOTONIC, &waiting_for_lock);
      if (opt_debug) fprintf(stdout, "locking spin lock\n");
      while (__sync_lock_test_and_set(&lock,1));
      clock_gettime(CLOCK_MONOTONIC, &obtained_lock);
      waitlock_total += getElapsedTime(waiting_for_lock, obtained_lock);
      total_num_locks++;
      if (opt_debug) fprintf(stdout,"Total waitlock time (with spin): %llu\n", waitlock_total);
      }*/
  
  if (opt_debug) fprintf(stdout, "This thread has gotten past locks, congrats\n"); 
  
  int* ptr_thread_num = (int*)thread_arg;
  int thread_num = *ptr_thread_num;

  //if (opt_debug_general && opt_sync) fprintf(stdout, "Thread %i has the lock\n", thread_num);

  if (opt_debug) fprintf(stdout, "Thread %i is about to start inserting, congrats\n", thread_num);

  long long element_start = thread_num*num_iterations;
  if (opt_debug_general) fprintf(stdout, "The start index for elements for thread %i is %llu\n", thread_num, element_start);
  long long element_end = element_start + num_iterations;

  ///////////////////* INSERT *///////////////////
  long j;
  int list_num;
  for (j = element_start; j < element_end; j++)
    {
      char *element_to_insert_key = malloc(sizeof(char)*11);
      if(element_to_insert_key == NULL)
	{
	  fprintf(stderr, "Error: unable to allocate memory to find hash to insert key \n");
	  exit(1);
	}
      else
	strcpy(element_to_insert_key,(elements+j)->key);
      
      list_num = hash(element_to_insert_key);
      
      // LOCK //
      if (sync_type == 'm')
	{	
	  clock_gettime(CLOCK_MONOTONIC, &waiting_for_lock);
	  pthread_mutex_lock(&mutex_locks[list_num]);
	  clock_gettime(CLOCK_MONOTONIC, &obtained_lock);
	  waitlock_total += getElapsedTime(waiting_for_lock, obtained_lock);
	  total_num_locks++;
	  if (opt_debug_general)fprintf(stdout, "Thread %i has the lock for list %i. Ready to Insert.\n", thread_num, list_num);
	}
      else if (sync_type == 's')
	{
	  clock_gettime(CLOCK_MONOTONIC, &waiting_for_lock);
	  while (__sync_lock_test_and_set(&locks[list_num],1));
	  clock_gettime(CLOCK_MONOTONIC, &obtained_lock);
	  waitlock_total += getElapsedTime(waiting_for_lock, obtained_lock);
	  total_num_locks++;
	  if (opt_debug_general)fprintf(stdout, "Thread %i has the lock for list %i. Ready to Insert.\n", thread_num, list_num);
	}
      
      SortedList_insert(&head[list_num],elements+j);

      // UNLOCK //
      if (sync_type == 'm')
	pthread_mutex_unlock(&mutex_locks[list_num]);
      else if (sync_type == 's')
	__sync_lock_release(&locks[list_num]);

      if (opt_debug || opt_debug_general) fprintf(stdout,"Thread %i has inserted an element with key %s into list %i and relinquished the lock\n", thread_num, (elements+j)->key, list_num);
    }

  //////////////////* LENGTH *////////////////////
  //get list length

  if (opt_debug) fprintf(stdout, "Getting length: locking all the locks\n");
  // LOCK ALL THE LIST LOCKS //
  int l;
  for (l = 0; l < num_lists; l++)
    {
      if (sync_type == 'm')
        {
	  clock_gettime(CLOCK_MONOTONIC, &waiting_for_lock);
          pthread_mutex_lock(&mutex_locks[l]);
          clock_gettime(CLOCK_MONOTONIC, &obtained_lock);
          waitlock_total += getElapsedTime(waiting_for_lock, obtained_lock);
          total_num_locks++;
	  if (opt_debug_general)fprintf(stdout, "Thread %i has the lock for list %i.\n", thread_num, l);
        }
      else if (sync_type == 's')
        {
          clock_gettime(CLOCK_MONOTONIC, &waiting_for_lock);
          while (__sync_lock_test_and_set(&locks[l],1));
          clock_gettime(CLOCK_MONOTONIC, &obtained_lock);
          waitlock_total += getElapsedTime(waiting_for_lock, obtained_lock);
          total_num_locks++;
	  if (opt_debug_general)fprintf(stdout, "Thread %i has the lock for list %i.\n", thread_num, l);
        }
    }
  
  if (opt_debug_general)fprintf(stdout, "Thread %i has all the locks. Ready to find the length of all lists.\n", thread_num);

  long long list_length = 0;
  int m;
  for (m = 0; m < num_lists; m++)
    {
      list_length += SortedList_length(&head[m]);
    }
  
  if (opt_debug_general)fprintf(stdout, "Thread %i has found the list length to be: %llu\n", thread_num, list_length);
  
  // UNLOCK ALL THE LIST LOCKS //
  for (l = 0; l < num_lists; l++)
    {
      if (sync_type == 'm')
	{	
	  pthread_mutex_unlock(&mutex_locks[l]);
	  fprintf(stdout, "Thread %i has released lock %i\n", thread_num, l);
	}
      else if (sync_type == 's')
	__sync_lock_release(&locks[l]);
    }


  if (opt_debug) fprintf(stdout, "Thread %i has inserted: list Length: %llu\n", thread_num, list_length);
  
  //look up and delete each of the keys it had previously inserted
  char element_key[11] = "";
  
  if (opt_debug) fprintf(stdout, "Thread %i is about to delete, congrats\n", thread_num);

  SortedListElement_t *element_to_delete;
  if (opt_debug) fprintf(stdout, "num_iterations = %llu\n", num_iterations);
  if (opt_debug) fprintf(stdout, "element start = %llu\n", element_start);
  if (opt_debug) fprintf(stdout, "element start + num_iterations = %llu\n", element_start+num_iterations);  
  if (opt_debug) fprintf(stdout, "element end equals: %llu\n", element_end);

  for (j = element_start; j < element_end; j++)
    {
      strcpy(element_key, (elements+j)->key);
      if (opt_debug) fprintf(stdout, "Thread %i has successfully copied %s to element_key, congrats\n", thread_num, element_key);


      ///////////////* LOOKUP */////////////////
      list_num = hash(element_key);

      // LOCK //
      if (sync_type == 'm')
        {
          clock_gettime(CLOCK_MONOTONIC, &waiting_for_lock);
          pthread_mutex_lock(&mutex_locks[list_num]);
          clock_gettime(CLOCK_MONOTONIC, &obtained_lock);
          waitlock_total += getElapsedTime(waiting_for_lock, obtained_lock);
          total_num_locks++;
        }
      else if (sync_type == 's')
        {
          clock_gettime(CLOCK_MONOTONIC, &waiting_for_lock);
          while (__sync_lock_test_and_set(&locks[list_num],1));
          clock_gettime(CLOCK_MONOTONIC, &obtained_lock);
          waitlock_total += getElapsedTime(waiting_for_lock, obtained_lock);
          total_num_locks++;
        }

      element_to_delete = SortedList_lookup(&head[list_num],element_key);

      if (opt_debug) fprintf(stdout, "element_to_delete has successfully been found and its key is: %s\n", element_to_delete->key);
      
      if (element_to_delete == NULL)
	{
	  if (opt_debug) fprintf(stdout, "lookup returned null\n");
	  fprintf(stderr,"Error: Corrupted List - Element not found in list\n");
	  exit(2);
	}

      if(opt_debug) fprintf(stdout, "element_to_delete is not null\n");


      ////////////////* DELETE *///////////////
      int delFlag = SortedList_delete(element_to_delete);
      if (delFlag != 0)	
	{
	  fprintf(stderr,"Error: Corrupted List - Invalid prev/next pointers\n");
	  exit(2);
	}
      if (opt_debug_general) fprintf(stdout, "Thread %i has succesfully deleted the element with the key: %s\n", thread_num, element_key);
      
      
      // UNLOCK LOCK //
      if (sync_type == 'm')
	pthread_mutex_unlock(&mutex_locks[list_num]);
      else if (sync_type == 's')
	__sync_lock_release(&locks[list_num]);
      
    }
  
  timeArr[thread_num] = waitlock_total;
  
  if (opt_sync)
    lockArr[thread_num]= total_num_locks;

// thread exits at end of function
}

void sigHandler()
{
  fprintf(stderr,"Error: Corrupted List - segmentation fault\n");
  exit(2);
}

int main (int argc, char** argv)
{
  srand(time(NULL));
  
  //SEGFAULT SIGNAL HANDLER
  signal(SIGSEGV, sigHandler);
  
  //getopt_long prep
  int opt;
  int input_fd;
  int output_fd;
  char buffer;
  
  int this_option_optiond = optind ? optind : 1;
  int option_index = 0;
  
  static struct option long_options[] =
    {
      {"threads",    optional_argument, 0, 't'},
      {"iterations", optional_argument, 0, 'i'},
      {"lists",      optional_argument, 0, 'l'},
      {"yield",      required_argument, 0, 'y'},
      {"sync",       required_argument, 0, 's'},
      {"debug",      no_argument,       0, 'd'},
      {"gendebug",   no_argument,       0, 'g'},
      {0, 0, 0, 0}
    };
  
  while (1) {
    
    opt = getopt_long(argc, argv, "", long_options, &option_index);
    
    if (opt == -1)
      break;
    
    switch(opt)
      {
      case 'd':
	opt_debug = 1;
	break;
      case 'g':
	opt_debug_general = 1;
	break;
      case 't':
	opt_threads = 1;
	if (optarg == NULL || (atoi(optarg)== 0))
	  num_threads = 1;
	else
	  num_threads = atoi(optarg);
        break;
      case 'i':
	opt_iterations = 1;
	if (optarg == NULL || (atoi(optarg)== 0))
	  num_iterations = 1;
	else
	  num_iterations = atoi(optarg);
	break;
      case 'l':
	opt_lists = 1;
	if (optarg == NULL || (atoi(optarg) == 0))
	  num_lists = 1;
	else
	  num_lists = atoi(optarg);
	break;
      case 'y':
	yielding = 1;
	int i;
	for (i = 0; i < strlen(optarg); i++)
	  {
	    if (optarg[i] == 'i')
	      {
		opt_yield_insert = 1;
		opt_yield = opt_yield | INSERT_YIELD;
	      }
	    if (optarg[i] == 'd')
	      {
		opt_yield_delete = 1;
		opt_yield = opt_yield | DELETE_YIELD;
	      }
	    if (optarg[i] == 'l')
	      {
		opt_yield_lookup = 1;
		opt_yield = opt_yield | LOOKUP_YIELD;
	      }
	  }
	break;
      case 's':
	opt_sync = 1;
	sync_type = optarg[0];
	break;
      default:
	exit(1);
	break;
      }
  }    
  
  if (opt_debug) fprintf(stdout, "You have recognized debug, congrats\n");
  
  //initialize an empty lists
  head = malloc(sizeof(SortedList_t)*num_lists);
  if (head == NULL)
    {
      fprintf(stderr, "Error: Unable to initialize list\n");
      exit(1);
    }
  
  //malloc list locks
  if (sync_type == 'm')
    {
      mutex_locks = malloc(sizeof(pthread_mutex_t)*num_lists);
      pthread_mutex_t tempLock;
      int n;
      for (n = 0; n < num_lists; n++)
	{
	  mutex_locks[n] = tempLock;
	}
    }
  else if (sync_type == 's')
    {
      locks = malloc(sizeof(int) * num_lists);
      int m;
      for (m = 0; m < num_lists; m++)
	{
	  locks[m] = 0;
	}
    }
  
  //create and initialize thread*iterations list elements
  long long num_elements = num_threads*num_iterations;
  
  elements = malloc(sizeof(SortedListElement_t)*num_elements);
  if (elements == NULL)
    {
      fprintf(stderr, "Error: Unable to intialize list elements\n");
      exit(1);
    }
  
  if (opt_debug) fprintf(stdout, "You have intialized the elements, congrats\n");
  
  long numEle;
  for (numEle = 0; numEle < num_elements; numEle++)
    {
      char * newKey = malloc(sizeof(char)*11);
      if (newKey == NULL)
	{
	  fprintf(stderr, "Error: unable to allocate memory for keys\n");
	  exit(1);
	}
      
      //create random key
      int p = 0;
      for (p = 0; p < 10; p++)
	{
	  newKey[p] = 'a' + (random() % 26);
	  if (opt_debug) fprintf(stdout, "added %c to newKey\n", newKey[p]);
	}
   
      newKey[10] = '\0';
      
      if (opt_debug) fprintf(stdout, "Created element number %llu with key: %s\n", numEle, newKey);
     
      (elements+numEle)->key = newKey;
      
      }
  
  if (opt_debug) fprintf(stdout, "You have created random keys, congrats\n");
  
  timeArr = malloc(sizeof(long long)*num_threads);
  if (timeArr == NULL)
    fprintf(stderr, "Error: Unable to initialize wait-for-lock time counters\n");

  lockArr = malloc(sizeof(long long)* num_threads);
  if (lockArr == NULL)
    fprintf(stderr, "Error: Unable to initialize lock counters\n");

  int *thread_number = malloc(sizeof(int) * num_threads);
  if (thread_number == NULL)
    {
      fprintf(stderr, "Error: unable to allocate thread number array\n");
      exit(1);
    }

  int thr = 0;
  for (thr = 0; thr < num_threads; thr++)
    {
      thread_number[thr] = thr;
    }

  //start the timer
  struct timespec start_time, end_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  
  //initialize the mutex locks
  if (sync_type == 'm')
    {
      int i; 
      for (i = 0; i < num_lists; i++)
	{
	  if (pthread_mutex_init(&mutex_locks[i], NULL)!= 0)
	    fprintf(stderr, "Error: Unable to intiialize lock\n");
	}
    }
    
  
  if (opt_debug) fprintf(stdout, "You have intialized the mutex lock, congrats\n");
  
  long th = 0;
  
  pthread_t *threadArr;
  threadArr = malloc(sizeof(pthread_t)*num_threads);
  if (threadArr == NULL)
    fprintf(stderr, "Error: Unable to create threads\n");

  //long long element_num = 0;
  for (th = 0; th < num_threads; th++)
    {	
      //create threads
      
      if (opt_debug) fprintf(stdout, "The thread number is: %i\n", thread_number[th]);
      if((pthread_create(&threadArr[th], NULL, &threadTask, (void *)(thread_number+th))) != 0)
	{
	  fprintf(stderr, "Error: unable to create threads\n");
	  exit(1);
	}
    }     
  
  for (th = 0; th < num_threads; th++)
    {
      //join threads
	pthread_join(threadArr[th],NULL);
    }
  
  //get end time
  clock_gettime(CLOCK_MONOTONIC, &end_time); 
  
  if (SortedList_length(head) != 0)
      {
	fprintf(stderr,"Error: Corrupted List - Final list length not equal to zero\n");
	exit(2);
      }
    
  //deinitialize allocated memeory
  free(head);
  free(threadArr);
  free(elements);

  /*  print results  */
  
  // print name
  fprintf(stdout,"list-");
  if (yielding) 
    {
      if (opt_yield_insert)
	fprintf(stdout,"i");
      if (opt_yield_delete)
	fprintf(stdout,"d");
      if (opt_yield_lookup)
	fprintf(stdout, "l");
      
      fprintf(stdout, "-");
    }
  else
    fprintf(stdout, "none-");
  if (opt_sync)
    fprintf(stdout, "%c,", sync_type);
  else fprintf(stdout, "none,");
  
  long long ti = 0;
  long long total_waitlock_time = 0;
  for (ti = 0; ti < num_threads; ti++)
    {
      total_waitlock_time += timeArr[ti];
    }

  long long lo = 0;
  long long total_locks = 0;
  for (lo = 0; lo < num_threads; lo++)
    {
      total_locks += lockArr[lo];
    }

  long long operations = num_threads*num_iterations*3;
  long long run_time = (1000000000L * (end_time.tv_sec - start_time.tv_sec)) + (end_time.tv_nsec - start_time.tv_nsec);
  long long average_time_per_operation = run_time/operations;
  long long average_waitforlock = total_waitlock_time/num_threads;

  fprintf(stdout, "%llu,", num_threads);                   // number of threads
  fprintf(stdout, "%llu,",num_iterations);                 // number of iterations
  fprintf(stdout, "%i,", num_lists);                       // number of lists
  fprintf(stdout, "%llu,", operations);                    // number of operations
  fprintf(stdout, "%llu,", run_time);                      // total run time
  fprintf(stdout, "%llu", average_time_per_operation);   // average time per operations
  if (opt_sync)
    {
      fprintf(stdout, ",%llu\n", average_waitforlock);
    }
  else fprintf(stdout,"\n");

  if (sync_type == 'm')
    {  
      int i = 0;
      for (i = 0; i < num_lists; i++)
	{
	  pthread_mutex_destroy(&mutex_locks[i]);
	}
    }
  free(timeArr);
  exit(0);
}



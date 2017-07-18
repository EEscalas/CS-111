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

/*
timespec structure definted in time.h:
struct timespec {
  time_t tv_sec; //seconds
  long tv_nsec;  //nanoseconds
};*/

//flags
int opt_threads = 0;
int opt_iterations = 0;
int opt_yield = 0;
int opt_sync = 0;
int opt_debug = 0;

//arg variables
long long num_threads = 1;
long long num_iterations = 1;
char sync_type;


long long counter = 0;

//mutex variable
pthread_mutex_t count_mutex;
//spin variable
int lock = 0;


void add(long long *pointer, long long value);
void mutex_add(long long *pointer, long long value);
void spin_add(long long *pointer, long long value);
void compare_and_swap_add(long long *pointer, long long value);

//void locked_add(long long *pointer, long long value);
//long long get_count();

/*struct timespec getStartTime()
{
  struct timespec start_time;
  int timer_valid = clock_gettime(CLOCK_MONOTONIC,&start_time);
  if (timer_valid < 0)
    fprintf(stderr, "Error: clock failure\n");
}

long long getTime_nano(struct timespec start)
{
  struct timespec current_time;
  int timer_valid = clock_gettime(CLOCK_MONOTONIC,&current_time);
  if (timer_valid < 0) fprintf(stderr, "Error: clock failure\n");
  if (opt_debug) fprintf(stdout, "Current time: %i seconds %i nanoseconds\n",current_time.tv_sec, current_time.tv_nsec);
  long long cur_nano = current_time.tv_sec*1000000000 + current_time.tv_nsec;
  long long start_nano = start.tv_sec*1000000000 + start.tv_nsec;
  return cur_nano - start_nano;
  }*/

void *threadTask(void *task_counter)
{
  //long long *ctr = (long long *) task_counter;
  
  long it = 0;
  
  for (it = 0; it < num_iterations; it++)
    { 
      if (!opt_sync) add(&counter,1);
      else if (sync_type == 'm') mutex_add(&counter,1);
      else if (sync_type == 's') spin_add(&counter,1);
      else if (sync_type == 'c') compare_and_swap_add(&counter,1);

      if (opt_debug) fprintf(stdout, "//   A Thread Completed Iteration %i\n//       Counter Value: %i\n", it+1,counter);      
    }
  for (it = 0; it < num_iterations; it++)
    {
      if (!opt_sync) add(&counter,-1);
      else if (sync_type == 'm') mutex_add(&counter,-1);
      else if (sync_type == 's') spin_add(&counter,-1);
      else if (sync_type == 'c') compare_and_swap_add(&counter,-1);
      
      if (opt_debug) fprintf(stdout, "//   A Thread Completed Iteration %i\n//       Counter Value: %i\n", it+1,counter);
    }
  // thread exits at end of function
}

int main (int argc, char** argv)
{
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
        {"yield",      no_argument,       0, 'y'},
        {"sync",       required_argument, 0, 's'},
        {"debug",      no_argument,       0, 'd'},
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
      case 'y':
	opt_yield = 1;
	break;
      case 's':
	opt_sync = 1;
	sync_type = *optarg;
	if (sync_type != 'm' && sync_type != 's' && sync_type != 'c')
	  {
	    fprintf(stderr, "Error: Invalid argument for [--sync] option\n");
	    exit(1);
	  }
	break;
      default:
	exit(1);
	break;
      }
  }    
  
  if (opt_debug)
    {
      if (opt_threads)
	fprintf(stdout, "num_threads = %i\n", num_threads);
      if (opt_iterations)
	fprintf(stdout, "num_iterations = %i\n", num_iterations); 
      if (opt_yield)
	fprintf(stdout, "yielding\n");
      if (opt_sync)
	{
	  if (sync_type == 'm')
	    fprintf(stdout, "syncing method: mutex\n");
	  else if (sync_type == 's')
	    fprintf(stdout, "synching method: spin locks\n");
	  else if (sync_type == 'c')
	    fprintf(stdout, "synching method: compare and swap\n");
	  else
	    {
	      fprintf(stderr, "Error: invalid syncing method\n");
	      exit(1);
	    }
	}
    }
  
  
  counter = 0;
  
  //start the timer
  struct timespec start_time, end_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  
  
  if (opt_debug) fprintf(stdout, "Start Time: %i seconds and %i nanoseconds\n", start_time.tv_sec, start_time.tv_nsec);
  
  long th = 0;
  
  pthread_t *threadArr;
  threadArr = malloc(sizeof(pthread_t)*num_threads);
  if (threadArr == NULL)
    fprintf(stderr, "Error: Unable to create threads\n");
  
  //initialize the mutex
  if (sync_type == 'm')
    {
      if (pthread_mutex_init(&count_mutex, NULL) != 0)
	{
	  fprintf(stderr, "Error: unable to create mutex\n");
	  exit(1);
	}
    }
  
  for (th = 0; th < num_threads; th++)
    {
      //create threads
      if(pthread_create(&threadArr[th], NULL, threadTask,&counter) != 0)
	{
	  fprintf(stderr, "Error: unable to create threads\n");
	  exit(1);
	}
      
      //if (opt_debug) fprintf(stdout, "//   Thread %i Created: \n//       Counter Value: %i\n", th+1, counter);
    }     
  
  for (th = 0; th < num_threads; th++)
    {
      //join threads
      pthread_join(threadArr[th],NULL);
      if (opt_debug) fprintf(stdout, "//   Thread %i Joined: \n//        Counter Value: %i\n", th+1, counter);
    }
  
  //get end time
  clock_gettime(CLOCK_MONOTONIC, &end_time); 

  long operations = num_threads * num_iterations * 2;
  long run_time = (1000000000L * (end_time.tv_sec - start_time.tv_sec)) + (end_time.tv_nsec - start_time.tv_nsec);
  long average_time_per_operation = run_time/operations;
  long total = counter;

  
  if (opt_debug) fprintf(stdout, "Total run time: %i nanoseconds\n", run_time);
  
  /*  print results  */
  
  // print name
  fprintf(stdout,"add-");
  if (opt_yield) fprintf(stdout,"yield-");
  if(!opt_sync) fprintf(stdout,"none,");
  else fprintf(stdout,"%c,",sync_type);
  
  
  fprintf(stdout,"%llu,",num_threads); // print number of threads
  fprintf(stdout,"%llu,",num_iterations); // print number of iterations
  fprintf(stdout,"%llu,",operations); // print number of operations
  fprintf(stdout,"%llu,",run_time); // print total run time
  fprintf(stdout,"%llu,",average_time_per_operation); // print average time per operation
  fprintf(stdout,"%llu\n",total); // counter total at the end of the run
  
  free(threadArr);
  exit(0);
}


void add(long long *pointer, long long value) 
{
  long long sum = *pointer + value;
  if(opt_yield)
    sched_yield();
  *pointer = sum;
}

void mutex_add(long long *pointer, long long value)
{
  pthread_mutex_lock(&count_mutex);

  long long sum = *pointer + value;
  if (opt_yield)
    sched_yield();
  *pointer = sum;

  pthread_mutex_unlock(&count_mutex);
}

void spin_add(long long *pointer, long long value)
{
  long long sum; 
  while (__sync_lock_test_and_set(&lock,1))
    { ; }
  sum = *pointer + value;
  if (opt_yield)
    sched_yield();
  *pointer = sum;
  
  __sync_lock_release(&lock);

  return;
}

void compare_and_swap_add(long long *pointer, long long value)
{
  long long prev;
  do
    {
      prev = counter;
      if (opt_yield)
	sched_yield();

    } while(__sync_val_compare_and_swap(&counter,prev,prev+value) != prev);
}

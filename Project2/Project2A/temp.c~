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

void sigHandler()
{
  fprintf(stderr, strerror(SIGSEGV));
  exit(4);
}

int main (int argc, char** argv)
{
  int opt;
  int input_fd;
  int output_fd;
  char buffer;
  int segfault = 0;
  int catch = 0;

  int this_option_optind = optind ? optind : 1;
  int option_index = 0;

  static struct option long_options[] =
      {
        {"input",    1, 0, 'i'},
        {"output",   1, 0, 'o'},
        {"segfault", 0, 0, 's'},
        {"catch",    0, 0, 'c'},
        {0, 0, 0, 0}
      };

  while (1) {
 
    opt = getopt_long(argc, argv, "", long_options, &option_index);

    if (opt == -1)
        break;

    switch(opt)
      {
      case 's':
        segfault = 1;
        break;
      case 'i':
	input_fd = open(optarg,O_RDONLY);
	if(input_fd == -1)
          { 
	    fprintf(stderr, "Error: %s\n", strerror(2));
	    exit(2);
	  }
	input_fd = dup2(input_fd,0);
	break;
      case 'o':
	output_fd = creat(optarg,0644);
	if(output_fd == -1)
	  {
	    fprintf(stderr,"Error: %s\n", strerror(3));
	    exit(3);
	  }
        output_fd = dup2(output_fd,1);
	break;
      case 'c':
	catch = 1;
	break;
      default:
	exit(1);
	break;
      }
  }
  
  if (catch)
    {
      signal(SIGSEGV, sigHandler);
    }

  if (segfault)
    {  
      char* segfault = NULL;
      *segfault = 'e';
    }


  while(!catch && read(0,&buffer,1))
  {
    write(1,&buffer,1);
  }
  close(1);
  exit(0);
  return 0;
}


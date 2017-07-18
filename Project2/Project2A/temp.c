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
        break;
      case 'i':
	break;
      case 'o':
	break;
      case 'c':
	break;
      default:
	exit(1);
	break;
      }
  }

  return 0;
}


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <getopt.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


/* Use this variable to remember original terminal attributes. */

struct termios saved_attributes;

//setting up terminal
void
reset_input_mode (void)
{
  tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
}

void
set_input_mode (void)
{
  struct termios tattr;
  char *name;

  /* Save the terminal attributes so we can restore them later. */
  tcgetattr (STDIN_FILENO, &saved_attributes);

  /* Set the funny terminal modes. */
  tcgetattr (STDIN_FILENO, &tattr);
  tattr.c_iflag=ISTRIP;
  tattr.c_oflag=0;
  tattr.c_lflag=0;
  tcsetattr (STDIN_FILENO, TCSANOW, &tattr);
}


int main (int argc, char** argv)
{
  set_input_mode ();

  int opt = 0; 
  int option_index = 0;

  static struct option long_options[] =
    {
      {"shell", 0, 0, 's'},
      {0, 0, 0, 0}
    };
 
  while(1)
    {  
      opt = getopt_long(argc, argv, "", long_options, &option_index);
      
      if (opt == 's')
	  break;
      else if (opt == -1)
	{
	  int notEOF = 1;
	  while (notEOF)
	    {
	      int counter = 0;
	      int i = 0;
	      char buff[1000];
	      counter = read (STDIN_FILENO, &buff, 1000);
	      for (i = 0; i < counter; i++)
		{
		  if (buff[i] == 0x0D || buff[i] == 0x0A)
		    {
		      write(1, "\r\n", 2);
		    }
		  else if (buff[i] == '\004')          /* C-d */
		    notEOF = 0;
		  else
		    write(1,buff,counter);
		}
	    }
	  reset_input_mode();
	  exit(0);
	}
      else 
	{
	  reset_input_mode();
	  exit(1);
	}
    }
  
  int to_child_pipe[2];
  int from_child_pipe[2];
  pid_t child_pid = -1;
  
  if (pipe(to_child_pipe) == -1)
    {
      fprintf(stderr, "pipe() failed!\n");
      exit(1);
    }
  if (pipe(from_child_pipe) == -1)
    {
      fprintf(stderr, "pipe() failed!\n");
    }
 
  struct pollfd fds[2];
  // polls[o] describes keyboard (stdin)
  // polls[1] describes the pipe that returns output from the shell
  fds[0].events = POLLIN | POLLHUP | POLLERR;
  fds[0].fd = STDIN_FILENO;

  fds[1].events = POLLIN | POLLHUP | POLLERR;
  fds[1].fd = from_child_pipe[0];
 
  child_pid = fork();

  if (child_pid > 0) // parent process
    {    
      int count = 0;
      
      int inShell = 1; 
      while(inShell)
	{
	  int value = poll(fds,2,0);
	  char buffer[1000];
	  if (fds[0].revents & POLLIN)
	    {    
	      close(to_child_pipe[0]);
	      close(from_child_pipe[1]);

	      count = read(STDIN_FILENO, buffer, 1000);
	      write(to_child_pipe[1], buffer, count);
	      count = read(from_child_pipe[0], buffer, 1000);
	      write(STDOUT_FILENO, buffer, count);
	    }
	}
    } 
  else if (child_pid == 0) // child process
    {
      close(to_child_pipe[1]);
      close(from_child_pipe[0]);
     
      int childInShell = 1;
         
      dup2(to_child_pipe[0], STDIN_FILENO);
      dup2(from_child_pipe[1], STDOUT_FILENO);
      
      close(to_child_pipe[0]);
      close(from_child_pipe[1]);
      
      char *execvp_argv[2];
      char execvp_filename[]="/bin/bash";
      execvp_argv[0] = execvp_filename;
      execvp_argv[1] = NULL;
      if (execvp(execvp_filename, execvp_argv) == -1)
	{
	  fprintf(stderr, "execvp() failed\n");
	  exit(1);
	}
    }
  else // fork failed
    {
      fprintf(stderr, "fork() failed\n");
      exit(1);
    }

  reset_input_mode();
  return 0;
}  


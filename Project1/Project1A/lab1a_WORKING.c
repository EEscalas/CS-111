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
  int debug = 0;
  int opt = 0; 
  int option_index = 0;

  static struct option long_options[] =
    {
      {"shell", 0, 0, 's'},
      {"debug", 0, 0, 'd'},
      {0, 0, 0, 0}
    };
 
  while(1)
    {  
      opt = getopt_long(argc, argv, "", long_options, &option_index);
      
      if (opt == 's')
	{
	  if (getopt_long(argc, argv, "", long_options, &option_index) == 'd')
	    debug = 1;
	  break;
	}
      else if (opt == -1)
	{
	  set_input_mode();
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
 
  if (debug)
    printf("this is the shell option\n");
  
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

      if (debug)
	printf("About to enter poll loop\n");
      
      int inShell = 1; 
      while(inShell)
	{
	  int value = poll(fds,2,0);
	  char buffer[1000];
	  int j= 0;
	  if (fds[0].revents & POLLIN)
	    {    
	      close(to_child_pipe[0]);
	      close(from_child_pipe[1]);

	      count = read(STDIN_FILENO, buffer, 1000);
	      for (j = 0; j < count; j++)
		{
		  if (buffer[j] == 0x0D)
		    {
		      buffer[j] == 0x0A;
		    }
		  if (buffer[j] == '\004')
                    {
                      close(to_child_pipe[0]);
		      close(to_child_pipe[1]);
		      close(from_child_pipe[0]);
		      close(from_child_pipe[1]);
                    }

		}
	      write(to_child_pipe[1], buffer, count);
	      count = read(from_child_pipe[0], buffer, 1000);
	      write(STDOUT_FILENO, buffer, count);
	    }
	  if (fds[0].revents & (POLLHUP+POLLERR))
	    {
	      fprintf(stderr, "error in shell");
	      break;
	    }
	}
    } 
  else if (child_pid == 0) // child process
    {
      if (debug)
	printf("In child process\n");
     
      int childInShell = 1;
      
      int valueChild = poll(fds, 2, 0);
  
      close(to_child_pipe[1]);
      close(from_child_pipe[0]);
      
      dup2(to_child_pipe[0], STDIN_FILENO);

      if (fds[1].revents & POLLIN)
	{
	  dup2(from_child_pipe[1], STDOUT_FILENO);
	  dup2(from_child_pipe[1], STDERR_FILENO);
	}     

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

      if (fds[1].revents & (POLLHUP+POLLERR))
	{
	  fprintf(stderr, "error in shell");
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


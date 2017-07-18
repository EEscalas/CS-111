//NAME: Elena Escalas
//EMAIL: elenaescalas@g.ucla.edu
//ID: 704560697

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

int shell_flag = 0;
pid_t child_pid = -1;


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

void sig_handler()
{
  reset_input_mode();
  int exit_status3 = 0;

  waitpid(child_pid,&exit_status3,0);
  printf("SHELL EXIT SIGNAL=%d ",WTERMSIG(exit_status3));
  printf("STATUS=%d\n", WEXITSTATUS(exit_status3));
 
  exit(0);
}

void sysCallError(char call)
{
  switch(call)
    {
    case 'r':
      fprintf(stderr, "Error: read system call failed\n");
      break;
    case 'w':
      fprintf(stderr, "Error: write system call failed\n");
      break;
    case 'd':
      fprintf(stderr, "Error: dup2 system call failed\n");
      break;
    case 'k':
      fprintf(stderr, "Error: kill system call failed\n");
      break;
    case 's':
      fprintf(stderr, "Error: signal system call failed\n");
      break;
    case 'a':
      fprintf(stderr, "Error: waitpid system call failed\n");
      break;
    default:
      break;
    }
  exit(1);
}

int main (int argc, char** argv)
{
  int debug = 0;
  int opt = 0; 
  int option_index = 0;
  int sysCallReturn = 0;

  static struct option long_options[] =
    {
      {"shell", 0, 0, 's'},
      {"debug", 0, 0, 'd'},
      {0, 0, 0, 0}
    };
 
  int isTrue = 1;
  while(isTrue)
    {  
      opt = getopt_long(argc, argv, "", long_options, &option_index);
      
      switch(opt)
	{
	case 's':
	  shell_flag = 1;
	  if (getopt_long(argc, argv, "", long_options, &option_index) == 'd')
	    debug = 1;
	  isTrue = 0;
	  break;
	case -1:
	  {
	    set_input_mode();
	    int notEOF = 1;
	    while (notEOF)
	      {
		int counter = 0;
		int i = 0;
		char buff[1000];
		counter = read (STDIN_FILENO, &buff, 1000);
		if (counter < 0) sysCallError('r');
		for (i = 0; i < counter; i++)
		  {
		    if (buff[i] == 0x0D || buff[i] == 0x0A)
		      {
			sysCallReturn = write(1, "\r\n", 2);
			if(sysCallReturn<0)sysCallError('w');
		      }
		    else if (buff[i] == '\004')          /* C-d */
		      notEOF = 0;
		    else
		      {  
			sysCallReturn = write(1,buff,counter);
			if (sysCallReturn < 0) sysCallError('w');
		      }
		  }
	      }
	    reset_input_mode();
	    exit(0);
	  }
	  break;
	default:
	  fprintf(stderr, "Usage: [--shell]\n");
	  exit(1);	
	}
    }
 
  if (shell_flag)
    {
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
      
      set_input_mode();
      struct pollfd fds[2];
      // polls[o] describes keyboard (stdin)
      // polls[1] describes the pipe that returns output from the shell
      fds[0].events = POLLIN | POLLHUP | POLLERR;
      fds[0].fd = STDIN_FILENO;
      
      fds[1].events = POLLIN | POLLHUP | POLLERR;
      fds[1].fd = from_child_pipe[0];
      signal(SIGPIPE, sig_handler);
      
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
		  if (count < 0) sysCallError('r');
		  
		  for (j = 0; j < count; j++)
		    {
		      if (buffer[j] == 0x0D || buffer[j] == 0x0A)
			{
			  sysCallReturn = write(1, "\r\n", 2);
			  if (sysCallReturn < 0) sysCallError('w');
			  buffer[j] = '\n';
			}
		      else if(buffer[j] == 0x03)
			{
			  sysCallReturn = kill(child_pid,SIGINT);
			  if (sysCallReturn < 0) sysCallError('k');
			  
			  int exit_status2 = 0;
			  
			  reset_input_mode();
			  
			  sysCallReturn = waitpid(child_pid,&exit_status2,0);
			  if (sysCallReturn < 0) sysCallError('a');
			  printf("SHELL EXIT SIGNAL=%d ",WTERMSIG(exit_status2));
			  printf("STATUS=%d\n", WEXITSTATUS(exit_status2));
			  
			  exit(0);
			}
		      else if (buffer[j] == '\004')          /* C-d */
			{      		      
			  close(to_child_pipe[0]);
			  close(to_child_pipe[1]);
			  
			  while((count = read(from_child_pipe[0], buffer, 1000)) != 0)
			    {
			      int num = 0;
			      for(num = 0; num < count; num++)
				{
				  if (buffer[num] == '\n')
				    {
				      sysCallReturn = write(1, "\r\n", 2);
				      if (sysCallReturn < 0) sysCallError('w');
				      continue;
				    }
				  sysCallReturn = write(STDOUT_FILENO, &buffer[num], 1);
				  if (sysCallReturn < 0) sysCallError('w');
				}
			    }
			  
			  reset_input_mode();
			  close(from_child_pipe[0]);
			  close(from_child_pipe[1]);
			  
			  int exit_status = 0;
			  
			  sysCallReturn = waitpid(child_pid,&exit_status,0);
			  if (sysCallReturn < 0) sysCallError('a');
			  printf("SHELL EXIT SIGNAL=%d ",WTERMSIG(exit_status));
			  printf("STATUS=%d\n", WEXITSTATUS(exit_status));
			  
			  exit(0);	      
			}
		      else
			{
			  char c = buffer[j];
			  sysCallReturn = write(1, &c, 1);
			  if (sysCallReturn < 0)sysCallError('w');
			}
		    }
		  write(to_child_pipe[1], buffer, count);
		}
	      if(fds[1].revents & POLLIN)
		{
		  count = read(from_child_pipe[0], buffer, 1000);
		  if (count < 0) sysCallError('r');
		  int num = 0;
		  for(num = 0; num < count; num++)
		    {
		      if (buffer[num] == '\004')
			{     	      
			  close(from_child_pipe[0]);
			  sysCallReturn = close(from_child_pipe[1]);
			  
			  reset_input_mode();
			  exit(0);
			}
		      if (buffer[num] == '\n')
			{
			  sysCallReturn = write(1, "\r\n", 2);
			  if (sysCallReturn < 0) sysCallError('w');
			  continue;
			}
		      sysCallReturn = write(STDOUT_FILENO, &buffer[num], 1);
		      if (sysCallReturn < 0) sysCallError('w');
		    }
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
	  
	  close(to_child_pipe[1]);
	  close(from_child_pipe[0]);
	  
	  sysCallReturn = dup2(to_child_pipe[0], STDIN_FILENO);
	  if (sysCallReturn < 0) sysCallError('d');
	  sysCallReturn = dup2(from_child_pipe[1], STDOUT_FILENO);
	  if (sysCallReturn < 0) sysCallError('d');
	  sysCallReturn = dup2(from_child_pipe[1], STDERR_FILENO);
	  if (sysCallReturn < 0) sysCallError('d');
	  
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
      
    }
  return 0;
}  


//NAME: Elena Escalas
//EMAIL: elenaescalas@g.ucla.edu
//ID: 704560697

//SERVER

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <poll.h>
#include <mcrypt.h>

pid_t child_pid = -1;
int encrypted = 0;
int key_len = 16;

char* IV = "ELENALUCIESCALAS";
char* key;

MCRYPT td;
MCRYPT td2;


void encrypt_init()
{
  if (key == NULL)
    fprintf(stderr, "ERROR: Invalid encryption key\n");

  td = mcrypt_module_open("twofish",NULL,"cfb",NULL);
  mcrypt_generic_init(td, key, key_len, IV);
}

void encrypt (char* charBuf, int buffer_len)
{
  void* buffer = (void*)charBuf;
  int key_len = 16;
  if (key == NULL)
    fprintf(stderr, "ERROR: Invalid encryption key\n");

  /*  int blocksize = mcrypt_enc_get_block_size(td);
  if (buffer_len % blocksize != 0) fprintf(stderr, "ERROR: Invalid encryption block size\n");
  */
  mcrypt_generic(td, buffer, buffer_len);
}

void encrypt_deinit()
{
  mcrypt_generic_deinit(td);
  mcrypt_module_close(td);
}

void decrypt_init()
{
  if (key == NULL)
    fprintf(stderr, "ERROR: Invalid encryption key\n");

  td2 = mcrypt_module_open("twofish",NULL,"cfb",NULL);
  mcrypt_generic_init(td2,key,key_len,IV);

}

void decrypt (char* charBuf, int buffer_len)
{
  void* buffer = (void*)charBuf;
  int key_len = 16;
  if (key == NULL)
    fprintf(stderr, "ERROR: Invalid encryption key\n");
  
  /*int blocksize = mcrypt_enc_get_block_size(td2);
  if (buffer_len % blocksize != 0) 
    fprintf(stderr, "ERROR: Invalid encryption block size\n");
  */
  mdecrypt_generic(td,buffer,buffer_len);
}

void decrypt_deinit()
{
  mcrypt_generic_deinit(td2);
  mcrypt_module_close(td2);
}

void sig_handler()
{
  int exit_status3 = 0;
  
  waitpid(child_pid,&exit_status3,0);
  printf("SHELL EXIT SIGNAL=%d ",WTERMSIG(exit_status3));
  printf("STATUS=%d\n", WEXITSTATUS(exit_status3));
  
  exit(0);
}

void sysCallError(char call);

void socketToShellToSocket(int socket_fd);

int serverOutput(char* portNum) {
  int sockfd, newsockfd, portno, clilen;
  char buffer[1000];
  char bufferToSocket[1000];
  struct sockaddr_in serv_addr, cli_addr;
  int  n;
  
  /* First call to socket() function */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }
  
  /* Initialize socket structure */
  memset((char *) &serv_addr, 0, sizeof(serv_addr));
  portno = atoi(portNum);
  
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  
  /* Now bind the host address using bind() call.*/
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR on binding");
    exit(1);
  }
  
  /* Now start listening for the clients, here process will
   * go in sleep mode and will wait for the incoming connection
   */
  
  listen(sockfd,5);
  clilen = sizeof(cli_addr);


  /* Accept actual connection from the client */
  newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
  
  if (newsockfd < 0) {
    perror("ERROR on accept");
    exit(1);
  }

   
  /* If connection is established then start communicating */

  socketToShellToSocket(newsockfd);

  if (n < 0) {
    perror("ERROR reading from socket");
    exit(1);
  }
  
  if (n < 0) {
    perror("ERROR reading from socket");
    exit(1);
  }
  
  return 0;
}

int main( int argc, char *argv[] ) 
{
  static struct option long_options[] =
    {
      {"port", 1, 0, 'p'},
      {"encrypt", 1, 0, 'e'},
      {0, 0, 0, 0}
    };
  
  key = malloc(16*sizeof(char));
  int pflag = 0;
  int input_fd;
  char* portNumber;
  char opt;
  int option_index = 0;
  char* keyFilename;

  signal(SIGPIPE, SIG_IGN);
  
  while(1)
    {
      opt = getopt_long(argc, argv, "", long_options, &option_index);

      if (opt == -1)
	break;
      switch(opt)
	{
	case 'p':
	  pflag = 1;
	  portNumber = optarg;
	  break;
        case 'e':
	  encrypted = 1;
	  keyFilename = optarg;
	  break;
	default:
	  exit(1);
	}
    }
  
  if (encrypted)
    {
      int keyfile_fd = open(keyFilename,O_RDONLY);
      read(keyfile_fd,key,16);
      decrypt_init();
      encrypt_init();
    }

  if (pflag)
    serverOutput(portNumber);

  if (encrypted)
    {
      decrypt_deinit();
      encrypt_deinit();
    }

  return 0;
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


void socketToShellToSocket(int socket_fd)
{
  int to_child_pipe[2];
  int from_child_pipe[2];
  int sysCallReturn = 1;

  if (pipe(to_child_pipe) == -1)
    {
      fprintf(stderr, "pipe() failed!\n");
      exit(1);
    }
  if (pipe(from_child_pipe) == -1)
    {
      fprintf(stderr, "pipe failed!\n");
      exit(1);
    }

  struct pollfd fds[2];
  
  //create pollfd to wait for info from socket
  fds[0].events = POLLIN | POLLHUP | POLLERR;
  fds[0].fd = socket_fd;

  //create pollfd to wait for info back from shell
  fds[1].events = POLLIN | POLLHUP | POLLERR;
  fds[1].fd = from_child_pipe[0];
  
  //check for signal pipe error
  signal(SIGPIPE, sig_handler);
  
  //fork the process
  child_pid = fork();
  
  //enter parent process
  if(child_pid > 0)
    {
      int count = 0;
      
      int inShell = 1;
      while(poll(fds,2,-1))
	{
	  int value = poll(fds,2,0);
	  char buffer[1000];
	  int j = 0;
	  if (fds[0].revents & POLLIN)
	    {      
	      //close the unused directions of the pipes
	      close(to_child_pipe[0]);
	      close(from_child_pipe[1]);
	      
	      count = read(socket_fd,buffer,1000);
	      if (count < 0) sysCallError('r');

	      //decrypt
	      for (j = 0; j < count; j++)
		{
		  if (encrypted)
		    decrypt(&buffer[j],1);
		}
	    

	      for (j = 0; j < count; j++)
		{
		  //account for \r and \n  
		  if (buffer[j] == 0x0D || buffer[j] == 0x0A)
		    {
		      buffer[j] = '\n';
		    }
		  else if(buffer[j] == 0x03) // account for ^C
		    {
		      sysCallReturn = kill(child_pid,SIGINT);
		      if (sysCallReturn < 0) sysCallError('k');
		      
		      int exit_status2 = 0;
		      
		      sysCallReturn = waitpid(child_pid,&exit_status2,0);
		      if (sysCallReturn < 0) sysCallError('a');
		      printf("SHELL EXIT SIGNAL=%d ",WTERMSIG(exit_status2));
		      printf("STATUS=%d\n", WEXITSTATUS(exit_status2));
		      
		      char ex = '\004';
		      if (encrypted) encrypt(&ex,1);
		      write(socket_fd,&ex,1);
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
			      if (buffer[num] == '\n' || buffer[num] == '\r')
				{
				  char cr = '\r';
				  char lf = '\n';
				  if (encrypted) encrypt(&cr,1);
				  write(socket_fd, &cr, 1);
				  if (encrypted) encrypt(&lf,1);
				  write(socket_fd, &lf, 1);
				}
			      
			      //encrypt
			      if (encrypted)
				{
				  encrypt(&buffer[num],1);
				}
			      //write remains of pipe from shell to the socket
			      sysCallReturn = write(socket_fd, &buffer[num], 1);
			      if (sysCallReturn < 0) sysCallError('w');
			    }
			}

		      close(from_child_pipe[0]);
		      close(from_child_pipe[1]);

		      int exit_status = 0;

		      sysCallReturn = waitpid(child_pid,&exit_status,0);
		      if (sysCallReturn < 0) sysCallError('a');
		      printf("SHELL EXIT SIGNAL=%d ",WTERMSIG(exit_status));
		      printf("STATUS=%d\n", WEXITSTATUS(exit_status));

		      exit(0);
		    }
		}
	      write(to_child_pipe[1], buffer, count);
	    } 
	  if (fds[1].revents & POLLIN)
	    {
	      count = read(from_child_pipe[0], buffer, 1000);
	      if (count < 0) sysCallError('r');
	      int num = 0;
	      for(num = 0; num < count; num++)
		{
		  if (buffer[num] == '\004')
		    {
		      close(from_child_pipe[0]);
		      close(from_child_pipe[1]);
		      
		      //encrypt
		      if (encrypted) encrypt(&buffer[num],1);
		      
		      write(socket_fd,&buffer[num],1);
		      exit(0);
		    }
		  if (buffer[num] == '\n')
		    {
		      char lf = '\n';
		      char cr = '\r';
		      //encrypt
		      if(encrypted)encrypt(&cr, 1);
		      sysCallReturn = write(socket_fd, &cr, 1);
		      if (sysCallReturn < 0) sysCallError('w');
		    }
		  
		  //encrypt
		  if (encrypted)encrypt(&buffer[num],1);
		  sysCallReturn = write(socket_fd, &buffer[num], 1);
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
      else if (child_pid == 0)
	{
          int childInShell = 1;

          close(to_child_pipe[1]);
          close(from_child_pipe[0]);

          sysCallReturn = dup2(to_child_pipe[0], STDIN_FILENO);
          if (sysCallReturn < 0) sysCallError('d');
          sysCallReturn = dup2(from_child_pipe[1],STDOUT_FILENO);
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
  return;
}







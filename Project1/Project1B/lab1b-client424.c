//NAME: Elena Escalas
//EMAIL: elenaescalas@g.ucla.edu
//ID: 704560697

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
#include <termios.h>
#include <poll.h>

int encrypted = 0;

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


int clientToServer(char *portnum) {
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
   
  char buffer[1000];
   
  /* if (argc < 2) {
    fprintf(stderr,"usage %s port\n", argv[0]);
    exit(0);
    }*/
  
  portno = atoi(portnum);
   
  /* Create a socket point */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  server = gethostbyname("127.0.0.1");
   
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }
   
  memset((char *) &serv_addr,0,sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  memcpy((char *) &serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
  serv_addr.sin_port = htons(portno);
   
  /* Now connect to the server */
  if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR connecting");
    exit(1);
  }
   
  /* Now ask for a message from the user, this message
   * will be read by server
   */

  int count;
  int j = 0;
  int inLoop = 1;
  char bufferToSocket[1000];
  int outputSize = 0;
  

  struct pollfd fds[2];
  fds[1].events = POLLIN | POLLHUP | POLLERR;
  fds[1].fd = STDIN_FILENO;
  fds[0].events = POLLIN | POLLHUP | POLLERR;
  fds[0].fd = sockfd;
  
  int count2 = 0;
  int num = 0;

  while(poll(fds,2,-1))
    {
      if (fds[1].revents & POLLIN)
	{ 
	  count = read(STDIN_FILENO, buffer, 1000);
	  
	  for (j = 0; j < count; j++)
	    {
	      if (buffer[j] == '\n' || buffer[j] == '\r')
		{
		  write(STDOUT_FILENO, "\r\n",2);
		  write(sockfd,"\r\n",2);
		}
	      bufferToSocket[j] = buffer[j];
	      write(STDOUT_FILENO, &buffer[j], 1);
	      n = write(sockfd,&buffer[j],1); //send message to server
	      if (n < 0) perror("ERROR writing to socket");
	    }
	}
      if (fds[0].revents & POLLIN)
	{
	  count2 = read(sockfd,buffer,1000);
	  for (num = 0; num < count2; num++)
	    {
	      if (buffer[num] == '\004')
		{
		  close(sockfd);
		  reset_input_mode();
		  exit(0);
		}
	      write(STDOUT_FILENO,&buffer[num],1);
	    }
	}      
    }

  if (n < 0) {
    perror("ERROR writing to socket");
    exit(1);
  }

  if (n < 0) {
    perror("ERROR reading from socket");
    exit(1);
  }

  return 0;
}


int main(int argc, char *argv[]) 
{
  static struct option long_options[] =
     {
       {"port", 1, 0, 'p'},
       {"encrypt", 1, 0, 'e'},
       {0, 0, 0, 0}
     };

  int pflag = 0;
  int input_fd;
  char* portNumber;
  char opt;
  int option_index = 0;
  
  while(1)
    {
      opt = getopt_long(argc, argv, "", long_options, &option_index);
      
      switch(opt)
	{
	case 'p':
	  pflag = 1;
	  portNumber = strdup(optarg);
	  break;
	case 'e':
	  encrypted = 1;
	  break;
	case -1:
	  exit(1);
	}
      break;
    }

  if (pflag)
    {
      set_input_mode();
      clientToServer(portNumber);
      reset_input_mode();
    }

  return 0;
}

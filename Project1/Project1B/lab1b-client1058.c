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
  
  /*printf("Please enter the message: ");
  memset(buffer,0,256);
  fgets(buffer,255,stdin);*/
  int count;
  int j = 0;
  int inLoop = 1;
  char bufferToSocket[1000];
  int outputSize = 0;
  
  while(inLoop)
    { 
      count = read(STDIN_FILENO, buffer, 1000);

      for (j = 0; j < count; j++)
	{
	  if (buffer[j] == '\n' || buffer[j] == '\r')
	    {
	      write(STDOUT_FILENO, "\r\n",2);
	      //bufferToSocket[j] = '\n';
	      //outputSize++;
	      write(sockfd,"\r\n",2);
	      inLoop = 0;
	      return 0;
	    }
	  bufferToSocket[j] = buffer[j];
	  outputSize++;
	  write(STDOUT_FILENO, &buffer[j], 1);
	  write(sockfd,&buffer[j],1);
	}
      //write(sockfd,&bufferToSocket,1000);
    }

  /* Send message to the server */
  //n = write(sockfd, buffer, strlen(buffer));
   
  if (n < 0) {
    perror("ERROR writing to socket");
    exit(1);
  }
   
  /* Now read server response */
  memset(buffer,0,256);
  n = read(sockfd, buffer, 255);
   
  if (n < 0) {
    perror("ERROR reading from socket");
    exit(1);
  }
  
  printf("%s\n",buffer);
  return 0;
}


int main(int argc, char *argv[]) 
{
  static struct option long_options[] =
     {
       {"port", 1, 0, 'p'},
       {"debug", 0, 0, 'd'},
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

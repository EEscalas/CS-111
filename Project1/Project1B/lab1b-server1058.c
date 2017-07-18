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

int serverOutput(char* portNum ) {
  int sockfd, newsockfd, portno, clilen;
  char buffer[256];
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
  memset(buffer,0,256);
  int bufLoc = 0;
  while( bufLoc < 256)
    {
      n = read(newsockfd,buffer,1);
      if (n == 0) break;
      else write(STDOUT_FILENO, &buffer[bufLoc], 1);
    }
   
  if (n < 0) {
    perror("ERROR reading from socket");
    exit(1);
  }
   
  /* Write a response to the client */
  n = write(newsockfd,"I got your message",18);
   
  if (n < 0) {
    perror("ERROR writing to socket");
    exit(1);
  }
      
  return 0;
}

int main( int argc, char *argv[] ) 
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
	   portNumber = optarg;
	   serverOutput(portNumber);
	   break;
	 }
       break;
     }
   return 0;
}

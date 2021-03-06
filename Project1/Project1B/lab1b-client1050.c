//NAME: Elena Escalas
//EMAIL: elenaescalas@g.ucla.edu
//ID: 704560697

//CLIENT

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
#include <mcrypt.h>

int logging = 0;
int logfile_fd = 0;

int encrypted = 0;
int key_len = 16;

MCRYPT td;
MCRYPT td2;

char* IV = "ELENALUCIESCALAS";
char* key;

void encrypt_init ()
{
  if (key == NULL)
    fprintf(stderr,"ERROR: Invalid encryption key\n");

  td = mcrypt_module_open("twofish",NULL,"cfb",NULL);
  mcrypt_generic_init(td, key, key_len, IV);
}

void encrypt (char* charBuf, int buffer_len)
{ 
  void* buffer = (void*)charBuf;
  if (key == NULL)
    fprintf(stderr,"ERROR: Invalid encryption key\n");
  
  /*int blocksize = mcrypt_enc_get_block_size(td);
  if (buffer_len % blocksize != 0) fprintf(stderr,"ERROR: Invalid encryption block size\n");  
  */
  mcrypt_generic(td, buffer, buffer_len);
}

void encrypt_deinit()
{
  mcrypt_generic_deinit(td);
  mcrypt_module_close(td);
}

void decrypt_init ()
{
  if (key == NULL)
    fprintf(stderr, "ERROR: Invalid encryption key\n");

  td2 = mcrypt_module_open("twofish",NULL,"cfb",NULL);
  mcrypt_generic_init(td2, key, key_len, IV);
}

void decrypt (char* charBuf, int buffer_len)
{
  void* buffer = (void*)charBuf;
  if (key == NULL)
    fprintf(stderr,"ERROR: Invalid encryption key\n");
  
  /*int blocksize = mcrypt_enc_get_block_size(td2); 
  if (buffer_len % blocksize != 0) fprintf(stderr,"ERROR: Invalid encryption block size\n");
  */
  mdecrypt_generic(td2,buffer,buffer_len);
}

void decrypt_deinit()
{
  mcrypt_generic_deinit(td2);
  mcrypt_module_close(td2);
}

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
    reset_input_mode();
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
		  char* crlf = "\r\n";
		  write(STDOUT_FILENO,crlf,2);
		  //encrypt, log, and write to socket
		  char lf = '\n';
		  char cr = '\r';
		  if (logging)
		    {
		      dprintf(logfile_fd, "SENT 1 bytes: %c\n",cr);
		    }
		  if (encrypted)
		    {
		      encrypt(&cr,1);
		      write(sockfd,&cr,1);
		      encrypt(&lf,1);
		      write(sockfd,&lf,1);
		    }
		  else
		    {
		      write(sockfd,&cr,1);
		      write(sockfd,&lf,1);
		    }
		}
	      bufferToSocket[j] = buffer[j];
	      write(STDOUT_FILENO, &buffer[j], 1);
	      // encrypt, log, and write to socket
	      if (encrypted) encrypt(&buffer[j],1);
              if (logging)
                {
                  if (buffer[j] != '\n')
                    dprintf(logfile_fd, "SENT 1 bytes: %c\n", buffer[j]);
                }

	      n = write(sockfd,&buffer[j],1); 
	      if (n < 0) perror("ERROR writing to socket");
	    }
	}
      if (fds[0].revents & POLLIN)
	{	 
	  count2 = read(sockfd,buffer,1000);
	  
	  //log and decrypt
	  int byte = 0;
	  for (byte = 0; byte < count2; byte++)
	    {
	      if (logging && buffer[byte] != '\n')
		{
		  dprintf(logfile_fd, "RECEIVED 1 bytes: %c\n", buffer[byte]);
		}
	      if (encrypted)
		{
		  decrypt(&buffer[byte], 1);
		}
	    }

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
       {"log", 1, 0, 'l'},
       {0, 0, 0, 0}
     };

  int pflag = 0;
  int input_fd;
  char* portNumber;
  int opt;
  int option_index = 0;
  char* keyFilename;
  char* logFilename;
  key = malloc(16*sizeof(char));

  while(1)
    {
      opt = getopt_long(argc, argv, "", long_options, &option_index);

      if (opt == -1)
	break;
      
      switch(opt)
	{
	case 'p':
	  pflag = 1;
	  portNumber = strdup(optarg);
	  break;
	case 'e':
	  encrypted = 1;
	  keyFilename = optarg;
	  break;
	case 'l':
	  logging = 1;
	  logFilename = optarg;
	  break;
	}
    }

  if (encrypted)
    {
      int keyfile_fd = open(keyFilename,O_RDONLY);
      read(keyfile_fd,key,16);
      decrypt_init();
      encrypt_init();
    }
  
  if(logging)
    {
      logfile_fd = creat(logFilename, S_IRWXU);
      fprintf(stdout, "%i",logfile_fd);
    }
  
  if (pflag)
    {
      set_input_mode();
      clientToServer(portNumber);
      reset_input_mode();
    }

  if (encrypted)
    {
      decrypt_deinit();
      encrypt_deinit();
    }

  return 0;
}

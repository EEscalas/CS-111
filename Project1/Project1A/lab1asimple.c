
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <getopt.h>

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
  tattr.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
  tattr.c_cc[VMIN] = 1;
  tattr.c_cc[VTIME] = 0;
  tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}


int
main (int argc, char** argv)
{
  char c;

  set_input_mode ();

  int opt = 0; 
  int option_index = 0;

  static struct option long_options[] =
    {
      {"shell", 0, 0, 's'},
      {0, 0, 0, 0}
    };
  
  while (1) {
    opt = getopt_long(argc, argv, "", long_options, &option_index);

    if (opt == -1)
      break;
  }

  while (1)
    {
      read (STDIN_FILENO, &c, 1);
      // if (c == 0x0D || c == 0x0A)
      //{
      //  write(1, "\r\n", 2);
      //}
      if (c == '\004')          /* C-d */
        break;
      else
        write(1,&c,1);
    }

  return EXIT_SUCCESS;
}

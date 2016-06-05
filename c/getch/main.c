/* Experimental program that configures stdin as a raw-input device. */

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  struct termios org, new;
  tcgetattr(STDIN_FILENO, &org);
  new = org;
  new.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new);

  int fd = open("/dev/stdin");
  char c;
  do {
    fprintf(stderr, "Type a character: ");
    read(fd, &c, 1);
    fprintf(stderr, "You typed '%c'.\n", c);
  } while(c != 'q');

  close(fd);
  tcsetattr(STDIN_FILENO, TCSANOW, &org);
  return 0;
}


/* A simple class that configures the console device as a raw device so a
 * program can read keyboard input one character at a time as it is being
 * entered by the user.  It makes for better user interaction.
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include "raw_stdin.h"

struct raw_stdin {
  int fd;
  struct termios org;
  struct termios new;
};

raw_stdin *raw_stdin_new(void)
{
  raw_stdin *this = malloc(sizeof(*this));

  /* Put stdin into raw mode so we can read keyboard input in realtime. */
  tcgetattr(STDIN_FILENO, &this->org);
  this->new = this->org;
  this->new.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &this->new);
  this->fd = open("/dev/stdin", O_RDONLY);

  return this;
}

const char
raw_stdin_getchar(raw_stdin *this)
{
  char c;
  read(this->fd, &c, 1);
  return c;
}

void
raw_stdin_free(raw_stdin *this)
{
  close(this->fd);
  tcsetattr(STDIN_FILENO, TCSANOW, &this->org);
  free(this);
}


/* A simple class that configures the console device as a raw device so a
 * program can read keyboard input one character at a time as it is being
 * entered by the user.  It makes for better user interaction.
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include "common.h"

#include "raw_stdin.h"

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/* This is the raw_stdin class. */
struct raw_stdin {
  int fd;
  struct termios org;
  struct termios new;
};

/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

/******************************************************************************
 ********************************* PUBLIC API *********************************
 *****************************************************************************/

/* Create a new raw_stdin object.  This object can be used to access the
 * raw_stdin class.
 *
 * Input:
 *   N/A.
 *
 * Output:
 *   Returns a pointer to the object.
 *   Returns 0 if unable to create the object.
 */
raw_stdin *
raw_stdin_new(void)
{
  raw_stdin *this = malloc(sizeof(*this));

  if(this != (raw_stdin *) 0)
  {
    /* Put stdin into raw mode so we can read keyboard input in realtime. */
    tcgetattr(STDIN_FILENO, &this->org);
    this->new = this->org;
    this->new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &this->new);
    if((this->fd = open("/dev/stdin", O_RDONLY)) == -1)
    {
      raw_stdin_delete(this);
      this = (raw_stdin *) 0;
    }
  }

  return this;
}

/* Delete a console object that was created by raw_stdin_new().
 *
 * Input:
 *   this = A pointer to the raw_stdin object.
 *
 * Output:
 *   true  = success.  this is deleted.
 *   false = failure.  this is undefined.
 */
bool
raw_stdin_delete(raw_stdin *this)
{
  bool retcode = false;

  if(this != (raw_stdin *) 0)
  {
    if(this->fd >= 0)
    {
      close(this->fd);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &this->org);

    free(this);
    retcode = true;
  }

  return retcode;
}

/* Get a single character from the console.
 *
 * Input:
 *   this = A pointer to the raw_stdin object.
 *
 *   c    = A pointer to the location to store the character in.
 *
 * Output:
 *   true  = success.  *c contains the character.
 *   false = failure.  *c contains a NULL (if it is a valid pointer).
 */
bool
raw_stdin_getchar(raw_stdin *this,
                  char *c)
{
  bool retcode = false;

  if(c != (char *) 0)
  {
    /* Initialize *c.  If we fail, then it returns a NULL. */
    *c = 0;

    /* Read the data. */
    if(this != (raw_stdin *) 0)
    {
      retcode = (read(this->fd, c, 1) == 1);
    }
  }

  return retcode;
}

/******************************************************************************
 ********************************** TEST API **********************************
 *****************************************************************************/

#if defined(TEST)
bool
raw_stdin_test(void)
{
  bool retcode = true;

  return retcode;
}
#endif // TEST


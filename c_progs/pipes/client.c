
/*******************************************************************************
 * This program is the client end of a named pipe test.
 ******************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "pipe_test.h"

int main(int argc, char **argv)
{
  if(make_pipe_node() == true) {

    int fd = create_pipe(false);
    if(fd != -1) {

      int i = 0;

      /* Run the test.  Loop here and receive data. */
      while(1) {

        /* You can play around with the usleep() value in order to cause
         * overflows in the pipe. */
        usleep(100000);

        char msg[1024];
	int bytes_read = read_from_pipe(fd, msg, sizeof(msg));
        if(bytes_read > 0) {
          msg[bytes_read] = 0;
          printf("%s\n", msg);
        }
      }
    }
  }

  /* If we reach this point, then the program has failed. */
  return 1;
}


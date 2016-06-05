
/*******************************************************************************
 * This program is the server end of a named pipe test.
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

    int fd = create_pipe(true);
    if(fd != -1) {

      int i = 0;

      /* Run the test.  Loop here and pass data. */
      while(1) {

        /* You can play around with the usleep() value in order to cause
         * overflows in the pipe. */
        usleep(1000000);

        char msg[1024];
        snprintf(msg, sizeof(msg) - 1, "%7d: Here is a message.", ++i);
        size_t msg_len = strlen(msg);

        int bytes_written = write_to_pipe(fd, msg, msg_len);
        printf("write(%s) returned %d.\n", msg, bytes_written);
      }
    }
  }

  /* If we reach this point, then the program has failed. */
  return 1;
}


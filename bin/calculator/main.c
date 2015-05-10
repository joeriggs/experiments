/* This is the main program entrypoint for a simple math calculator.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include "common.h"

#include "calculator.h"
#include "raw_stdin.h"

int main(int argc, char **argv)
{
  /* Create an object that will allow us to read raw console input. */
  raw_stdin *console = raw_stdin_new();
  if(console != (raw_stdin *) 0)
  {
    /* This will hold the calculator object.  We create it inside the while()
     * loop because we will need to kill and recreate it if an internal error
     * occurs (such as a divide by zero). */
    calculator *calc = (calculator *) 0;

    fprintf(stderr, "Type an equation.\n");
    fprintf(stderr, "Type 'q' to quit.\n");
    fprintf(stderr, "-->                                <--\b\b\b\b");
    bool done = false;

    while(done == false)
    {
      /* Create a calculator object if one doesn't currently exist.  Note that
       * we delete the calculator object if an error occurs, but we don't
       * terminate the calculator program.  We recover and continue. */
      if( (calc == (calculator *) 0) && ((calc = calculator_new()) == (calculator *) 0) )
      {
        /* If we fail to create a calculator object, loop around and try again.
         * Don't ever fail and give up. */
        continue;
      }

      char c = raw_stdin_getchar(console);
      switch(c)
      {
      case 'q':
        done = 1;
        break;

      default:
        {
          char   calc_buf[33];
          size_t calc_buf_size = sizeof(calc_buf);
          if(calculator_add_char(calc, c) == false)
          {
            snprintf(calc_buf, (sizeof(calc_buf) - 1), "ERROR");

            /* If the calculator object fails for any reason, delete it and
             * create a new one.  The most likely cause of failure would be a
             * divide by zero. */
            calculator_delete(calc);
            calc = (calculator *) 0;
          }
          else
          {
            calculator_get_console(calc, calc_buf, sizeof(calc_buf));
          }

          char buf2[33];
          if(strlen(calc_buf) < (calc_buf_size - 1))
          {
            memset(buf2, ' ', sizeof(buf2));
            buf2[sizeof(buf2) - 1] = 0;
            memcpy(buf2 + (sizeof(buf2) - 1 - strlen(calc_buf)), calc_buf, strlen(calc_buf));
          }
          else
          {
            memcpy(buf2, calc_buf, sizeof(calc_buf));
          }

          char buf3[64];
          snprintf(buf3, sizeof(buf3), "\r-->%s<--\b\b\b\b", buf2);
          fprintf(stderr, buf3);
        }
        break;
      }
    }

    if(calc != (calculator *) 0)
    {
      calculator_delete(calc);
    }


    raw_stdin_free(console);
    printf("Bye.\n");
  }

  else
  {
    fprintf(stderr, "Unable to create raw console object.\n");
    return 1;
  }
  return 0;
}


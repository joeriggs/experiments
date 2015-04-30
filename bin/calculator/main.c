/* This is the main program entrypoint for a simple math calculator.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include "calculator.h"
#include "raw_stdin.h"

int main(int argc, char **argv)
{
  /* Create an object that will allow us to read raw console input. */
  raw_stdin *console = raw_stdin_new();
  if(console == (raw_stdin *) 0)
  {
    fprintf(stderr, "Unable to create raw console object.\n");
    return 1;
  }

  /* Create a calculator object. */
  calculator *calc = calculator_new();
  if(calc == (calculator *) 0)
  {
    fprintf(stderr, "Unable to create calculator object.\n");
    return 1;
  }

  fprintf(stderr, "Type an equation.\n");
  fprintf(stderr, "Type 'q' to quit.\n");
  int done = 0;

  while(!done) {
    char c = raw_stdin_getchar(console);
    switch(c)
    {
    case 'q':
      done = 1;
      break;

    default:
      {
        const char *str = calculator_add_char(calc, c);
        if(str != (const char *) 0)
        {
          fprintf(stderr, "\r%s %c", str, 0x08);
        }
      }
      break;
    }
  };

  raw_stdin_free(console);
  return 0;
}


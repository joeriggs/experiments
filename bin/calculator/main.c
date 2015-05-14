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

/* This is the help message.
 *
 * Input:
 *   N/A.
 *
 * Output:
 *   N/A.
 */
static void
display_help(void)
{
  fprintf(stderr, "\n"
    "This is a simple text-based math calculator.\n"
    "\n"
    "The commands are:\n"
    " h - Display this help message.\n"
    " q - Quit the program.\n"
    " m - Toggle Decimal and Hexadecimal mode.\n"
    "\n"
    "The supported operators are:\n"
    " + - Addition\n"
    " - - Subtraction\n"
    " * - Multiplication\n"
    " / - Division\n"
    " ^ - Exponentiation.  Right to left associative.  2^3^3 = 2^(3^3).\n"
    "\n"
    "Operators that are only supported for Hexadecimal are:\n"
    " & - Bitwise AND\n"
    " | - Bitwise OR\n"
    " x - Bitwise XOR\n"
    " ~ - Bitwise NOT (1's complement).  This is a UNARY operator.\n"
    "\n"
  );
}

/* This function updates the calculator display.
 *
 * Input:
 *   this = A pointer to the calculator object.
 *
 * Output:
 *   true  = Success.  The calculator display is updated.
 *   false = Failure.  The calculator display is NOT updated.
 */
static bool
display_calc(calculator *calc)
{
  bool retcode = false;

  char   calc_obj_buf[33];
  size_t calc_obj_buf_size = sizeof(calc_obj_buf);

  /* If the caller passes us a NULL calculator object, then display an error
   * message.  Otherwise get the console data from the calculator object. */
  if( (calc == (calculator *) 0) ||
      (calculator_get_console(calc, calc_obj_buf, sizeof(calc_obj_buf)) == false) )
  {
    snprintf(calc_obj_buf, (sizeof(calc_obj_buf) - 1), "ERROR");
  }

  /* Now stretch/shrink the console data to fit the calculator's display. */
  char calc_dsp_buf[33];
  size_t calc_dsp_buf_size = sizeof(calc_dsp_buf);
  if(strlen(calc_obj_buf) < (calc_dsp_buf_size - 1))
  {
    /* Initialize the buffer with all spaces.  Then NULL terminate. */
    memset(calc_dsp_buf, ' ', calc_dsp_buf_size);
    calc_dsp_buf[calc_dsp_buf_size - 1] = 0;

    /* Now copy the calc_obj data to the end of the calc_dsp buffer. */
    int calc_dsp_buf_index = (calc_dsp_buf_size - 1 - strlen(calc_obj_buf));
    memcpy(&calc_dsp_buf[calc_dsp_buf_index], calc_obj_buf, strlen(calc_obj_buf));
  }
  else
  {
    memcpy(calc_dsp_buf, calc_obj_buf, calc_obj_buf_size);
  }

  /* Get the hex/dec setting. */
  char   base_str[16];
  size_t base_str_max = (sizeof(base_str) - 1);
  calculator_base base;
  if(calculator_get_base(calc, &base) == true)
  {
    switch(base)
    {
    case calculator_base_10: strncpy(base_str, "dec", base_str_max); break;
    case calculator_base_16: strncpy(base_str, "hex", base_str_max); break;
    default:                 strncpy(base_str, "!!!", base_str_max); break;
    }
  }
  else
  {
    strncpy(base_str, "???", base_str_max);
  }

  /* Display our calculator output. */
  char calc_buf[64];
  snprintf(calc_buf, sizeof(calc_buf), "\r-- %s -->%s<--\b\b\b\b", base_str, calc_dsp_buf);
  fprintf(stderr, calc_buf);

  retcode = true;

  return retcode;
}

int main(int argc, char **argv)
{
  /* Create a console object.  We will use it to read raw console input. */
  raw_stdin *console = raw_stdin_new();
  if(console != (raw_stdin *) 0)
  {
    /* Create a calculator object. */
    calculator *calc = calculator_new();
    if(calc != (calculator *) 0)
    {
      fprintf(stderr, "Enter an equation.  'h' for help.\n");
      display_calc(calc);

      bool done = false;
      while(done == false)
      {
        char c = raw_stdin_getchar(console);
        switch(c)
        {
        case 'h':
          display_help();
          break;

        case 'm':
          {
            calculator_base cur_base;
            calculator_base new_base;
            if(calculator_get_base(calc, &cur_base) == true)
            {
              switch(cur_base)
              {
              case calculator_base_10: new_base = calculator_base_16;      break;
              case calculator_base_16: new_base = calculator_base_10;      break;
              default:                 new_base = calculator_base_unknown; break;
              }
              calculator_set_base(calc, new_base);
            }
          }
          break;

        case 'q':
          done = true;
          break;

        default:
          calculator_add_char(calc, c);
          break;
        }

        display_calc(calc);
      }

      calculator_delete(calc);
    }

    raw_stdin_free(console);
    printf("\nBye.\n");
  }

  else
  {
    fprintf(stderr, "Unable to create raw console object.\n");
    return 1;
  }
  return 0;
}


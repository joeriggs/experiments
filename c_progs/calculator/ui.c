/* This is the UI for the calculator.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#include "common.h"

#include "calculator.h"
#include "raw_stdin.h"
#include "ui.h"

/* This is the width of the calculator display window. */
#define CALC_DISPLAY_WINDOW_WIDTH 32

/* This is the help message.
 *
 * Input:
 *   N/A.
 *
 * Output:
 *   N/A.
 */
static void
ui_display_help(void)
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
 *   true  = success.  The calculator display is updated.
 *   false = failure.  The calculator display is NOT updated.
 */
static bool
ui_display_calc(calculator *calc)
{
  bool retcode = false;

  char   calc_obj_buf[CALC_DISPLAY_WINDOW_WIDTH];
  size_t calc_obj_buf_size = sizeof(calc_obj_buf);

  /* If the caller passes us a NULL calculator object, then display an error. */
  if(calc == (calculator *) 0)
  {
    snprintf(calc_obj_buf, (calc_obj_buf_size - 1), "ERROR");
  }
  /* Get the console data from the calculator object. */
  else if(calculator_get_console(calc, calc_obj_buf, calc_obj_buf_size) == false)
  {
    snprintf(calc_obj_buf, (calc_obj_buf_size - 1), "Error");
  }

  /* Now stretch/shrink the console data to fit the calculator's display. */
  char   calc_dsp_buf[CALC_DISPLAY_WINDOW_WIDTH];
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
  char   base_str[8];
  size_t base_str_max = (sizeof(base_str) - 1);
  operand_base base;
  if(calculator_get_operand_base(calc, &base) == true)
  {
    switch(base)
    {
    case operand_base_10: strncpy(base_str, "dec", base_str_max); break;
    case operand_base_16: strncpy(base_str, "hex", base_str_max); break;
    default:                 strncpy(base_str, "!!!", base_str_max); break;
    }
  }
  else
  {
    strncpy(base_str, "???", base_str_max);
  }

  /* Display our calculator output. */
  char calc_buf[CALC_DISPLAY_WINDOW_WIDTH + 64];
  snprintf(calc_buf, sizeof(calc_buf), "\r-- %s -->%s<--\b\b\b\b", base_str, calc_dsp_buf);
  fprintf(stderr, calc_buf);

  retcode = true;

  return retcode;
}

bool ui(void)
{
  /* Create the console and calculator objects. */
  raw_stdin *console = raw_stdin_new();
  calculator *calc = calculator_new();
  if( (console != (raw_stdin *) 0) && (calc != (calculator *) 0) )
  {
    fprintf(stderr, "Enter an equation.  'h' for help.\n");
    ui_display_calc(calc);

    bool keep_going = true;
    while(keep_going == true)
    {
      /* Get the next character from the user. */
      char c;
      if((keep_going = raw_stdin_getchar(console, &c)) == true)
      {
        switch(c)
        {
        case 'h':
          ui_display_help();
          break;

        case 'm':
          {
            operand_base cur_base;
            if(calculator_get_operand_base(calc, &cur_base) == true)
            {
              operand_base new_base;
              switch(cur_base)
              {
              case operand_base_10: new_base = operand_base_16;      break;
              case operand_base_16: new_base = operand_base_10;      break;
              default:              new_base = operand_base_unknown; break;
              }
              calculator_set_operand_base(calc, new_base);
            }
          }
          break;

        case 'q':
          keep_going = false;
          break;

        default:
          calculator_add_char(calc, c);
          break;
        }
      }

      ui_display_calc(calc);
    }
  }

  if(calc != (calculator *) 0)
  {
    calculator_delete(calc);
  }

  if(console != (raw_stdin *) 0)
  {
    raw_stdin_delete(console);
    printf("\nBye.\n");
  }

  return 0;
}


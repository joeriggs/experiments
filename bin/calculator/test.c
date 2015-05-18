/* This is a test program that exercises all of the classes that comprise the
 * calculator.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include "common.h"

#include "calculator.h"
#include "list.h"
#include "operand.h"
#include "operator.h"
#include "raw_stdin.h"
#include "stack.h"

/******************************** PRIVATE API *********************************/

/* Return a string that indicates the result of a test.  All tests return a
 * boolean that indicates PASS (true) or FAIL (false).  This function simply
 * converts the boolean to a string that can be displayed.
 *
 * Input:
 *   result = The boolean value to convert to a string.
 *
 * Output:
 *   Returns a string that indicates the PASS/FAIL result.
 */
static const char *
test_result_to_str(bool result)
{
  return (result == true) ? "PASS" : "FAIL";
}

/********************************* PUBLIC API *********************************/

int main(int argc, char **argv)
{
  bool retcode;

  retcode = calculator_test();
  printf("Calculator test: %s.\n", test_result_to_str(retcode));

  if(retcode == true)
  {
    retcode = list_test();
    printf("List test: %s.\n", test_result_to_str(retcode));
  }

  if(retcode == true)
  {
    retcode = operand_test();
    printf("Operand test: %s.\n", test_result_to_str(retcode));
  }

  if(retcode == true)
  {
    retcode = operator_test();
    printf("Operator test: %s.\n", test_result_to_str(retcode));
  }

  if(retcode == true)
  {
    retcode = raw_stdin_test();
    printf("Raw Console test: %s.\n", test_result_to_str(retcode));
  }

  if(retcode == true)
  {
    retcode = stack_test();
    printf("Stack test: %s.\n", test_result_to_str(retcode));
  }

  return 0;
}


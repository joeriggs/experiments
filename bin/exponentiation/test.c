/*
 * This program will test the floating point exponent class.
 */
#include <stdio.h>

#include "fp_exponent.h"

/******************************** PRIVATE API *********************************/

typedef bool (*test_func)(void);

/* Return a string that indicates the result of a test.  All tests return a
 * boolean that indicates PASS (true) or FAIL (false).  This function simply
 * converts the boolean to a string that can be displayed.
 *
 * Input:
 *   name = An ASCII string that contains the name of the test.
 *
 *   test = A pointer to the test function.
 *
 * Output:
 *   true  = success.  The test passed.
 *   false = failure.  The test failed.
 */
static bool
test_run_one_test(const char *name,
                  test_func   test)
{
  bool retcode = test();
  printf("%s: %s.\n", name, (retcode == true) ? "PASS" : "FAIL");
  return retcode;
}

/********************************* PUBLIC API *********************************/

int
main(int argc,
     char **argv)
{
  bool retcode = false;

  printf("Library tests.\n");

  do
  {
    if((retcode = test_run_one_test("fp_exponent", fp_exponent_test)) == false) break;

  } while (0);

  if(retcode == true)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

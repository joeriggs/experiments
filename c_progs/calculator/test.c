/* This is a test program that exercises all of the classes that comprise the
 * calculator.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include "common.h"

#include "bcd.h"
#include "calculator.h"
#include "fp_exp.h"
#include "hex.h"
#include "list.h"
#include "operand.h"
#include "operator.h"
#include "raw_stdin.h"
#include "stack.h"
#include "test.h"

/******************************** PRIVATE API *********************************/

typedef bool (*test_func)(void);

/* Run a test, print the result, and return a pass/fail status.  All tests
 * return a boolean that indicates PASS (true) or FAIL (false).  This function
 * converts the boolean to a string and dumps it to the console.
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

bool test(void)
{
  bool retcode = true;

  printf("Run tests.\n");

  /* Look through each of the unit tests.  Fail immediately if any of the tests
   * fails. */
  typedef struct unit_test {
    const char *name;
    test_func   func;
  } unit_test;
  unit_test tests[] = {
    { "BCD",         bcd_test         },
    { "Calculator",  calculator_test  },
    { "FP Exponent", fp_exp_test      },
    { "HEX",         hex_test         },
    { "List",        list_test        },
    { "Operand",     operand_test     },
    { "Operator",    operator_test    },
    { "Raw Console", raw_stdin_test   },
    { "Stack",       stack_test       },
  };
  size_t tests_size = (sizeof(tests) / sizeof(unit_test));

  int x;
  for(x = 0; (x < tests_size) && (retcode == true); x++)
  {
    unit_test *t = &tests[x];
    retcode = test_run_one_test(t->name, t->func);
  }

  if(retcode == true)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}


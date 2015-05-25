
#include <stdint.h>
#include <stdio.h>

#include "fp_exponent.h"

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

/******************************************************************************
 ********************************* PUBLIC API *********************************
 *****************************************************************************/

/* Perform an exponentiation with a floating point base and an integer exponent.
 * We will calculate (base^exp).
 *
 * Input:
 *   base = The base.
 *
 *   exp  = The exponent.  Currently only supports positive exponents.
 *
 * Output:
 *   Returns the result.
 */
static double
fp_exponent_integer_exp(double base,
                        int exp)
{
  double result = 0.0;

  /* base^0 = 1.0 */
  if(exp == 0)
  {
    result = 1.0;
  }

  /* base^1 = base */
  else if(exp == 1)
  {
    result = base;
  }

  /* All others must be calculated. */
  else
  {
    result = base;
    while(--exp > 0)
    {
      result *= base;
    }
  }

  return result;
}

/* This is a very fast-converging nth root algorithm for finding the nth root
 * of A.  This member will return "Delta X_k".
 *
 * Taken from http://en.wikipedia.org/wiki/Nth_root_algorithm
 *
 * 1. Make an initial guess X_k.
 *
 * 2. X_k+1 = (1 / n) * ( (n - 1) * X_k + (A / X_k^(n-1))).
 *
 *    In practice we do:
 *      Delta X_k = (1 / n) * ((A / X_k^(n-1)) - X_k); X_k+1 = X_k + Delta X_k.
 *
 * 3. Repeat step 2 until the desired precision is reached, i.e.
 *      |Delta X_k| < Epsilon.
 *
 * Input:
 *   A   = A (see above).
 *
 *   n   = nth root (see above).
 *
 *   X_k = The next guess.
 *
 * Output:
 *   Returns the delta.
 */
static double
fp_exponent_nth_root_guess(double A,
                           int    n,
                           double X_k)
{
  double delta = X_k;

  /*                       --------PART__4--------
   *                       -----PART__3----
   *             PART__1         -PART__2-
   * Delta X_k = (1 / n) * ((A / X_k^(n-1)) - X_k); X_k+1 = X_k + Delta X_k.
   */
  double part1 = 1.0 / n;
  double part2 = fp_exponent_integer_exp(X_k, (n - 1));
  double part3 = A / part2;
  double part4 = part3 - X_k;
  delta = part1 * part4;

  return delta;
}

/*
 * Taken from http://mathforum.org/library/drmath/view/55896.html
 *
 * What we want to do in this module is calculate an exponentiation where the
 * exponent is not an integer.  For this explanation, we'll use the following
 * example:
 *
 *     x = (5^3.4).
 *
 * Using this example, we need to think of (3.4) as (34/10).  (34/10) can be
 * thought of as (34 * 1/10).  Using this info, we can rewrite the original
 * problem as:
 *
 *     x = ( 5^(1/10) )^34).
 *
 * Breaking this down into 2 steps, we first solve the following:
 *
 *     R = 5^(1/10)
 *
 * That's the same as calculating the 5th root of 10.  Easy enough.  We can
 * do that using Newton's guessing algorithm.
 *
 * Then we solve the following:
 *
 *     x = R^34
 *
 * And then we have solved x = (5^3.4).
 *
 * Input:
 *   base   - The base of the exponentiation.
 *
 *   exp    - The exponent of the exponentiation.
 *
 *   result - A pointer to the location to store the result.
 *
 * Output:
 *   true  = success.  *result contains the result.
 *   false = failure.  *results is undefined.
 */
bool
fp_exponent_calc(double  base,
                 double  exp,
                 double *result)
{
  bool retcode = false;
  uint64_t exp_i;
  double   exp_f;

  if(result == (double *) 0)
  {
    return retcode;
  }

  /* If the exponent is less than zero, then convert to its absolute value and
   * set a flag to remind us it was negative.  x^-n = 1/(x^n), so we just need
   * to get the inverse when we're done. */
  bool do_inverse = false;
  if(exp < 0.0)
  {
    do_inverse = true;
    exp = 0.0 - exp;
  }

  /* Is the exponent a whole number? */
  exp_i = (uint64_t) exp;
  exp_f = (double) exp_i;
  if(exp == exp_f)
  {
    *result = fp_exponent_integer_exp(base, exp_i);
    retcode = true;
  }

  else
  {
    /* Split the exponent into 2 pieces (Better implementation later):
     * 1. The integer portion (exp_i).
     * 2. The fraction portion (exp_f).
     */
    int loop;
    for(loop = 1; loop < 6; loop++)
    {
      double root = fp_exponent_integer_exp(10.0, loop);
      double tmp_f1 = exp * root;
      uint64_t tmp_i = (uint64_t) tmp_f1;
      double tmp_f2 = (double) tmp_i;
      if(tmp_f1 == tmp_f2)
      {
        exp_i = tmp_i;
        exp_f = root;
        break;
      }
    }
    // printf("exp %f: exp_i %lld: exp_f %f\n", exp, exp_i, exp_f);

    double guess = 2;
    double delta;
    int x;
    for(x = 0; x < 100; x++)
    {
      delta = fp_exponent_nth_root_guess(base, exp_f, guess);
      if(delta == 0.0)
      {
        break;
      }
      guess += delta;
    }
    // printf("base %f: exp_f %f: guess %f: delta %f\n", base, exp_f, guess, delta);

    *result = fp_exponent_integer_exp(guess, exp_i);
    // printf("guess %f: exp_i %lld: *result %f\n", guess, exp_i, *result);
    retcode = true;
  }

  if((retcode == true) && (do_inverse == true))
  {
    *result = 1.0 / *result;
  }

  return retcode;
}

/******************************************************************************
 ********************************** TEST API **********************************
 *****************************************************************************/

#if defined(TEST)
bool
fp_exponent_test(void)
{
  bool retcode = true;

  /* Here are some problems to test against. */
  typedef struct fp_exponent_test {
    double base;
    double exp;
    double result;
  } fp_exponent_test;
  fp_exponent_test tests[] = {
    { 2.0,  3.0,  8.0               },
    { 2.0, -3.0,  0.125             },
    { 2.0,  3.5, 11.313708498984749 },
  };
  size_t tests_size = (sizeof(tests) / sizeof(fp_exponent_test));

  int x;
  for(x = 0; x < tests_size; x++)
  {
    fp_exponent_test *t = &tests[x];

    double result;
    bool rc = fp_exponent_calc(t->base, t->exp, &result);
    printf("rc %s: result = %10.15f: t->result %10.15f: ",
            (rc == true) ? "true" : "false", result, t->result);
    if((rc == true) && (result == t->result))
    {
      printf("PASS\n");
    }
    else
    {
      printf("FAIL\n");
      retcode = false;
      break;
    }
  }

  return retcode;
}
#endif // TEST

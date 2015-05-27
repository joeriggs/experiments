
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "common.h"

#include "fp_exponent.h"

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/* This is the fp_exponent class. */
struct fp_exponent {
  double base;
  double exp;
  double result;

  /* We often need to convert exp to a fraction.  It is stored here. */
  uint64_t exp_numerator;
  uint64_t exp_denominator;
};

/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

/* Perform an exponentiation with a floating point base and an integer exponent.
 * We've already determined this->exp is a whole number, so we can do simple
 * exponent math.  We will calculate (this->base^this->exp).
 *
 * Note that we aren't using the base and exp that are stored in an fp_exponent
 * object.  This method just provides a utility service that are needed to
 * calculate the actual exponent.
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
fp_exponent_integer_exp(double       base,
                        int          exp)
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


/* Convert the floating point exponent into a fraction, and then reduce the
 * fraction.  For example, if this->exp = 3.45:
 * 1. Convert it to = 345/100.
 * 2. Reduce it down to 69/20.
 *
 * To reduce, find the greatest common Divisor (GCD) (using the Euclid method),
 * and use it to reduce the exponent fraction.
 *
 * Input:
 *   this = A pointer to the fp_exponent object.
 *
 * Output:
 *   true  = success.  The exponent has been converted to a fraction.
 *   false = failure.  Something went wrong.
 */
static bool
fp_exponent_to_fraction(fp_exponent *this)
{
  bool retcode = false;

  if(this != (fp_exponent *) 0)
  {
    /* Convert the exponent to a fraction (numerator and denominator), and then
     * reduce the fraction.
     */
    int loop;
    for(loop = 1; loop < 20; loop++)
    {
      double root = fp_exponent_integer_exp(10.0, loop);
      double tmp_f1 = this->exp * root;
      int64_t tmp_i = (int64_t) tmp_f1;
      double tmp_f2 = (double) tmp_i;
      if(tmp_f1 == tmp_f2)
      {
        this->exp_numerator   = tmp_i;
        this->exp_denominator = root;
        retcode = true;
        break;
      }
    }

    if(retcode == true)
    {
      DBG_PRINT("%s(BEF): this->exp_numerator %lld: this->exp_denominator %lld\n",
                  __func__, this->exp_numerator, this->exp_denominator);

      /* Initialize a and b to equal the numerator and denominator.  Make sure
       * that a >= b. */
      uint64_t a, b;
      if(this->exp_numerator >= this->exp_denominator)
      {
        a = this->exp_numerator;
        b = this->exp_denominator;
      }
      else
      {
        a = this->exp_denominator;
        b = this->exp_numerator;
      }

      /* Loop until we calculate the GCD. */
      uint64_t gcd;
      while(1)
      {
        if((gcd = a % b) == 0)
        {
          gcd = b;
          break;
        }

        a = b;
        b = gcd;
      }

      /* Now reduce the fraction by the GCD. */
      this->exp_numerator /= gcd;
      this->exp_denominator /= gcd;

      DBG_PRINT("%s(AFT): this->exp_numerator %lld: this->exp_denominator %lld\n",
                  __func__, this->exp_numerator, this->exp_denominator);
    }
  }

  return retcode;
}

/* This is a very fast-converging nth root algorithm for finding the nth root
 * of A.
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
 *   this = A pointer to the fp_exponent object.  In this member, the variables
 *          are as follows:
 *          A   = this->base
 *          n   = this->exp_denominator
 *          X_k = guess
 *
 * Output:
 *   Returns the guess.
 */
static double
fp_exponent_nth_root_guess(fp_exponent *this)
{
  double   A   = this->base;
  uint64_t n   = this->exp_denominator;
  DBG_PRINT("%s(): A %f: n %lld\n", __func__, A, n);
  double X_k = 2;

  /* Solve the nth root (see description above). */
  int x;
  for(x = 0; x < 1000; x++)
  {
    /*                       --------PART__4--------
     *                       -----PART__3----
     *             PART__1         -PART__2-
     * Delta X_k = (1 / n) * ((A / X_k^(n-1)) - X_k); X_k+1 = X_k + Delta X_k.
     */
    double part1 = 1.0 / n;
    double part2 = fp_exponent_integer_exp(X_k, (n - 1));
    double part3 = A / part2;
    double part4 = part3 - X_k;
    double delta_X_k = part1 * part4;
    if(delta_X_k == 0.0)
    {
      break;
    }
    X_k += delta_X_k;
  }
  DBG_PRINT("%s(): x %d: this->base %f: this->exp_denominator %lld: X_k %10.15f\n",
             __func__, x, this->base, this->exp_denominator, X_k);

  return X_k;
}

/******************************************************************************
 ********************************* PUBLIC API *********************************
 *****************************************************************************/

/* Create a new fp_exponent object.  This object can be used to access the
 * fp_exponent class.
 *
 * Input:
 *   base = The base for the exponentiation.
 *
 *   exp  = The floating point exponent.
 *
 * Output:
 *   Returns a pointer to the object.
 *   Returns 0 if unable to create the object.
 */
fp_exponent *
fp_exponent_new(double base,
                double exp)
{
  fp_exponent *this = (fp_exponent *) 0;

  /* Initialize. */
  if((this = (fp_exponent *) malloc(sizeof(*this))) != (fp_exponent *) 0)
  {
    /* Save the base and exponent. */
    this->base = base;
    this->exp  = exp;
  }

  return this;
}

/* Delete an fp_exponent object that was created by fp_exponent_new().
 *
 * Input:
 *   this = A pointer to the fp_exponent object.
 *
 * Output:
 *   true  = success.  The object is deleted.
 *   false = failure.
 */
bool
fp_exponent_delete(fp_exponent *this)
{
  bool retcode = false;

  if(this != (fp_exponent *) 0)
  {
    free(this);
    retcode = true;
  }

  return retcode;
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
 * 1. Breaking this down into 2 steps, we first solve the 5th root of 10.  Easy
 *    enough.  We can do that using Newton's guessing algorithm.
 *
 *     R = 5^(1/10)
 *
 * 2. Then we solve the following:
 *
 *     x = R^34
 *
 * And then we have solved x = (5^3.4).
 *
 * Input:
 *   this   = A pointer to the fp_exponent object.
 *
 * Output:
 *   true  = success.  *result contains the result.
 *   false = failure.  *results is undefined.
 */
bool
fp_exponent_calc(fp_exponent *this)
{
  bool retcode = false;

  if(this == (fp_exponent *) 0)
  {
    return retcode;
  }

  /* If the exponent is negative, convert to its absolute value and set a flag
   * to remind us it was negative.  x^-n = 1/(x^n), so we just need to get the
   * inverse when we're done. */
  bool do_inverse = false;
  if(this->exp < 0.0)
  {
    do_inverse = true;
    this->exp = 0.0 - this->exp;
  }

  /* Check to see if the exponent is a whole number.  If it is, then we can do
   * easy exponentiation. */
  int64_t tmp_exp_i  = (int64_t) this->exp;
  double   tmp_exp_f = (double) tmp_exp_i;
  if(this->exp == tmp_exp_f)
  {
    this->result = fp_exponent_integer_exp(this->base, tmp_exp_i);
    retcode = true;
  }

  else
  {
    /* Convert the exponent to a fraction (numerator and denominator). */
    if((retcode = fp_exponent_to_fraction(this)) == true)
    {
      /* Solve the nth root (see description above). */
      double guess = fp_exponent_nth_root_guess(this);
      DBG_PRINT("%s(): nth_root: this->base %f: this->exp_denominator %lld: guess %f\n",
                 __func__, this->base, this->exp_denominator, guess);

      this->result = fp_exponent_integer_exp(guess, this->exp_numerator);
      DBG_PRINT("%s(): exp: guess %f: this->exp_numerator %lld: this->result %f\n",
                  __func__, guess, this->exp_numerator, this->result);
    }
  }

  if((retcode == true) && (do_inverse == true))
  {
    this->result = 1.0 / this->result;
  }

  return retcode;
}

/* Get the result from fp_exponent_calc().
 * Input:
 *   this   = A pointer to the fp_exponent object.
 *
 *   result = A pointer to the location where we will store the result.
 *
 * Output:
 *   true  = success.  *result contains the result.
 *   false = failure.  *results is undefined.
 */
bool
fp_exponent_get_result(fp_exponent *this,
                       double      *result)
{
  bool retcode = false;

  if((this != (fp_exponent *) 0) && (result != (double *) 0))
  {
    *result = this->result;
    retcode = true;
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
    { 2.0,   3.0,    8.0               },
    { 2.0,  -3.0,    0.125             },
    { 2.0,  -2.3456, 0.196745153116215 },
    { 2.34,  3.45,  18.784286696359032 },
    { 2.0,   3.5,   11.313708498984754 },
    { 2.0,   3.6,   12.125732532083205 },
  };
  size_t tests_size = (sizeof(tests) / sizeof(fp_exponent_test));

  int x;
  for(x = 0; x < tests_size; x++)
  {
    bool rc;
    fp_exponent_test *t = &tests[x];
    fp_exponent *obj;
    rc = ((obj = fp_exponent_new(t->base, t->exp)) != (fp_exponent *) 0) ? true : false;
    if((rc) && (obj != (fp_exponent *) 0))
    {
      double result = 0.0;
      if((rc = fp_exponent_calc(obj)) == true)
      {
        rc = fp_exponent_get_result(obj, &result);
      }
      printf("  rc %s: result = %10.15f: t->result %10.15f: ",
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

    if(rc == false)
    {
      break;
    }
  }

  return retcode;
}
#endif // TEST

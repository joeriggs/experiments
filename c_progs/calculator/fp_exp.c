/* This file contains an ADT that performs floating point exponentiation.
 * Floating poing exponentiation is a lot more complicated than plain old
 * decimal exponentiation, so it has been isolated in this ADT.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

#include "bcd.h"
#include "fp_exp.h"

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/* This is the fp_exp class. */
struct fp_exp {
  bcd *base;
  bcd *exp;
  bcd *result;

  /* We often need to convert exp to a fraction.  It is stored here. */
  uint64_t exp_numerator;
  uint64_t exp_denominator;
};

/******************************************************************************
 ******************************** PRIMITIVES **********************************
 *****************************************************************************/

/* Perform an exponentiation with a floating point base and an integer exponent.
 *
 * Input:
 *   base   = The floating point base.
 *
 *   exp    = The integer exponent.  Currently only supports positive exponents.
 *
 *   result = A pointer to the bcd object that will receive the result.  Note
 *            that it is okay if base == result.  We will make sure we don't
 *            stomp on base while we compute result.
 *
 * Output:
 *   true  = success.  *result contains the result.
 *   false = failure.  *result is undefined.
 */
static bool
fp_exp_integer_exp(bcd *base,
                   int  exp,
                   bcd *result)
{
  bool retcode = false;

  if((base != (bcd *) 0) && (result != (bcd *) 0))
  {
    bcd *rslt_tmp = (bcd *) 0;
    bcd *base_tmp = (bcd *) 0;
    bcd *zero     = (bcd *) 0;
    bcd *one      = (bcd *) 0;

    do
    {
      if((rslt_tmp = (base == result) ? bcd_new() : result) == (bcd *) 0) { break; }
      if((base_tmp = bcd_new()) == (bcd *) 0)                             { break; }
      if((zero     = bcd_new()) == (bcd *) 0)                             { break; }
      if((one      = bcd_new()) == (bcd *) 0)                             { break; }

      if(bcd_copy(base, base_tmp) == false)                               { break; }
      if(bcd_import(zero, 0) == false)                                    { break; }
      if(bcd_import(one, 1) == false)                                     { break; }

      /* Special case.  base ^ 0 = 1. */
      if(exp == 0)
      {
        retcode = bcd_copy(one, rslt_tmp);
        break;
      }

      /* Special case.  0 ^ exp = 0. */
      if(bcd_cmp(base, zero) == 0)
      {
        retcode = bcd_copy(zero, rslt_tmp);
        break;
      }

      /* Set res = 1.  (base ^ 0) = 1, so this is the right place to start. */
      if(bcd_import(rslt_tmp, 1) == false)                                { break; }

      while(exp != 0)
      {
        if((exp & 1) != 0)
        {
          if(bcd_op_mul(rslt_tmp, base_tmp) == false)                     { break; }
        }

        /* Prepare for the next iteration. */
        if(bcd_op_mul(base_tmp, base_tmp) == false)                       { break; }
        exp >>= 1;
      }

      retcode = (exp == 0) ? true : false;
      
    } while(0);

    bcd_delete(one);
    bcd_delete(zero);
    bcd_delete(base_tmp);

    if((retcode == true) && (base == result))
    {
      retcode = bcd_copy(rslt_tmp, result);
      bcd_delete(rslt_tmp);
    }
  }

  return retcode;
}

/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

/* Convert the floating point exponent into a fraction, and then reduce the
 * fraction.  For example, if this->exp = 3.45:
 * 1. Convert it to = 345/100.
 * 2. Reduce it down to 69/20.
 *
 * To reduce, find the greatest common Divisor (GCD) (using the Euclid method),
 * and use it to reduce the exponent fraction.
 *
 * Input:
 *   this = A pointer to the fp_exp object.
 *
 * Output:
 *   true  = success.  The exponent has been converted to a fraction.  The
 *                     fraction is stored in this->exp_numerator and
 *                     this->exp_denominator.
 *   false = failure.  Something went wrong.
 */
static bool
fp_exp_to_fraction(fp_exp *this)
{
  bool retcode = false;

  /* We'll use these inside a loop, then we'll delete them when we're done. */
  bcd *ten    = (bcd *) 0;
  bcd *root   = (bcd *) 0;
  bcd *tmp_f1 = (bcd *) 0;
  bcd *tmp_f2 = (bcd *) 0;

  do
  {
    if(this == (fp_exp *) 0)                                       { break; }

    if((ten    = bcd_new()) == (bcd *) 0)                          { break; }
    if((root   = bcd_new()) == (bcd *) 0)                          { break; }
    if((tmp_f1 = bcd_new()) == (bcd *) 0)                          { break; }
    if((tmp_f2 = bcd_new()) == (bcd *) 0)                          { break; }

    if(bcd_import(ten, 10) == false)                               { break; }

    /* Convert the exponent to a fraction (numerator and denominator), and then
     * reduce the fraction.
     */
    int loop;
    for(loop = 1; loop < 20; loop++)
    {
      if((retcode = fp_exp_integer_exp(ten, loop, root)) == false) { break; }

      if(bcd_copy(this->exp, tmp_f1) == false)                     { break; }
      if((retcode = bcd_op_mul(tmp_f1, root)) == false)            { break; }

      int64_t tmp_i;
      if(bcd_export(tmp_f1, &tmp_i) == false)                      { break; }
      if(bcd_import(tmp_f2, tmp_i) == false)                       { break; }

      if(bcd_cmp(tmp_f1, tmp_f2) == 0)
      {
        this->exp_numerator = tmp_i;
        if(bcd_export(root, &tmp_i) == false)                      { break; }
        this->exp_denominator = tmp_i;
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
  } while(0);

  bcd_delete(tmp_f2);
  bcd_delete(tmp_f1);
  bcd_delete(root);
  bcd_delete(ten);

  return retcode;
}

/* This is a very fast-converging nth root algorithm to find the nth root of A.
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
 *   this  = A pointer to the fp_exp object.  In this member, the variables
 *           are as follows:
 *           A   = this->base
 *           n   = this->exp_denominator
 *
 *   guess = A pointer to a bcd object that will receive the guess.
 *           X_k = guess
 *
 * Output:
 *   true  = success.  *guess contains the guess.
 *   false = failure.  The contents of *guess is undefined.
 */
static bool
fp_exp_nth_root_guess(fp_exp *this,
                      bcd    *guess)
{
  bool retcode = false;

  bcd *A         = (bcd *) 0;
  bcd *n_f       = (bcd *) 0;
  bcd *X_k       = (bcd *) 0;
  bcd *part1     = (bcd *) 0;
  bcd *part2     = (bcd *) 0;
  bcd *part3     = (bcd *) 0;
  bcd *part4     = (bcd *) 0;
  bcd *delta_X_k = (bcd *) 0;
  bcd *zero      = (bcd *) 0;

  do
  {
    if((A = bcd_new()) == (bcd *) 0)                           { break; }
    if(bcd_copy(this->base, A) == false)                       { break; }

    uint64_t n_int = this->exp_denominator;
    if((n_f = bcd_new()) == (bcd *) 0)                         { break; }
    if(bcd_import(n_f, this->exp_denominator) == false)        { break; }

    char str1[64];
    if(bcd_to_str(A, str1, sizeof(str1)) == false)             { break; }
    DBG_PRINT("%s(): A %s: n %lld\n", __func__, str1, n_int);

    if((X_k = bcd_new()) == (bcd *) 0)                         { break; }
    if(bcd_import(X_k, 2) == false)                            { break; }

    if((part1     = bcd_new()) == (bcd *) 0)                   { break; }
    if((part2     = bcd_new()) == (bcd *) 0)                   { break; }
    if((part3     = bcd_new()) == (bcd *) 0)                   { break; }
    if((part4     = bcd_new()) == (bcd *) 0)                   { break; }
    if((delta_X_k = bcd_new()) == (bcd *) 0)                   { break; }
    if((zero      = bcd_new()) == (bcd *) 0)                   { break; }

    if(bcd_import(zero, 0) == false)                           { break; }

    /* Solve the nth root (see description above). */
    int x;
    for(x = 0; x < 1000; x++)
    {
      /*                       --------PART__4--------
       *                       -----PART__3----
       *             PART__1         -PART__2-
       * Delta X_k = (1 / n) * ((A / X_k^(n-1)) - X_k); X_k+1 = X_k + Delta X_k.
       */
      if(bcd_import(part1, 1) == false)                        { break; }
      if(bcd_op_div(part1, n_f) == false)                      { break; }

      if(fp_exp_integer_exp(X_k, (n_int - 1), part2) == false) { break; }

      if(bcd_copy(A, part3) == false)                          { break; }
      if(bcd_op_div(part3, part2) == false)                    { break; }

      if(bcd_copy(part3, part4) == false)                      { break; }
      if(bcd_op_sub(part4, X_k) == false)                      { break; }

      if(bcd_copy(part1, delta_X_k) == false)                  { break; }
      if(bcd_op_mul(delta_X_k, part4) == false)                { break; }

      if(bcd_cmp(delta_X_k, zero) == 0)
      {
        break;
      }
      if(bcd_op_add(X_k, delta_X_k) == false)                  { break; }
      if(bcd_copy(X_k, guess) == false)                        { break; }
    }
    char str2[64];
    if(bcd_to_str(this->base, str1, sizeof(str1)) == false)    { break; }
    if(bcd_to_str(X_k,        str2, sizeof(str2)) == false)    { break; }
    DBG_PRINT("%s(): x %d: this->base %s: this->exp_denominator %lld: X_k %s\n",
               __func__, x, str1, this->exp_denominator, str2);

    retcode = true;
  } while(0);

  bcd_delete(zero);
  bcd_delete(delta_X_k);
  bcd_delete(part4);
  bcd_delete(part3);
  bcd_delete(part2);
  bcd_delete(part1);
  bcd_delete(X_k);
  bcd_delete(n_f);
  bcd_delete(A);

  return retcode;
}

/******************************************************************************
 ********************************* PUBLIC API *********************************
 *****************************************************************************/

/* Create a new fp_exp object.  This object can be used to access the
 * fp_exp class.
 *
 * Input:
 *   base = The floating point base.
 *
 *   exp  = The floating point exponent.
 *
 * Output:
 *   Returns a pointer to the object.
 *   Returns 0 if unable to create the object.
 */
fp_exp *
fp_exp_new(bcd *base,
           bcd *exp)
{
  fp_exp *this = (fp_exp *) 0;

  bool success = false;
  do
  {
    /* Initialize. */
    if((this = (fp_exp *) malloc(sizeof(*this))) == (fp_exp *) 0) { break; }

    this->base   = bcd_new();
    this->exp    = bcd_new();
    this->result = bcd_new();

    /* Make copies of the base and exponent. */
    if((this->base == (bcd *) 0) || (this->exp == (bcd *) 0))     { break; }
    
    if(bcd_copy(base, this->base) == false)                       { break; }
    if(bcd_copy(exp,  this->exp) == false)                        { break; }

    success = true;
  } while(0);

  if(success == false)
  {
    fp_exp_delete(this);
    this = (fp_exp *) 0;
  }

  return this;
}

/* Delete an fp_exp object that was created by fp_exp_new().
 *
 * Input:
 *   this = A pointer to the fp_exp object.
 *
 * Output:
 *   true  = success.  The object is deleted.
 *   false = failure.
 */
bool
fp_exp_delete(fp_exp *this)
{
  bool retcode = false;

  if(this != (fp_exp *) 0)
  {
    bcd_delete(this->result);
    bcd_delete(this->exp);
    bcd_delete(this->base);

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
 *   this   = A pointer to the fp_exp object.
 *
 * Output:
 *   true  = success.  *result contains the result.
 *   false = failure.  *results is undefined.
 */
bool
fp_exp_calc(fp_exp *this)
{
  bool retcode = false;

  bcd *zero      = (bcd *) 0;
  bcd *one      = (bcd *) 0;
  bcd *tmp_exp_f = (bcd *) 0;
  bcd *guess     = (bcd *) 0;

  do
  {
    if(this == (fp_exp *) 0)                                                           { break; }

    if((zero      = bcd_new()) == (bcd *) 0)                                           { break; }
    if((one       = bcd_new()) == (bcd *) 0)                                           { break; }
    if((tmp_exp_f = bcd_new()) == (bcd *) 0)                                           { break; }
    if((guess     = bcd_new()) == (bcd *) 0)                                           { break; }

    /* If the exponent is negative, convert to its absolute value and set a flag
     * to remind us it was negative.  x^-n = 1/(x^n), so we just need to get the
     * inverse when we're done. */
    bool is_neg_exponent = (bcd_cmp(this->exp, zero) < 0);
    if(is_neg_exponent)
    {
      if(bcd_op_sub(zero, this->exp) == false)                                         { break; }
      if(bcd_copy(zero, this->exp) == false)                                           { break; }
    }

    /* Check to see if the exponent is a whole number.  If it is, then we can do
     * easy exponentiation. */
    int64_t tmp_exp_i;
    if(bcd_export(this->exp, &tmp_exp_i) == false)                                     { break; }
    if(bcd_import(tmp_exp_f, tmp_exp_i) == false)                                      { break; }
    if(bcd_cmp(this->exp, tmp_exp_f) == 0)
    {
      if((retcode = fp_exp_integer_exp(this->base, tmp_exp_i, this->result)) == false) { break; }
    }

    else
    {
      /* Convert the exponent to a fraction (numerator and denominator). */
      if(fp_exp_to_fraction(this) == false)                                            { break; }

      /* Solve the nth root (see description above). */
      if(fp_exp_nth_root_guess(this, guess) == false)                                  { break; }

      char buf1[64], buf2[64];
      if(bcd_to_str(this->base, buf1, sizeof(buf1)) == false)                          { break; }
      if(bcd_to_str(guess,      buf2, sizeof(buf2)) == false)                          { break; }
      DBG_PRINT("%s(): nth_root: this->base %s: this->exp_denominator %lld: guess %s\n",
                 __func__, buf1, this->exp_denominator, buf2);

      if(bcd_to_str(guess,        buf1, sizeof(buf1)) == false)                        { break; }
      if(bcd_to_str(this->result, buf2, sizeof(buf1)) == false)                        { break; }
      if(fp_exp_integer_exp(guess, this->exp_numerator, this->result) == false)        { break; }
      DBG_PRINT("%s(): exp: guess %s: this->exp_numerator %lld: this->result %s\n",
                  __func__, buf1, this->exp_numerator, buf2);

      retcode = true;
    }

    if((retcode == true) && (is_neg_exponent == true))
    {
      if((retcode = bcd_import(one, 1)) == false)                                      { break; }
      if((retcode = bcd_op_div(one, this->result)) == false)                           { break; }
      if((retcode = bcd_copy(one, this->result)) == false)                             { break; }
    }
  } while(0);

  bcd_delete(guess);
  bcd_delete(tmp_exp_f);
  bcd_delete(one);
  bcd_delete(zero);

  return retcode;
}

/* Get the result from fp_exp_calc().
 *
 * Input:
 *   this   = A pointer to the fp_exp object.
 *
 *   result = A pointer to the location where we will store the result.
 *
 * Output:
 *   true  = success.  *result contains the result.
 *   false = failure.  *results is undefined.
 */
bool
fp_exp_get_result(fp_exp *this,
                  bcd    *result)
{
  bool retcode = false;

  if((this != (fp_exp *) 0) && (result != (bcd *) 0))
  {
    retcode = bcd_copy(this->result, result);
  }

  return retcode;
}

/******************************************************************************
 ********************************** TEST API **********************************
 *****************************************************************************/

#if defined(TEST)
bool
fp_exp_test(void)
{
  bool retcode = false;

  /* Here are some problems to test against. */
  typedef struct fp_exp_test {
    char *name;
    char *base;
    char *exp;
    char *result;
  } fp_exp_test;
  fp_exp_test tests[] = {
    { "FP_EXP_01",  "2"    ,   "3"      ,                     "8"                     }, // Int ^ Int = Int.
    { "FP_EXP_02", "18"    ,   "8"      ,        "11,019,960,576"                     }, //   Little tougher.
    { "FP_EXP_03", "97"    ,  "16"      ,                     "6.142536534626857e+31" }, // Test_1.
    { "FP_EXP_04", "97"    ,   "8"      , "7,837,433,594,376,961"                     }, // Test_2.
    { "FP_EXP_05", "97"    ,   "1"      ,                    "97"                     }, // Test_3.
    { "FP_EXP_06", "97"    ,  "25"      ,                     "4.66974705254372e+49"  }, // Test_1 * Test_2 * Test_3.
    { "FP_EXP_07",  "2"    ,   "3s"     ,                     "0.125"                 }, // Int ^ -Int = Fraction.
    { "FP_EXP_08", "25"    ,   "7s"     ,                     "0.00000000016384"      }, //   Little tougher.
    { "FP_EXP_09", "17"    ,  "21s"     ,                     "1.447346952625563e-26" }, //   Little tougher.
    { "FP_EXP_10", "17"    ,    ".23"   ,                     "1.918683107361833"     },
    { "FP_EXP_11",   ".9"  ,    ".7"    ,                     "0.928901697685371"     },
    { "FP_EXP_12",   ".63" ,   "8"      ,                     "0.024815578026752"     },
    { "FP_EXP_13",  "2"    ,   "2.3456s",                     "0.196745153116215"     },
    { "FP_EXP_14",  "2.34" ,   "3.45"   ,                    "18.784286696359032"     },
    { "FP_EXP_15",  "2"    ,   "3.5"    ,                    "11.313708498984754"     },
    { "FP_EXP_16",  "2"    ,   "3.6"    ,                    "12.125732532083205"     },
    { "FP_EXP_17",  "2"    ,   "0"      ,                     "1"                     }, // zero exponent.
    { "FP_EXP_18",  "0"    ,   "3"      ,                     "0"                     }, // zero base.
    { "FP_EXP_19",  "0"    ,   "0"      ,                     "1"                     }, // zero base and exponent.
    { "FP_EXP_20",  "2"    , "199"      ,                     "8.034690221294951e+59" }, // big exponent.
    { "FP_EXP_21", "25.43" ,   "1"      ,                    "25.43"                  }, // X ^ 1 = X.
    { "FP_EXP_22",  "3"    ,  "12.345"  ,               "776,357.74428398"            }, // Stolen from calculator.c.
  };
  size_t tests_size = (sizeof(tests) / sizeof(fp_exp_test));

  bcd *base   = (bcd *) 0;
  bcd *exp    = (bcd *) 0;
  bcd *result = (bcd *) 0;

  do
  {
    int x;
    for(x = 0; x < tests_size; x++)
    {
      fp_exp_test *t = &tests[x];
      printf("%s: %s ^ %s\n", t->name, t->base, t->exp);

      if((base   = bcd_new()) == (bcd *) 0)               { break; }
      if((exp    = bcd_new()) == (bcd *) 0)               { break; }
      if((result = bcd_new()) == (bcd *) 0)               { break; }

      /* Load the base and exponent into bcd objects.  We're not testing the bcd
       * class here, so don't worry too much about error checking. */
      {
        char *p;
        for(p = t->base; *p != 0; p++) { bcd_add_char(base, *p); }
        for(p = t->exp;  *p != 0; p++) { bcd_add_char(exp,  *p); }
      }

      fp_exp *obj;
      if((obj = fp_exp_new(base, exp)) == (fp_exp *) 0)   { break; }

      if(fp_exp_calc(obj) == false)                       { break; }
      if(fp_exp_get_result(obj, result) == false)         { break; }
      if(fp_exp_delete(obj) == false)                     { break; }

      char buf1[1024];
      if(bcd_to_str(result, buf1, sizeof(buf1)) == false) { break; }
      printf("  result = %s: t->result %s: ", buf1, t->result);

      if(strcmp(buf1, t->result) == 0)
      {
        printf("PASS\n");
      }
      else
      {
        printf("FAIL\n");
        break;
      }

      if(bcd_delete(base)   != true)                      { break; }
      base   = (bcd *) 0;
      if(bcd_delete(exp)    != true)                      { break; }
      exp    = (bcd *) 0;
      if(bcd_delete(result) != true)                      { break; }
      result = (bcd *) 0;
    }
  } while(0);

  bcd_delete(result);
  bcd_delete(exp);
  bcd_delete(base);

  return retcode;
}
#endif // TEST

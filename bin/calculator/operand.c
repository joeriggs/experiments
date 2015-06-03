/* A class that can be used to store and manipulate numbers.  It is used
 * exclusively by the calculator class.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "fp_exp.h"
#include "operand.h"

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/* This is the operand class. */
struct operand {
  int64_t  i_val;
  double   f_val;
  bool     got_decimal_point;
  double   fp_multiplier;

  /* The number base we're currently configured to use.  This refers to things
   * like base_10 or base_16. */
  operand_base base;
};
  
/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

/* Return the current value of the specified operand object.
 *
 * Input:
 *   this  = A pointer to the operand object.
 *
 *   is_fp = A ptr to a bool that is set to true if the operand is a float.
 *
 *   i_val = A ptr to an integer.
 *           if (is_fp == true)  = 0.
 *           if (is_fp == false) = the int value of the operand.
 *
 *   f_val = A ptr to a float.
 *           if (is_fp == true)  = the fp value of the operand.
 *           if (is_fp == false) = the int value of the operand.
 *
 * Output:
 *   true  = success.
 *   false = failure.
 */
static bool
operand_get_val(operand  *this,
                bool     *is_fp,
                int64_t  *i_val,
                double   *f_val)
{
  bool retcode = false;

  if(this != (operand *) 0)
  {
    if((*is_fp = this->got_decimal_point) == true)
    {
      *i_val = 0;
      *f_val = this->f_val;
    }
    else
    {
      *i_val = this->i_val;
      *f_val = this->i_val;
    }

    retcode = true;
  }

  return retcode;
}

/* Load 2 operands.  This is used to prep for a BINARY operation.
 *
 * Input:
 *   op1   = A pointer to the 1st operand.
 *
 *   op2   = A pointer to the 2nd operand.
 *
 *   is_fp = We set to true if you need to do fp math.
 *
 *   i1    = A pointer to an integer.  It is set to the integer value that is
 *           returned from operand_get_val(op1).
 *
 *   f1    = A pointer to a float.  It is set to the floating point value that
 *           is returned from operand_get_val(op1).
 *
 *   i2    = A pointer to an integer.  It is set to the integer value that is
 *           returned from operand_get_val(op2).
 *
 *   f1    = A pointer to a float.  It is set to the floating point value that
 *           is returned from operand_get_val(op2).
 *
 * Output:
 *   true  = success.
 *   false = failure.
 */
static bool
operand_op_get_binary_ops(operand  *op1,
                          operand  *op2,
                          bool     *is_fp,
                          int64_t  *i1,
                          double   *f1,
                          int64_t  *i2,
                          double   *f2)
{
  bool retcode = false;

  bool is_fp1, is_fp2;
  if( ((retcode = operand_get_val(op1, &is_fp1, i1, f1)) == true) &&
      ((retcode = operand_get_val(op2, &is_fp2, i2, f2)) == true) )
  {
    if((is_fp1 == true) || (is_fp2 == true))
    {
      *f1 = is_fp1 ? *f1 : *i1;
      *f2 = is_fp2 ? *f2 : *i2;
      *is_fp = true;
    }
    else
    {
      *f1 = *i1;
      *f2 = *i2;
      *is_fp = false;
    }
  }

  return retcode;
}

/* Update the values in an operand object.
 *
 * Input:
 *   this  = A pointer to the operand object.
 *
 *   is_fp = Indicates whether the operand contains a floating point value.
 *
 *   i_val = Indicates the integer value to save.  If (is_fp == true), then
 *           this value is N/A.
 *
 *   f_val = Indicates the floating point value to save.  If (is_fp == false),
 *           then this value is N/A.
 *
 * Output:
 *   true  = success.
 *   false = failure.
 */
static bool
operand_set_val(operand  *this,
                bool      is_fp,
                int64_t   i_val,
                double    f_val)
{
  bool retcode = false;

  if(this != (operand *) 0)
  {
    if(is_fp == true)
    {
      this->got_decimal_point = true;
      this->f_val = f_val;
      this->i_val = 0;
    }
    else
    {
      this->got_decimal_point = false;
      this->i_val = i_val;
      this->f_val = 0.0;
    }

    retcode = true;
  }

  return retcode;
}

/******************************************************************************
 ********************************* PUBLIC OPS *********************************
 *****************************************************************************/

/* This is the addition function.
 *
 * Input:
 *   op1  = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  Addition is BINARY.
 *
 * Output:
 *   true  = success.  op1 contains the sum.
 *   false = failure.
 */
bool
operand_op_add(operand *op1,
               operand *op2)
{
  bool retcode = false;

  bool is_fp;
  int64_t  i1 = 0,   i2 = 0;
  double   f1 = 0.0, f2 = 0.0;

  if((retcode = operand_op_get_binary_ops(op1, op2, &is_fp, &i1, &f1, &i2, &f2)) == true)
  {
    if(is_fp == true)
    {
      f1 += f2;
    }
    else
    {
      i1 += i2;
    }

    retcode = operand_set_val(op1, is_fp, i1, f1);
  }

  return retcode;
}

/* This is the subtraction function.
 *
 * Input:
 *   op1  = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  Subtraction is BINARY.
 *
 * Output:
 *   true  = success.  op1 contains the difference.
 *   false = failure.
 */
bool
operand_op_sub(operand *op1,
               operand *op2)
{
  bool retcode = false;

  bool is_fp;
  int64_t  i1 = 0,   i2 = 0;
  double   f1 = 0.0, f2 = 0.0;

  if((retcode = operand_op_get_binary_ops(op1, op2, &is_fp, &i1, &f1, &i2, &f2)) == true)
  {
    if(is_fp == true)
    {
      f1 -= f2;
    }
    else
    {
      i1 -= i2;
    }

    retcode = operand_set_val(op1, is_fp, i1, f1);
  }

  return retcode;
}

/* This is the multiplication function.
 *
 * Input:
 *   op1  = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  Multiplication is BINARY.
 *
 * Output:
 *   true  = success.  op1 contains the product.
 *   false = failure.
 */
bool
operand_op_mul(operand *op1,
               operand *op2)
{
  bool retcode = false;

  bool is_fp;
  int64_t  i1 = 0,   i2 = 0;
  double   f1 = 0.0, f2 = 0.0;

  if((retcode = operand_op_get_binary_ops(op1, op2, &is_fp, &i1, &f1, &i2, &f2)) == true)
  {
    if(is_fp == true)
    {
      f1 *= f2;
    }
    else
    {
      i1 *= i2;
    }

    retcode = operand_set_val(op1, is_fp, i1, f1);
  }

  return retcode;
}

/* This is the division function.
 *
 * Input:
 *   op1  = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  Division is BINARY.
 *
 * Output:
 *   true  = success.  op1 contains the quotient.
 *   false = failure.
 */
bool
operand_op_div(operand *op1,
               operand *op2)
{
  bool retcode = false;

  bool is_fp;
  int64_t  i1 = 0,   i2 = 0;
  double   f1 = 0.0, f2 = 0.0;

  if((retcode = operand_op_get_binary_ops(op1, op2, &is_fp, &i1, &f1, &i2, &f2)) == true)
  {
    if(is_fp == false)
    {
      /* Divide by zero is an error. */
      if(i2 == 0)
      {
        retcode = false;
      }

      /* Check to see if the result will be a float. */
      else if((i1 % i2) != 0)
      {
        f1 = i1;
        f2 = i2;
        is_fp = true;
      }

      /* Do the regular integer division. */
      else
      {
        i1 /= i2;
        retcode = operand_set_val(op1, is_fp, i1, f1);
      }
    }

    /* One (or more) of the following is true:
     * 1. op1 is a float.
     * 2. op2 is a float.
     * 3. i1 / i2 would leave a remainder.
     */
    if(is_fp == true)
    {
      if(f2 == 0)
      {
        retcode = false;
      }
      else
      {
        f1 /= f2;
        retcode = operand_set_val(op1, is_fp, i1, f1);
      }
    }
  }

  return retcode;
}

/* This is the exponentiation function.
 *
 * Input:
 *   op1  = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  Exponentiation is BINARY.
 *
 * Output:
 *   true  = success.  op1 contains the result.
 *   false = failure.
 */
bool
operand_op_exp(operand *op1,
               operand *op2)
{
  bool retcode = false;

  bool is_fp;
  int64_t  i1 = 0,   i2 = 0;
  double   f1 = 0.0, f2 = 0.0;

  if((retcode = operand_op_get_binary_ops(op1, op2, &is_fp, &i1, &f1, &i2, &f2)) == true)
  {
    /* If the exponent is a floating point number, or if the exponent is a
     * negative number, then do the exponentiation as floating point. */
    if( (is_fp == true) || (i2 < 0) )
    {
      fp_exp *fp = fp_exp_new(f1, f2);
      if(fp != (fp_exp *) 0)
      {
        if((retcode = fp_exp_calc(fp)) == true)
        {
          retcode = fp_exp_get_result(fp, &f1);
        }
        retcode = (retcode == true) && (fp_exp_delete(fp) == true);
        is_fp = true;
      }
    }
    else
    {
      /* x^0 = 1 */
      if(i2 == 0)
      {
        i1 = 1;
      }
      /* x^1 = x */
      else if(i2 == 1)
      {
      }
      /* Calculate x^n */
      else if(i2 > 1)
      {
        int i;
        int org_i1 = i1;
        for(i = 1; i < i2; i++)
        {
          i1 *= org_i1;
        }
      }
    }

    retcode = operand_set_val(op1, is_fp, i1, f1);
  }

  return retcode;
}

/* This is the hexadecimal AND function.
 *
 * Input:
 *   op1  = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  AND is BINARY.
 *
 * Output:
 *   true  = success.  op1 contains the result.
 *   false = failure.
 */
bool
operand_op_and(operand *op1,
               operand *op2)
{
  bool retcode = false;

  operand_base base1, base2;
  if((operand_get_base(op1, &base1) == true) && (base1 == operand_base_16) &&
     (operand_get_base(op2, &base2) == true) && (base2 == operand_base_16))
  {
    bool is_fp;
    int64_t  i1 = 0,   i2 = 0;
    double   f1 = 0.0, f2 = 0.0;

    if((retcode = operand_op_get_binary_ops(op1, op2, &is_fp, &i1, &f1, &i2, &f2)) == true)
    {
      i1 &= i2;
      retcode = operand_set_val(op1, is_fp, i1, f1);
    }
  }

  return retcode;
}

/* This is the hexadecimal OR function.
 *
 * Input:
 *   op1  = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  OR is BINARY.
 *
 * Output:
 *   true  = success.  op1 contains the result.
 *   false = failure.
 */
bool
operand_op_or(operand *op1,
              operand *op2)
{
  bool retcode = false;

  operand_base base1, base2;
  if((operand_get_base(op1, &base1) == true) && (base1 == operand_base_16) &&
     (operand_get_base(op2, &base2) == true) && (base2 == operand_base_16))
  {
    bool is_fp;
    int64_t  i1 = 0,   i2 = 0;
    double   f1 = 0.0, f2 = 0.0;

    if((retcode = operand_op_get_binary_ops(op1, op2, &is_fp, &i1, &f1, &i2, &f2)) == true)
    {
      i1 |= i2;
    }
  }

  return retcode;
}


/* This is the hexadecimal XOR function.
 *
 * Input:
 *   op1  = A pointer to the operand object.
 *
 *   op2  = The other operand.  XOR is BINARY.
 *
 * Output:
 *   true  = success.  op1 contains the result.
 *   false = failure.
 */
bool
operand_op_xor(operand *op1,
               operand *op2)
{
  bool retcode = false;

  operand_base base1, base2;
  if((operand_get_base(op1, &base1) == true) && (base1 == operand_base_16) &&
     (operand_get_base(op2, &base2) == true) && (base2 == operand_base_16))
  {
    bool is_fp;
    int64_t  i1 = 0,   i2 = 0;
    double   f1 = 0.0, f2 = 0.0;

    if((retcode = operand_op_get_binary_ops(op1, op2, &is_fp, &i1, &f1, &i2, &f2)) == true)
    {
      i1 ^= i2;
    }
  }

  return retcode;
}

/* This is the hexadecimal NOT function.
 *
 * Input:
 *   this = A pointer to the only important operand.  NOT is a UNARY operation.
 *
 * Output:
 *   true  = success.  this contains the result.
 *   false = failure.
 */
bool
operand_op_not(operand *this)
{
  bool retcode = false;

  operand_base base;
  if((operand_get_base(this, &base) == true) && (base == operand_base_16))
  {
    bool is_fp;
    int64_t  i;
    double   f;

    if((retcode = operand_get_val(this, &is_fp, &i, &f)) == true)
    {
      i = ~i;
    }
  }

  return retcode;
}
/******************************************************************************
 ********************************* PUBLIC API *********************************
 *****************************************************************************/

/* Create a new operand object.  This object can be used to access the operand
 * class.
 *
 * Input:
 *   base = The number base to use.
 *
 * Output:
 *   Returns a pointer to the object.
 *   Returns 0 if unable to create the object.
 */
operand *
operand_new(operand_base base)
{
  operand *this = malloc(sizeof(*this));

  if(this != (operand *) 0)
  {
    this->i_val             = 0LL;
    this->f_val             = 0.0;
    this->got_decimal_point = false;
    this->base              = base;
  }

  return this;
}

/* Delete an operand object that was created by operand_new().
 *
 * Input:
 *   this = A pointer to the operand object.
 *
 * Output:
 *   true  = success.
 *   false = failure.
 */
bool
operand_delete(operand *this)
{
  bool retcode = false;

  if(this != (operand *) 0)
  {
    free(this);
    retcode = true;
  }

  return retcode;
}

/* Get the current number base that the operand is configured for.
 *
 * Input:
 *   this = A pointer to the operand object.
 *
 *   base = A pointer to a variable that is set to the current base.
 *
 * Output:
 *   true  = success.  *base = the current base.
 *   false = failure.  *base = operand_base_unknown.
 */
bool
operand_get_base(operand *this,
                 operand_base *base)
{
  bool retcode = false;

  /* Check parameters and pick a reasonable default answer. */
  if(base != (operand_base *) 0)
  {
    if(this != (operand *) 0)
    {
      *base = this->base;
      retcode = true;
    }
    else
    {
      *base = operand_base_unknown;
    }
  }

  return retcode;
}

/* Set the number base that the operand should use.
 *
 * Input:
 *   this = A pointer to the operand object.
 *
 *   base = The new base that we should set.
 *
 * Output:
 *   true  = success.  The operand is now using the new base.
 *   false = failure.  The operand is NOT using the new base.  State unknown.
 */
bool
operand_set_base(operand *this,
                 operand_base base)
{
  bool retcode = false;

  if(this != (operand *) 0)
  {
    if(this->base != base)
    {
      switch(base)
      {
      case operand_base_10:
        this->base = base;
        retcode = true;
        break;
      
      case operand_base_16:
        if(this->base == operand_base_10)
        {
          if(this->got_decimal_point == true)
          {
            this->i_val = this->f_val;
            this->got_decimal_point = false;
            this->f_val = 0.0;
          }
        }
        this->base = base;
        retcode = true;
        break;

      default:
        /* We will return false. */
        break;
      }
    }
  }

  return retcode;
}

/* Attempt to add a character to an existing operand object.  The character is
 * first checked to see if it's a valid part of an operand.  If it's valid, it
 * is added.
 *
 * Input:
 *   this = A pointer to the operand object.
 *
 *   c = The character to add.  Note that it might not be a valid operand.
 *
 * Output:
 *   true  = success.  c is valid, and it has been added to this.
 *   false = failure.  c is NOT an operand OR we were unable to add c to this.
 */
bool
operand_add_char(operand *this,
                 char c)
{
  bool retcode = false;

  /* Get the base for the operand. */
  operand_base base;
  if(operand_get_base(this, &base) == true)
  {
    /* If it's a decimal point, prepare to start doing decimal math.  If we
     * already got a decimal point, then this one is silently dropped. */
    if((base == operand_base_10) && (c == '.'))
    {
      if(this->got_decimal_point == false)
      {
        this->f_val = this->i_val;
        this->i_val = 0;
        this->got_decimal_point = true;
        this->fp_multiplier = 0.10;
      }

      retcode = true;
    }

    /* An 'S' switches the +/- sign of the operand. */
    else if((c & 0xDF) == 'S')
    {
      /* Subtract the current value from 0.  That flips the sign.  It's the
       * same for integer and floating point. */
      if(this->got_decimal_point == false)
      {
        this->i_val = 0 - this->i_val;
      }
      else
      {
        this->f_val = 0.0 - this->f_val;
      }

      retcode = true;
    }

    /* Not a decimal point or sign.  It better be a digit. */
    else
    {
      /* If it's an ASCII digit, convert it to a binary. */
      unsigned char c_val = 0xFF;
      if((c >= '0') && (c <= '9'))
      {
        c_val = c - '0';
      }
      else if( (base == operand_base_16) && (((c & 0xDF) >= 'A') && ((c & 0xDF) <= 'F')) )
      {
        c_val = (c & 0xDF) - 0x37;
      }

      /* Is it as a digit?  If it is, then add it to the current value. */
      if(c_val != 0xFF)
      {
        switch(base)
        {
        case operand_base_16:
          this->i_val = (this->i_val * 16) + c_val;
          break;

        case operand_base_10:
        default:
          if(this->got_decimal_point == true)
          {
            double f = c_val;
            f *= this->fp_multiplier;
            this->f_val += f;
            this->fp_multiplier /= 10.0;
          }
          else
          {
            this->i_val = (this->i_val * 10) + c_val;
          }
          break;
        }

        retcode = true;
      }
    }
  }

  return retcode;
}

/* Create an ASCII string the represents the current value of the operand.
 *
 * Input:
 *   this     = A pointer to the operand object.
 *
 *   buf      = The caller-supplied buffer to build the string in.
 *
 *   buf_size = The size of buf.  Note that we must allow 1 byte for the NULL
 *              terminator.
 *
 * Output:
 *   true  = success.  buf contains the string.  Note that it might be
 *                     truncated if buf is too small to hold the entire size of
 *                     the string.
 *   false = failure.  buf is undefined.
 */
bool operand_to_str(operand *this,
                    char *buf,
                    size_t buf_size)
{
  bool retcode = false;

  bool     is_fp;
  int64_t  i_val;
  double   f_val;
  if((retcode = operand_get_val(this, &is_fp, &i_val, &f_val)) == true)
  {
    if(is_fp)
    {
      snprintf(buf, buf_size, "%f", f_val);

      /* Remove trailing zeroes. */
      
    }
    else
    {
      operand_base base;
      if((retcode = operand_get_base(this, &base)) == true)
      {
        if(base == operand_base_16)
        {
          snprintf(buf, buf_size, "%llX", (long long) i_val);
        }
        else
        {
          snprintf(buf, buf_size, "%lld", (long long) i_val);
        }
      }
    }
  }

  return retcode;
}

/******************************************************************************
 ********************************** TEST API **********************************
 *****************************************************************************/

#ifdef TEST
bool
operand_test(void)
{
  printf("%s():\n", __func__);

  /* Loop through some assorted operand patterns.  This tests the basic
   * functionality of the operand class.  We're checking to make sure it can
   * handle the types of numbers that we support. */
  typedef struct operand_test {
    const char  *str;
    operand_base base;
    bool         is_fp;
    int64_t      i_val;
    double       f_val;
  } operand_test;
  operand_test tests[] = {
    { "123",     operand_base_10, false,     123,    123.0   }, // Simple integer value.
    { "123000",  operand_base_10, false,  123000, 123000.0   }, // Integer with trailing zeroes.
    { "123.456", operand_base_10,  true,       0,    123.456 }, // Simple floating point value.
  };
  size_t operand_test_size = (sizeof(tests) / sizeof(operand_test));

  int x;
  for(x = 0; x < operand_test_size; x++)
  {
    operand_test *t = &tests[x];
    printf("  %s\n", t->str);

    DBG_PRINT("operand_new()\n");
    operand *this;
    if((this = operand_new(t->base)) == (operand *) 0)               return false;

    const char *str = t->str;
    while(*str)
    {
      DBG_PRINT("operand_add_char()\n");
      if(operand_add_char(this, *str++) != true)                     return false;
    }

    DBG_PRINT("operand_get_base()\n");
    operand_base base;
    if(operand_get_base(this, &base) != true)                        return false;
    if(base != t->base)                                              return false;

    DBG_PRINT("operand_get_val()\n");
    bool is_fp;
    int64_t i_val;
    double f_val;
    if(operand_get_val(this, &is_fp, &i_val, &f_val) != true)        return false;
    if(is_fp != t->is_fp)                                            return false;
    if(i_val != t->i_val)                                            return false;
    if(f_val != t->f_val)                                            return false;

    DBG_PRINT("operand_to_str()\n");
    char result[1024];
    if(operand_to_str(this, result, sizeof(result)) != true)         return false;
    DBG_PRINT("  str = '%s'.\n", result);
    if(strcmp(result, t->str) != 0)                                  return false;

    DBG_PRINT("operand_delete()\n");
    if(operand_delete(this) != true)                                 return false;
  }

  return true;
}
#endif // TEST


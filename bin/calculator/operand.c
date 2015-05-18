/* A class that can be used to store and manipulate numbers.  It is used
 * exclusively by the calculator class.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#include "operand.h"

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/* This is the operand class. */
struct operand {
  uint64_t i_val;
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

/* Load 2 operands.  This is used to prep for a BINARY operation.
 *
 * Input:
 *   op1   = A pointer to the 1st operand.
 *
 *   op2   = A pointer to the 2nd operand.
 *
 *   is_fp = Set to true if you need to do fp math.
 *
 *   i1    = Gets the int value from op1 if (if_fp == false).
 *
 *   f1    = Gets the floating point value from op1 if (if_fp == true).
 *
 *   i2    = Gets the int value from op2 if (if_fp == false).
 *
 *   f2    = Gets the floating point value from op2 if (if_fp == true).
 *
 * Output:
 *   true  = success.
 *   false = failure.
 */
static bool
operand_op_get_binary_ops(operand  *op1,
                          operand  *op2,
                          bool     *is_fp,
                          uint64_t *i1,
                          double   *f1,
                          uint64_t *i2,
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
                uint64_t  i_val,
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
  uint64_t i1 = 0,   i2 = 0;
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
  uint64_t i1 = 0,   i2 = 0;
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
  uint64_t i1 = 0,   i2 = 0;
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
  uint64_t i1 = 0,   i2 = 0;
  double   f1 = 0.0, f2 = 0.0;

  if((retcode = operand_op_get_binary_ops(op1, op2, &is_fp, &i1, &f1, &i2, &f2)) == true)
  {
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
    else
    {
      if(i2 == 0)
      {
        retcode = false;
      }
      else
      {
        i1 /= i2;
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
  uint64_t i1 = 0,   i2 = 0;
  double   f1 = 0.0, f2 = 0.0;

  if((retcode = operand_op_get_binary_ops(op1, op2, &is_fp, &i1, &f1, &i2, &f2)) == true)
  {
    if(is_fp == true)
    {
    }
    else
    {
      if(i2 == 0)
      {
        i1 = 1;
      }
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
    uint64_t i1 = 0,   i2 = 0;
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
    uint64_t i1 = 0,   i2 = 0;
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
    uint64_t i1 = 0,   i2 = 0;
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
    uint64_t i;
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

/* Return the current value of the specified operand object.
 *
 * Input:
 *   this  = A pointer to the operand object.
 *
 *   is_fp = A ptr to a bool that is set to true if the operand is a float.
 *
 *   i_val = A ptr to an integer.  It is set to the value of the object if
 *           is_fp == false.
 *
 *   f_val = A ptr to a float.  It is set to the value of the object if
 *           is_fp == true.
 *
 * Output:
 *   true  = success.
 *   false = failure.
 */
bool
operand_get_val(operand  *this,
                bool     *is_fp,
                uint64_t *i_val,
                double   *f_val)
{
  bool retcode = false;

  if(this != (operand *) 0)
  {
    if((*is_fp = this->got_decimal_point) == true)
    {
      *f_val = this->f_val;
    }
    else
    {
      *i_val = this->i_val;
    }

    retcode = true;
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
  bool retcode = false;

  operand *this = operand_new(operand_base_10);
  if(this != (operand *) 0)
  {
    operand_delete(this);
    retcode = true;
  }

  return retcode;
}
#endif // TEST


/* A class that can be used to store and manipulate operands.  It is used
 * exclusively by the calculator class.
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#include "operand.h"

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/* This is the operand class. */
struct operand {
  int    i_val;
  double d_val;
  bool   got_decimal_point;
  double fp_multiplier;

  /* The number system we're currently configured to use.  The allowed settings
   * are currently base_10 and base_16. */
  operand_base base;
};
  
/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

/******************************************************************************
 ********************************* PUBLIC API *********************************
 *****************************************************************************/

/* This is the addition function.
 *
 * Input:
 *   this = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  Addition is BINARY.
 *
 * Output:
 *   true  = successful.
 *   false = failure.
 */
bool
operand_op_add(operand *op1,
               operand *op2)
{
  bool retcode = false;

  bool   is_fp1,   is_fp2;
  int    i1 = 0,   i2 = 0;
  double f1 = 0.0, f2 = 0.0;

  if( (operand_get_val(op1, &is_fp1, &i1, &f1) == true) &&
      (operand_get_val(op2, &is_fp2, &i2, &f2) == true) )
  {
    if(is_fp1 || is_fp2)
    {
      f1 = is_fp1 ? f1 : i1;
      f2 = is_fp2 ? f2 : i2;
      op1->d_val = f1 + f2;
      op1->got_decimal_point = true;
      retcode = true;
    }
    else
    {
      op1->i_val += op2->i_val;
      retcode = true;
    }
  }

  return retcode;
}

/* This is the subtraction function.
 *
 * Input:
 *   this = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  Subtraction is BINARY.
 *
 * Output:
 *   true  = successful.
 *   false = failure.
 */
bool
operand_op_sub(operand *op1,
               operand *op2)
{
  bool retcode = false;

  bool   is_fp1,   is_fp2;
  int    i1 = 0,   i2 = 0;
  double f1 = 0.0, f2 = 0.0;

  if( (operand_get_val(op1, &is_fp1, &i1, &f1) == true) &&
      (operand_get_val(op2, &is_fp2, &i2, &f2) == true) )
  {
    if(is_fp1 || is_fp2)
    {
      f1 = is_fp1 ? f1 : i1;
      f2 = is_fp2 ? f2 : i2;
      op1->d_val = f1 - f2;
      op1->got_decimal_point = true;
      retcode = true;
    }
    else
    {
      op1->i_val -= op2->i_val;
      retcode = true;
    }
  }

  return retcode;
}

/* This is the multiplication function.
 *
 * Input:
 *   this = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  Multiplication is BINARY.
 *
 * Output:
 *   true  = successful.
 *   false = failure.
 */
bool
operand_op_mul(operand *op1,
               operand *op2)
{
  bool retcode = false;

  bool   is_fp1,   is_fp2;
  int    i1 = 0,   i2 = 0;
  double f1 = 0.0, f2 = 0.0;

  if( (operand_get_val(op1, &is_fp1, &i1, &f1) == true) &&
      (operand_get_val(op2, &is_fp2, &i2, &f2) == true) )
  {
    if(is_fp1 || is_fp2)
    {
      f1 = is_fp1 ? f1 : i1;
      f2 = is_fp2 ? f2 : i2;
      op1->d_val = f1 * f2;
      op1->got_decimal_point = true;
      retcode = true;
    }
    else
    {
      op1->i_val *= op2->i_val;
      retcode = true;
    }
  }

  return retcode;
}
/* This is the division function.
 *
 * Input:
 *   this = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  Division is BINARY.
 *
 * Output:
 *   true  = successful.
 *   false = failure.
 */
bool
operand_op_div(operand *op1,
               operand *op2)
{
  bool retcode = false;

  bool   is_fp1,   is_fp2;
  int    i1 = 0,   i2 = 0;
  double f1 = 0.0, f2 = 0.0;

  if( (operand_get_val(op1, &is_fp1, &i1, &f1) == true) &&
      (operand_get_val(op2, &is_fp2, &i2, &f2) == true) )
  {
    if(is_fp1 || is_fp2)
    {
      if(f2 != 0)
      {
        f1 = is_fp1 ? f1 : i1;
        f2 = is_fp2 ? f2 : i2;
        op1->d_val = f1 / f2;
        op1->got_decimal_point = true;
        retcode = true;
      }
    }
    else
    {
      if(op2->i_val != 0)
      {
        op1->i_val /= op2->i_val;
        retcode = true;
      }
    }
  }

  return retcode;
}

/* This is the exponentiation function.
 *
 * Input:
 *   this = A pointer to the first operand.  The result is returned in this one.
 *
 *   op2  = The other operand.  Exponentiation is BINARY.
 *
 * Output:
 *   true  = successful.
 *   false = failure.
 */
bool
operand_op_exp(operand *op1,
               operand *op2)
{
  bool retcode = false;

  bool is_fp1, is_fp2;
  int      i1,     i2;
  double   f1,     f2;

  operand_get_val(op1, &is_fp1, &i1, &f1);
  operand_get_val(op2, &is_fp2, &i2, &f2);

  printf("op1->i_val %d: op2->i_val %d.\n", op1->i_val, op2->i_val);
  if(op2->i_val == 0)
  {
    op1->i_val = 1;
    retcode = true;
  }
  else if(op2->i_val > 1)
  {
    int i;
    int res = op1->i_val;
    for(i = 1; i < op2->i_val; i++)
    {
      res *= op1->i_val;
    }
    op1->i_val = res;
    retcode = true;
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
 *   base = The number base (base_10 or base_16) to use.
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
    this->i_val             = 0;
    this->d_val             = 0.0;
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
 *   Returns 0 if successful.
 *   Returns 1 if not successful.
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
 * The current allowed bases are base_10 (decimal) and base_16 (hexadecimal).
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
 * The current allowed bases are base_10 (decimal) and base_16 (hexadecimal).
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
        if(this->base == operand_base_16)
        {
          this->got_decimal_point = false;
        }
        this->base = base;
        retcode = true;
        break;
      
      case operand_base_16:
        if(this->base == operand_base_10)
        {
          if(this->got_decimal_point == true)
          {
            this->i_val = this->d_val;
            this->got_decimal_point = false;
            this->d_val = 0.0;
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
 *   true  = c is part of an operand and it was added to this.
 *   false = c is NOT an operand OR unable to add c to this.
 */
bool
operand_add_char(operand *this,
                 char c)
{
  bool retcode = false;

  /* Get the base (base_10 or base_16) for the operand. */
  operand_base base;
  if(operand_get_base(this, &base) == false)
  {
    base = operand_base_10;
  }
  
  /* If it's a decimal point, prepare to start doing decimal math. */
  if((base == operand_base_10) && (c == '.'))
  {
    if(this->got_decimal_point == false)
    {
      this->d_val = this->i_val;
      this->i_val = 0;
      this->got_decimal_point = true;
      this->fp_multiplier = 0.10;
    }

    retcode = true;
  }

  /* Not a decimal point.  It better be a digit. */
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
          this->d_val += f;
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
 *   d_val = A ptr to a float.  It is set to the value of the object if
 *           is_fp == true.
 *
 * Output:
 *   true  = successful.
 *   false = unsuccessful.
 */
bool
operand_get_val(operand *this,
                bool    *is_fp,
                int     *i_val,
                double  *d_val)
{
  bool retcode = false;

  if(this != (operand *) 0)
  {
    if((*is_fp = this->got_decimal_point) != 0)
    {
      *d_val = this->d_val;
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

  operand *this = operand_new();
  if(this != (operand *) 0)
  {
    operand_delete(this);
    retcode = true;
  }

  return retcode;
}
#endif // TEST


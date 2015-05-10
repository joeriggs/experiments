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
};
  
/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

/* This function checks to see if a character is part of a numeric operand.  It
 * currently only supports decimal.
 *
 * Input:
 *   this = A pointer to the operand object.
 *
 *   c - This is the character to check.
 *
 * Output:
 *   Returns true if the character is part of a numeric operand.
 *   Returns false if the character is NOT part of a numeric operand.
 */
static bool
operand_is_valid(operand *this,
                 char c)
{
  return ((c >= '0') && (c <= '9')) ? true : false;
}

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

  bool is_fp1, is_fp2;
  int      i1,     i2;
  double   f1,     f2;

  operand_get_val(op1, &is_fp1, &i1, &f1);
  operand_get_val(op2, &is_fp2, &i2, &f2);

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

  bool is_fp1, is_fp2;
  int      i1,     i2;
  double   f1,     f2;

  operand_get_val(op1, &is_fp1, &i1, &f1);
  operand_get_val(op2, &is_fp2, &i2, &f2);

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

  bool is_fp1, is_fp2;
  int      i1,     i2;
  double   f1,     f2;

  operand_get_val(op1, &is_fp1, &i1, &f1);
  operand_get_val(op2, &is_fp2, &i2, &f2);

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

  bool is_fp1, is_fp2;
  int      i1,     i2;
  double   f1,     f2;

  operand_get_val(op1, &is_fp1, &i1, &f1);
  operand_get_val(op2, &is_fp2, &i2, &f2);

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
 *   N/A.
 *
 * Output:
 *   Returns a pointer to the object.
 *   Returns 0 if unable to create the object.
 */
operand *
operand_new(void)
{
  operand *this = malloc(sizeof(*this));

  if(this != (operand *) 0)
  {
    this->i_val             = 0;
    this->d_val             = 0.0;
    this->got_decimal_point = false;
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
 *   Returns 0 if c is part of an operand and it was added to this.
 *   Returns 1 if c is NOT an operand OR unable to add c to this.
 */
int
operand_add_char(operand *this,
                 char c)
{
  int retval = 1;

  /* Check to see if the character is a valid part of an operand.  If it is,
   * add it. */
  if(operand_is_valid(this, c) == true)
  {
    if(this->got_decimal_point == true)
    {
      double f = c - '0';
      f *= this->fp_multiplier;
      this->d_val += f;
      this->fp_multiplier /= 10.0;
    }
    else
    {
      this->i_val = (this->i_val * 10) + (c - '0');
    }

    retval = 0;
  }

  /* If it's a decimal point, prepare to start doing decimal math. */
  else if(c == '.')
  {
    if(this->got_decimal_point == false)
    {
      this->d_val = this->i_val;
      this->i_val = 0;
      this->got_decimal_point = true;
      this->fp_multiplier = 0.10;
    }

    retval = 0;
  }

  return retval;
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
 *   Returns 0 if successful.
 *   Returns 1 if unsuccessful.
 */
int
operand_get_val(operand *this,
                bool    *is_fp,
                int     *i_val,
                double  *d_val)
{
  int retval = 1;

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

    retval = 0;
  }

  return retval;
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


/* A class that can be used to store and manipulate numbers.  It is used
 * exclusively by the calculator class.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "bcd.h"
#include "fp_exp.h"
#include "hex.h"
#include "operand.h"

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/* This is the operand class. */
struct operand {

  /* This is the number when we're running in decimal mode. */
  bcd *decnum;

  /* This is the number when we're running in hexadecimal mode. */
  hex *hexnum;

  /* The number base we're currently configured to use.  This refers to things
   * like base_10 or base_16. */
  operand_base base;

  /* A newly-created operand object supports the operand_add_char() method.
   * After the object has been used in an equation calculation it is no longer
   * eligible for operand_add_char(). */
  bool add_char_allowed;
};
  
/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

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

  /* Both operands have to be of the same base.  We don't try to correct that
   * kind of problem. */
  if( (op1 != (operand *) 0) && (op2 != (operand *) 0) && (op1->base == op2->base) )
  {
    /* Pass the operation along to the class that knows how to deal with this
     * type of number. */
    switch(op1->base)
    {
    case operand_base_10:
      retcode = bcd_op_add(op1->decnum, op2->decnum);
      break;

    case operand_base_16:
      retcode = hex_op_add(op1->hexnum, op2->hexnum);
      break;

    default:
      break;
    }

    op1->add_char_allowed = op2->add_char_allowed = false;
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

  /* Both operands have to be of the same base.  We don't try to correct that
   * kind of problem. */
  if( (op1 != (operand *) 0) && (op2 != (operand *) 0) && (op1->base == op2->base) )
  {
    /* Pass the operation along to the class that knows how to deal with this
     * type of number. */
    switch(op1->base)
    {
    case operand_base_10:
      retcode = bcd_op_sub(op1->decnum, op2->decnum);
      break;

    case operand_base_16:
      retcode = hex_op_sub(op1->hexnum, op2->hexnum);
      break;

    default:
      break;
    }

    op1->add_char_allowed = op2->add_char_allowed = false;
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

  /* Both operands have to be of the same base.  We don't try to correct that
   * kind of problem. */
  if( (op1 != (operand *) 0) && (op2 != (operand *) 0) && (op1->base == op2->base) )
  {
    /* Pass the operation along to the class that knows how to deal with this
     * type of number. */
    switch(op1->base)
    {
    case operand_base_10:
      retcode = bcd_op_mul(op1->decnum, op2->decnum);
      break;

    case operand_base_16:
      retcode = hex_op_mul(op1->hexnum, op2->hexnum);
      break;
      break;

    default:
      break;
    }

    op1->add_char_allowed = op2->add_char_allowed = false;
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

  /* Both operands have to be of the same base.  We don't try to correct that
   * kind of problem. */
  if( (op1 != (operand *) 0) && (op2 != (operand *) 0) && (op1->base == op2->base) )
  {
    /* Pass the operation along to the class that knows how to deal with this
     * type of number. */
    switch(op1->base)
    {
    case operand_base_10:
      retcode = bcd_op_div(op1->decnum, op2->decnum);
      break;

    case operand_base_16:
      retcode = hex_op_div(op1->hexnum, op2->hexnum);
      break;

    default:
      break;
    }

    op1->add_char_allowed = op2->add_char_allowed = false;
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

  /* Both operands have to be of the same base.  We don't try to correct that
   * kind of problem. */
  if( (op1 != (operand *) 0) && (op2 != (operand *) 0) && (op1->base == op2->base) )
  {
    /* Pass the operation along to the class that knows how to deal with this
     * type of number. */
    switch(op1->base)
    {
    case operand_base_10:
      retcode = bcd_op_exp(op1->decnum, op2->decnum);
      break;

    case operand_base_16:
      break;

    default:
      break;
    }

    op1->add_char_allowed = op2->add_char_allowed = false;
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

  /* Both operands have to be of the same base.  We don't try to correct that
   * kind of problem. */
  if( (op1 != (operand *) 0) && (op2 != (operand *) 0) && (op1->base == op2->base) )
  {
    /* Pass the operation along to the class that knows how to deal with this
     * type of number. */
    switch(op1->base)
    {
    case operand_base_10:
      break;

    case operand_base_16:
      break;

    default:
      break;
    }

    op1->add_char_allowed = op2->add_char_allowed = false;
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

  /* Both operands have to be of the same base.  We don't try to correct that
   * kind of problem. */
  if( (op1 != (operand *) 0) && (op2 != (operand *) 0) && (op1->base == op2->base) )
  {
    /* Pass the operation along to the class that knows how to deal with this
     * type of number. */
    switch(op1->base)
    {
    case operand_base_10:
      break;

    case operand_base_16:
      break;

    default:
      break;
    }

    op1->add_char_allowed = op2->add_char_allowed = false;
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

  /* Both operands have to be of the same base.  We don't try to correct that
   * kind of problem. */
  if( (op1 != (operand *) 0) && (op2 != (operand *) 0) && (op1->base == op2->base) )
  {
    /* Pass the operation along to the class that knows how to deal with this
     * type of number. */
    switch(op1->base)
    {
    case operand_base_10:
      break;

    case operand_base_16:
      break;

    default:
      break;
    }

    op1->add_char_allowed = op2->add_char_allowed = false;
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

  if(this!= (operand *) 0)
  {
    /* Pass the operation along to the class that knows how to deal with this
     * type of number. */
    switch(this->base)
    {
    case operand_base_10:
      break;

    case operand_base_16:
      break;

    default:
      break;
    }

    this->add_char_allowed = false;
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
    this->base = base;
    this->add_char_allowed = true;

    if( ((this->decnum = bcd_new()) == (bcd *) 0) ||
        ((this->hexnum = hex_new()) == (hex *) 0) )
    {
      if(this->decnum != (bcd *) 0)
      {
        bcd_delete(this->decnum);
      }

      if(this->hexnum != (hex *) 0)
      {
        hex_delete(this->hexnum);
      }

      operand_delete(this);
      this = (operand *) 0;
    }
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
    retcode = bcd_delete(this->decnum);
    retcode = hex_delete(this->hexnum);

    free(this);
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
 *   false = failure.  *base = operand_base_unknown (if it's a valid pointer).
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

/* Check to see if the specified character is a valid operand character that
 * can be passed to operand_add_char().
 *
 * Input:
 *   base = The requested number base (base_10, base_16, etc).  We need this in
 *          order to know which base class to pass this request to.
 *
 *   c    = The character to check.
 *
 * Output:
 *   true  = Yes, c is a valid operand character.
 *   false = No, c is NOT a valid operand character.
 */
bool
operand_add_char_is_valid_operand(operand_base base,
                                  char c)
{
  bool retcode = false;

  /* Let the class for each number type decide if the character is valid. */
  switch(base)
  {
  case operand_base_10:
    retcode = bcd_add_char_is_valid_operand(c);
    break;

  case operand_base_16:
    retcode = hex_add_char_is_valid_operand(c);
    break;

  default:
    break;
  }

  return retcode;
}

/* Check to see if the operand class will allow character input.  Once an
 * operand has been used in an equation it is no longer eligible for character
 * input.
 *
 * Input:
 *   this = A pointer to the operand object.
 *
 * Output:
 *   true  = success.  The object will accept input via operand_add_char().
 *   false = failure.  The object is not eligible for character input.
 */
bool
operand_add_char_allowed(operand *this)
{
  bool retcode = false;

  if(this != (operand *) 0)
  {
    retcode = this->add_char_allowed;
  }

  return retcode;
}

/* Attempt to add a character to the operand object.
 *
 * Input:
 *   this = A pointer to the operand object.
 *
 *   c    = The char to add.
 *
 * Output:
 *   true  = success.  c was processed.  It's possible that it was dropped, but
 *                     at least it was a valid character.
 *   false = failure.  c is NOT an operand OR we were unable to add c to this.
 */
bool
operand_add_char(operand *this,
                 char c)
{
  bool retcode = false;

  if((this != (operand *) 0) && (this->add_char_allowed == true))
  {
    /* Get the base for the operand, and then pass the character to the
     * applicable ADT. */
    operand_base base;
    if(operand_get_base(this, &base) == true)
    {
      switch(base)
      {
      case operand_base_10:
        retcode = bcd_add_char(this->decnum, c);
        break;

      case operand_base_16:
        retcode = hex_add_char(this->hexnum, c);
        break;

      default:
        break;
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

  if(this != (operand *) 0)
  {
    switch(this->base)
    {
    case operand_base_10:
      retcode = bcd_to_str(this->decnum, buf, buf_size);
      break;

    case operand_base_16:
      retcode = hex_to_str(this->hexnum, buf, buf_size);
      break;

    default:
      break;
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
  } operand_test;
  operand_test tests[] = {
    { "123",     operand_base_10 }, // Simple integer value.
    { "123000",  operand_base_10 }, // Integer with trailing zeroes.
    { "123.456", operand_base_10 }, // Simple floating point value.
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


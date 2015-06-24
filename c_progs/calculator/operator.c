/* A class that can be used to evaluate C-style operators. It is used
 * exclusively by the calculator class.
 *
 * Refer to en.cppreference.com/w/c/language/operator_precedence for a good
 * list of C operator preference and associativity.
 */

#include <stdlib.h>

#include "common.h"

#include "operand.h"
#include "operator.h"

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/* This is the list of supported operators. */
typedef struct operator_property {
  const char             value;
  const char            *name;
  operator_type          op_type;
  int                    input_precedence;
  int                    stack_precedence;
  operand_binary_op      binary_op_exec;
  operand_unary_op       unary_op_exec;
} operator_property;
static operator_property operator_properties[] = {
  { '(',  "(",    op_type_none,    0, 99, 0, 0 }, // Open parentheses
  { ')',  ")",    op_type_none,   98,  0, 0, 0 }, // Open parentheses
  { '+',  "+",    op_type_binary,  9,  8, operand_op_add, 0 }, // Addition
  { '-',  "-",    op_type_binary,  9,  8, operand_op_sub, 0 }, // Subtraction
  { '*',  "*",    op_type_binary,  7,  6, operand_op_mul, 0 }, // Multiplication
  { '/',  "/",    op_type_binary,  7,  6, operand_op_div, 0 }, // Division
  { '^',  "^",    op_type_binary,  4,  5, operand_op_exp, 0 }, // Exponentiation
  { '&',  "AND",  op_type_binary, 17, 16, operand_op_and, 0 }, // Bitwise AND
  { '|',  "OR",   op_type_binary, 21, 20, operand_op_or,  0 }, // Bitwise OR
  { 'x',  "XOR",  op_type_binary, 19, 18, operand_op_xor, 0 }, // Bitwise XOR
  { '~',  "NOT",  op_type_unary,   3,  2, 0, operand_op_not }, // Bitwise Negate
  { '%',  "MOD",  op_type_binary,  7,  6, 0, 0 }, // Modulus
  { '<',  "SHL",  op_type_binary, 11, 10, 0, 0 }, // Shift Left
  { '>',  "SHR",  op_type_binary, 11, 10, 0, 0 }, // Shift Right
  { 'l',  "ROL",  op_type_binary, 11, 10, 0, 0 }, // Rotate Left
  { 'r',  "ROR",  op_type_binary, 11, 10, 0, 0 }, // Rotate Right
  {  0 ,  "",     op_type_none,    0,  0, 0, 0 }
};

/* This is the operator class.  Each object represents a single operator. */
struct operator {
  /* This is a pointer to the operator_property that defines the operator. */
  operator_property *op_prop;
};
  
/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

/* This function checks the operator and figures out which operator it is.
 *
 * Input:
 *   this = A pointer to the operator object.
 *
 *   c    = The one-character operator mnemonic.  All operators are one character.
 *
 * Output:
 *   true  = success.  this->op_prop points to the operator_property that
 *                     describes the operator.
 *   false = failure.  this->op_prop is unchanged.
 */
static bool
operator_set_op_prop(operator *this,
                     char c)
{
  bool retcode = false;

  if(this != (operator *) 0)
  {
    int i;
    for(i = 0; operator_properties[i].value != 0; i++) {
      if(c == operator_properties[i].value) {
        this->op_prop = &operator_properties[i];
        retcode = true;
        break;
      }
    }
  }

  return retcode;
}

/******************************************************************************
 ********************************* PUBLIC API *********************************
 *****************************************************************************/

/* Create a new operator object.  This object can be used to access the
 * operator class.
 *
 * Input:
 *   c = The one-character operator value.
 *
 * Output:
 *   Returns a pointer to the object.
 *   Returns 0 if unable to create the object.  For example, if c is not a
 *             recognized operator.
 */
operator *
operator_new(const char c)
{
  operator *this = malloc(sizeof(*this));

  if(operator_set_op_prop(this, c) == false)
  {
    free(this);
    this = (operator *) 0;
  }

  return this;
}

/* Delete an operator object that was created by operator_new().
 *
 * Input:
 *   this = A pointer to the operator object.
 *
 * Output:
 *   true  = success.  The object is deleted.
 *   false = failure.
 */
bool
operator_delete(operator *this)
{
  bool retcode = false;

  if(this != (operator *) 0)
  {
    free(this);
    retcode = true;
  }

  return retcode;
}

/* This function returns a value that represents the precedence of an operator.
 * Precedence deal with the order in which operators should be processed.
 *
 * The values are defined in operator_properties.  There are 2 precedence
 * values for each operator.  They are both returned to the caller.  The caller
 * knows how to use them.
 *
 * Input:
 *   this = A pointer to the operator object.
 *
 *   input = A pointer to a variable to will receive the input precedence.
 *
 *   stack = A pointer to a variable to will receive the stack precedence.
 *
 * Output:
 *   true  = success.  *input and *stack are set appropriately.
 *   false = failure.  *input and *stack are set to 0 (if they are valid ptrs).
 */
bool
operator_precedence(operator *this,
                    int *input,
                    int *stack)
{
  bool retcode = false;

  /* Make sure the parameters are valid, and set default values. */
  if(input != (int *) 0)
  {
    *input = 0;
  }
  if(stack != (int *) 0)
  {
    *stack = 0;
  }

  if( (this != (operator *) 0) && (this->op_prop != (operator_property *) 0) )
  {
    *input = this->op_prop->input_precedence;
    *stack = this->op_prop->stack_precedence;

    retcode = true;
  }

  return retcode;
}

/* Return the name of the operator object.  This refers to an ASCII string that
 * describes the operator.  A lot of operators (like +, -, *, /) make sense,
 * but some don't.  So we provide a name that the user will understand.
 *
 * Input:
 *   this    = A pointer to the operator object.
 *
 *   op_name = A ptr to a (char *) that will be set to the name of the operator.
 *
 * Output:
 *   true  = success. *op_name points to the name of the operator.
 *   false = failure. *op_name points to "" (if it is a valid pointer).
 */
bool
operator_get_name(operator *this,
                  const char **op_name)
{
  bool retcode = false;

  if(op_name != (const char **) 0)
  {
    if( (this != (operator *) 0) && (this->op_prop != (operator_property *) 0) )
    {
      *op_name = this->op_prop->name;
      retcode = true;
    }
    else
    {
      *op_name = (const char *) "";
    }
  }

  return retcode;
}

/* Return the operator type (unary or binary).
 *
 * Input:
 *   this = A pointer to the operator object.
 *
 * Output:
 *   true  = success.  *type is set to the operand type.
 *   false = failure.  *type is set to op_type_none (if it is a valid pointer).
 */
bool
operator_get_op_type(operator *this,
                     operator_type *type)
{
  bool retcode = false;

  if(type != (operator_type *) 0)
  {
    if(this != (operator *) 0)
    {
      *type = this->op_prop->op_type;
      retcode = true;
    }
    else
    {
      *type = op_type_none;
    }
  }

  return retcode;
}

/* Perform a unary operation.
 *
 * Input:
 *   this = A pointer to the operator object.
 *
 *   op   = A ptr to the operand.  The result will be stored in op.
 *
 * Output:
 *   true  = success.  The result is in op.
 *   false = failure.  op is undefined.
 */
bool
operator_do_unary(operator *this,
                  operand *op)
{
  bool retcode = false;

  if(this->op_prop->unary_op_exec != (operand_unary_op) 0)
  {
    retcode = this->op_prop->unary_op_exec(op);
  }

  return retcode;
}

/* Perform a binary operation.
 *
 * Input:
 *   this = A pointer to the operator object.
 *
 *   op1  = A ptr to operand1.  The result will be stored in op1.
 *
 *   op2  = A ptr to operand2.
 *
 * Output:
 *   true  = success.  The result is in op1.
 *   false = failure.
 */
bool
operator_do_binary(operator *this,
                   operand *op1,
                   operand *op2)
{
  bool retcode = false;

  if(this->op_prop->binary_op_exec != (operand_binary_op) 0)
  {
    retcode = this->op_prop->binary_op_exec(op1, op2);
  }

  return retcode;
}

/******************************************************************************
 ********************************** TEST API **********************************
 *****************************************************************************/

#ifdef TEST
bool
operator_test(void)
{
  bool retcode = false;

  operator *this = operator_new('+');
  if(this != (operator *) 0)
  {
    operator_delete(this);
    retcode = true;
  }

  return retcode;
}
#endif // TEST


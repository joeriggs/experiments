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

/* This a the list of operator types.  It deals specifically with how many
 * operands are required. */
typedef enum {
  op_type_none,
  op_type_unary,
  op_type_binary
} operator_type;

/* This is the list of supported operators. */
typedef struct operator_property {
  const char             value;
  const char            *name;
  operator_type          op_type;
  int                    input_precedence;
  int                    stack_precedence;
  operand_binary_op      op_exec;
} operator_property;
static operator_property operator_properties[] = {
  { '(',  "(",    op_type_none,    0, 99, 0 }, // Open parentheses
  { ')',  ")",    op_type_none,   98,  0, 0 }, // Open parentheses
  { '+',  "+",    op_type_binary,  9,  8, operand_op_add }, // Addition
  { '-',  "-",    op_type_binary,  9,  8, operand_op_sub }, // Subtraction
  { '*',  "*",    op_type_binary,  7,  6, operand_op_mul }, // Multiplication
  { '/',  "/",    op_type_binary,  7,  6, operand_op_div }, // Division
  { '^',  "^",    op_type_binary,  4,  5, operand_op_exp }, // Exponentiation
  { '&',  "AND",  op_type_binary, 17, 16, 0 }, // Bitwise AND
  { '|',  "OR",   op_type_binary, 21, 20, 0 }, // Bitwise OR
  { 'x',  "XOR",  op_type_binary, 19, 18, 0 }, // Bitwise XOR
  { '~',  "NOT",  op_type_unary,   3,  2, 0 }, // Bitwise Negate
  { '%',  "MOD",  op_type_binary,  7,  6, 0 }, // Modulus
  { '<',  "SHL",  op_type_binary, 11, 10, 0 }, // Shift Left
  { '>',  "SHR",  op_type_binary, 11, 10, 0 }, // Shift Right
  { 'l',  "ROL",  op_type_binary, 11, 10, 0 }, // Rotate Left
  { 'r',  "ROR",  op_type_binary, 11, 10, 0 }, // Rotate Right
  {  0 ,  "",     op_type_none,    0,  0, 0 }
};

/* This is the operator class. */
struct operator {
  /* This is a pointer to the operator_property that defines the operator. */
  operator_property *op_prop;
};
  
/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

/* This function checks to see if the next thing in the equation is a binary
 * operator.  A binary operator requires 2 operands (ex. "1 + 2").
 *
 * Input:
 *   c - The character to check.  All operators are one character.
 *
 * Output:
 *   Returns a pointer to the operator_property that describes the operator.
 *   Returns 0 if c isn't an operator.
 */
static operator_property
*is_binary_operator(char c)
{
  operator_property *retval = (operator_property *) 0;

  int i;
  for(i = 0; operator_properties[i].value != 0; i++) {
    if(c == operator_properties[i].value) {
      retval = &operator_properties[i];
      break;
    }
  }

  return retval;
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
 *   Returns 0 if unable to create the object.
 */
operator *
operator_new(const char c)
{
  operator *this = malloc(sizeof(*this));

  if((this->op_prop = is_binary_operator(c)) == (operator_property *) 0)
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
 *   Returns 0 if successful.
 *   Returns 1 if not successful.
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

/* This function returns a value that represents the priority of an operator.
 *
 * Input:
 *   this = A pointer to the operator object.
 *
 *   input = A pointer to a variable to will receive the input precedence.
 *
 *   stack = A pointer to a variable to will receive the stack precedence.
 *
 * Output:
 *   Returns 0 if successful.
 *   Returns 1 on failure.
 */
int
operator_precedence(operator *this,
                    int *input,
                    int *stack)
{
  int retcode = 1;

  if(this != (operator *) 0)
  {
    *input = this->op_prop->input_precedence;
    *stack = this->op_prop->stack_precedence;

    retcode = 0;
  }

  return retcode;
}

/* Return the name of the operator object.
 *
 * Input:
 *   this    = A pointer to the operator object.
 *
 *   op_name = A ptr to a (char *) that will get the name of the operator.
 *
 * Output:
 *   Returns 0 if successful.
 *   Returns 1 if unsuccessful.
 */
int
operator_get_name(operator *this,
                  const char **op_name)
{
  int retval = 1;

  if(this != (operator *) 0)
  {
    *op_name = this->op_prop->name;
    retval = 0;
  }

  return retval;
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
 *   true  = successful.
 *   false = unsuccessful.
 */
bool
operator_do_binary(operator *this,
                   operand *op1,
                   operand *op2)
{
  bool retcode = false;

  if(this->op_prop->op_exec != (operand_binary_op) 0)
  {
    retcode = this->op_prop->op_exec(op1, op2);
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


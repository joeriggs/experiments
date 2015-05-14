/* A class that can be used to evaluate C-style operators.
 */

/****************************** CLASS DEFINITION ******************************/

/* This a the list of operator types.  It deals specifically with how many
 * operands are required.
 *
 * Unary operators require 1 operand.
 * Binary operators require 2 operands.
 */
typedef enum {
  op_type_none,
  op_type_unary,
  op_type_binary
} operator_type;

typedef struct operator operator;

/********************************* PUBLIC API *********************************/

operator *operator_new(const char c);

int operator_precedence(operator *this, int *input, int *stack);

int operator_get_name(operator *this, const char **op_name);

bool operator_get_op_type(operator *this, operator_type *type);

bool operator_do_unary(operator *this, operand *op);

bool operator_do_binary(operator *this, operand *op1, operand *op2);

/********************************** TEST API **********************************/

bool operator_test(void);


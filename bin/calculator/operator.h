/* A class that can be used to evaluate C-style operators.
 */

/****************************** CLASS DEFINITION ******************************/

typedef struct operator operator;

/********************************* PUBLIC API *********************************/

operator *operator_new(const char c);

int operator_precedence(operator *this, int *input, int *stack);

int operator_get_name(operator *this, const char **op_name);

bool operator_do_binary(operator *this, operand *op1, operand *op2);

/********************************** TEST API **********************************/

bool operator_test(void);


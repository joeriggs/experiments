/* A class that can be used to store and manipulate operands.  It is used
 * exclusively by the calculator class.
 */

/****************************** CLASS DEFINITION ******************************/

typedef enum {
  operand_base_10,
  operand_base_16,
  operand_base_unknown
} operand_base;

typedef struct operand operand;

/********************************* PUBLIC OPS *********************************/

typedef bool (*operand_binary_op)(operand *op1, operand *op2);

bool operand_op_add(operand *op1, operand *op2);
bool operand_op_sub(operand *op1, operand *op2);
bool operand_op_mul(operand *op1, operand *op2);
bool operand_op_div(operand *op1, operand *op2);
bool operand_op_exp(operand *op1, operand *op2);

/********************************* PUBLIC API *********************************/

operand *operand_new(operand_base base);

bool operand_delete(operand *this);

bool operand_get_base(operand *this, operand_base *base);

bool operand_set_base(operand *this, operand_base base);

bool operand_add_char(operand *this, char c);

bool operand_get_val(operand *this, bool *is_fp, int *i_val, double *d_val);

/********************************** TEST API **********************************/

bool operand_test(void);


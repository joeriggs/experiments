/* This is a BCD (Binary Coded Decimal) implementation.  We need to be able to
 * do decimal math (float and double won't suffice), so this ADT provides that
 * capability.
 */

#ifndef __BCD_H__
#define __BCD_H__

/****************************** CLASS DEFINITION ******************************/

typedef struct bcd bcd;

/********************************* PUBLIC OPS *********************************/

bool bcd_op_add(bcd *op1, bcd *op2);
bool bcd_op_sub(bcd *op1, bcd *op2);
bool bcd_op_mul(bcd *op1, bcd *op2);
bool bcd_op_div(bcd *op1, bcd *op2);
bool bcd_op_exp(bcd *op1, bcd *op2);

/********************************* PUBLIC API *********************************/

bcd *bcd_new(void);

bool bcd_delete(bcd *this);

bool bcd_add_char_is_valid_operand(char c);

bool bcd_add_char(bcd *this, char c);

bool bcd_to_str(bcd *this, char *buf, size_t buf_size);

bool bcd_copy(bcd *src, bcd *dst);

int bcd_cmp(bcd *obj1, bcd *obj2);

bool bcd_import(bcd *this, int64_t src);

bool bcd_export(bcd *this, int64_t *dst);

/********************************** TEST API **********************************/

#if defined(TEST)

bool bcd_test(void);

#endif // TEST

#endif // __BCD_H__


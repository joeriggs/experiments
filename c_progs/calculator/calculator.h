/* An implementation of a simple math calculator.
 *
 * Refer to "Data Structures and Problem Solving in C++" by Mark Allen Weiss
 * for a really good description of an infix-to-postfix calculator.
 */
#ifndef __CALCULATOR_H__
#define __CALCULATOR_H__

#include "operand.h"

/****************************** CLASS DEFINITION ******************************/

typedef struct calculator calculator;

/********************************* PUBLIC API *********************************/

calculator *calculator_new(void);

bool calculator_delete(calculator *this);

bool calculator_get_operand_base(calculator *this, operand_base *cur_base);

bool calculator_set_operand_base(calculator *this, operand_base  new_base);

bool calculator_add_char(calculator *this, char c);

bool calculator_get_console(calculator *this, char *buf, size_t buf_size);

/********************************** TEST API **********************************/

#if defined(TEST)

bool calculator_test(void);

#endif // TEST

#endif // __CALCULATOR_H__

/* An implementation of a simple math calculator.
 *
 * Refer to "Data Structures and Problem Solving in C++" by Mark Allen Weiss
 * for a really good description of an infix-to-postfix calculator.
 */

/****************************** CLASS DEFINITION ******************************/

typedef enum {
  calculator_base_10,
  calculator_base_16,
  calculator_base_unknown
} calculator_base;

typedef struct calculator calculator;

/********************************* PUBLIC API *********************************/

calculator *calculator_new(void);

bool calculator_delete(calculator *this);

bool calculator_get_base(calculator *this, calculator_base *base);

bool calculator_set_base(calculator *this, calculator_base base);

bool calculator_add_char(calculator *this, char c);

bool calculator_get_console(calculator *this, char *buf, size_t buf_size);

/********************************** TEST API **********************************/

bool calculator_test(void);


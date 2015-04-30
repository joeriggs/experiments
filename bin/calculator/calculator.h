/* An implementation of a simple math calculator.
 *
 * Refer to "Data Structures and Problem Solving in C++" by Mark Allen Weiss
 * for a really good description of an infix-to-postfix calculator.
 */

typedef struct calculator calculator;

calculator *calculator_new(void);

const char *calculator_add_char(calculator *this, char c);


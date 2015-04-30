/* An implementation of a simple math calculator.
 *
 * Refer to "Data Structures and Problem Solving in C++" by Mark Allen Weiss
 * for a really good description of an infix-to-postfix calculator.
 *
 * This is currently a work-in-progress.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "calculator.h"
#include "stack.h"

/* Boolean type. */
typedef enum { false, true }  bool;

/* Maximum supported sizes for the calculator. */
#define MAX_INFIX_SIZE 1024
#define MAX_POSTFIX_SIZE 1024

/* ************************************************************************** */
/* ************************************************************************** */

/* This is the list of supported operators. */
typedef struct operator {
  const char value;
  const char *name;
  bool is_binary;
} operator;
static operator operators[] = {
  { '+',  "ADD", true  }, // Addition
  { '-',  "SUB", true  }, // Subtraction
  { '*',  "MUL", true  }, // Multiplication
  { '/',  "DIV", true  }, // Division
  { '&',  "AND", true  }, // Bitwise AND
  { '|',  "OR",  true  }, // Bitwise OR
  { '^',  "XOR", true  }, // Bitwise XOR
  { '~',  "NOT", false }, // Bitwise Negate
  { '%',  "MOD", true  }, // Modulus
  { '<',  "SHL", true  }, // Shift Left
  { '>',  "SHR", true  }, // Shift Right
  { 'l',  "ROL", true  }, // Rotate Left
  { 'r',  "ROR", true  }, // Rotate Right
  { 0, "", false }
};

/* This is the calculator class. */
struct calculator {
  char infix_str[MAX_INFIX_SIZE];
  char postfix_str[MAX_POSTFIX_SIZE];
  unsigned int allowed;
};
  
/* ************************************************************************** */
/* ************************************************************************** */

/* This function checks to see if a character is part of a numeric operand.  It
 * currently only supports decimal.
 *
 * Input:
 *   c - This is the character to check.
 *
 * Output:
 *   Returns true if the character is part of a numeric operator.
 *   Returns false if the character is NOT part of a numeric operator.
 */
static bool is_operand(char c)
{
  bool retcode;

  switch(c)
  {
  case '.':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    retcode = true;
    break;

  default:
    retcode = false;
    break;
  }

  return retcode;
}

/* This function checks to see if the next thing in the equation is a binary
 * operator.  A binary operator requires 2 operands (ex. "1 + 2").
 *
 * Input:
 *   c - The character to check.  In our internal calculator, all operators are
 *       one character.
 *
 * Output:
 *   Returns an object that describes the operator.
 *   Returns 0 if str doesn't point to an operator.
 */
static operator *is_binary_operator(char c)
{
  struct operator *retval = (struct operator *) 0;

  int i;
  for(i = 0; operators[i].value != 0; i++) {
    if(c == operators[i].value) {
      retval = &operators[i];
      break;
    }
  }

  return retval;
}

/* This function returns a value that represents the priority of an operator.
 *
 * Input:
 *   c - The operator to check.
 *
 * Output:
 *   Returns a numeric value that represents the relative priority of the
 *   operand.
 */
static int
operator_priority(char c)
{
  int retcode = 0;

  switch(c) {
  case '(': retcode = 0; break;
  case '*': retcode = 2; break;
  case '/': retcode = 2; break;
  case '+': retcode = 1; break;
  case '-': retcode = 1; break;
  }

  return retcode;
}

/* This function evaluates the postfix equation and returns the result.
 *
 * Input:
 *   this = A pointer to the calculator object.
 *
 * Output:
 *   0 = success.  The result is stored in the object.
 *   1 = failure.  No result is available.
 */
static int
calculator_postfix(calculator *this)
{
  int retcode = 0;
  int doing_operand = 0;
  char operand_val = 0;

  int postfix_index = 0;
  while(this->postfix_str[postfix_index]) {

    char c = this->postfix_str[postfix_index++];

    /* Skip white space. */
    if(c == ' ') {
      if(doing_operand)
      {
        push(operand_val);
        doing_operand = 0;
      }
      continue;
    }

    /* Process operands. */
    if(is_operand(c) == true) {
      doing_operand = 1;
      operand_val = (c - 0x30);
      continue;
    }

    /* Everything else is an operator. */
    else if(is_binary_operator(c) != (operator *) 0) {
      char o1, o2;
      pop(&o1);
      pop(&o2);
      switch(c)
      {
      case '+':
        o1 += o2;
        break;
      case '*':
        o1 *= o2;
        break;
      }
      push(o1);
    }
  }

  char result;
  pop(&result);
  printf("result = %d.\n", result);
  return retcode;
}

/* This function is the workhorse of the class.  It converts an infix equation
 * to postfix.
 *
 * Input:
 *   this = A pointer to the calculator object.
 *
 * Output:
 *   0 = success.  You can feed the postfix str into the postfix evaluator.
 *   1 = failure.
 */
static int
calculator_infix2postfix(calculator *this)
{
  int retcode = 0;
  int infix_index = 0;
  int postfix_index = 0;

  char a;

  while(this->infix_str[infix_index]) {

    char c = this->infix_str[infix_index];
    infix_index++;

    /* Skip white space. */
    if(c == ' ') {
      continue;
    }

    /* Process operands. */
    if(is_operand(c) == true) {
      this->postfix_str[postfix_index++] = c;
      continue;
    }

    /* Processs open parentheses. */
    if(c == '(') {
      if(push(c)) {
        printf("Line %d: Failed to push %c.\n", c, __LINE__);
        return 1;
      }
    }

    /* Process closing parentheses. */
    else if(c == ')') {
      while((peek(&a) == 0) && (a != '(')) {
        if(pop(&a)) {
          printf("Line %d: Failed to pop.\n", __LINE__);
          return 1;
        }

        /* Add the operator to the postfix. */
        this->postfix_str[postfix_index++] = ' ';
        this->postfix_str[postfix_index++] = a;
        this->postfix_str[postfix_index++] = ' ';
      }

      /* Pop off the opening parentheses. */
      pop(&a);
    }

    /* Everything else is an operator. */
    else if(is_binary_operator(c) != (operator *) 0) {
      this->postfix_str[postfix_index++] = ' ';

      /* Keep popping operators until you hit one that is a lower priority. */
      while((peek(&a) == 0) && (operator_priority(a) >= operator_priority(c))) {
        if(pop(&a)) {
          printf("Line %d: Failed to pop.\n", __LINE__);
          return 1;
        }

        /* Add the operator to the postfix. */
        this->postfix_str[postfix_index++] = ' ';
        this->postfix_str[postfix_index++] = a;
        this->postfix_str[postfix_index++] = ' ';
      }

      /* Push the operator after we're done popping. */
      if(push(c)) {
        printf("Line %d: Failed to push.\n", __LINE__);
        return 1;
      }
    }
  }

  /* When we're done, pop the rest of the stack and add it to the postfix. */
  while(pop(&a) == 0) {
    this->postfix_str[postfix_index++] = ' ';
    this->postfix_str[postfix_index++] = a;
  }

  return retcode;
}

/******************************************************************************
 ********************************* PUBLIC API *********************************
 *****************************************************************************/

/* Create a new calculator object.  This object can be used to access the
 * calculator class.
 *
 * Input:
 *   N/A
 *
 * Output:
 *   Returns a pointer to the object.
 *   Returns 0 if unable to create the object.
 */
calculator *
calculator_new(void)
{
  calculator *this = malloc(sizeof(*this));
  return this;
}


/* Add a character to the current equation.  As the user enters their equation
 * the data is passed to the calculator object via this member.
 *
 * Input:
 *   this = A pointer to the calculator object.
 *   c = The character to add.  Note that we only support 8-bit characters.
 *
 * Output:
 *   Returns a pointer the current value that should be displayed on the
 *   calculator's screen.
 */
const char *
calculator_add_char(calculator *this,
                    const char c)
{
  int curlen = strlen(this->infix_str);

  switch(c)
  {
    /* Backspace. */
    case 0x7F:
    case 0x08:
      if(curlen > 0)
      {
        this->infix_str[curlen - 1] = 0;
      }
      break;

    case '=':
      {
        bool res = calculator_infix2postfix(this);
        calculator_postfix(this);
      }
      break;

    default:
      {
        if(curlen < sizeof(this->infix_str) - 1)
        {
          this->infix_str[curlen] = c;
          calculator_infix2postfix(this);
          printf("\n%c -> %s\n", c, this->postfix_str);
        }
      }
      break;
  }

  return this->infix_str;
}


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

#include "common.h"

#include "calculator.h"
#include "list.h"
#include "operand.h"
#include "operator.h"
#include "stack.h"

/* These define the types of objects that we'll store in the list. */
#define LIST_OBJ_TYPE_OPERAND  1
#define LIST_OBJ_TYPE_OPERATOR 2
#define LIST_OBJ_TYPE_NONE     3

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/* This is the calculator class. */
struct calculator {
  /* As we collect operands and operators, we store them on a list. */
  list *infix_list;

  /* We convert infix-to-postfix in this list. */
  list *postfix_list;

  /* A buffer that's used to build the console output. */
  char console_buf[1024];
};
  
/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

/* This function evaluates the postfix equation and returns the result.
 *
 * Input:
 *   this = A pointer to the calculator object.  The postfix equation is
 *          already stored in postfix_list.
 *
 * Output:
 *   true  = success.  The result is stored in the object.
 *   false = failure.  No result is available.
 */
static bool
calculator_postfix(calculator *this)
{
  bool retcode = true;

  /* A stack to use while processing the postfix. */
  stack *tmp_stack;
  if((tmp_stack = stack_new()) == (stack *) 0)
  {
    printf("Unable to create postfix stack.\n");
  }

  else
  {
    /* These are used while processing the infix equation. */
    void *cur_obj;
    int type;
    while(list_rem_head(this->postfix_list, &cur_obj, &type) == true)
    {
      /* Operands immediately go onto the stack. */
      if(type == LIST_OBJ_TYPE_OPERAND)
      {
        stack_push(tmp_stack, cur_obj);
      }

      /* Operators are immediately processed. */
      else if(type == LIST_OBJ_TYPE_OPERATOR)
      {
        operand *op1,  *op2;
        stack_pop(tmp_stack, &op2);
        stack_pop(tmp_stack, &op1);

        if((retcode = operator_do_binary(cur_obj, op1, op2)) == false)
        {
          break;
        }
        stack_push(tmp_stack, op1);
      }

      else
      {
        printf("%s(): ERROR ERROR ERROR.\n", __func__);
      }
    }

    /* If the equation was completed successful, then the result is on the
     * stack.  Move it to the infix_list so we can start the next equation. */
    if(retcode == true)
    {
      operand *result;
      bool   is_fp;
      int    i_val;
      double f_val;
      stack_pop(tmp_stack, &result);
      operand_get_val(result, &is_fp, &i_val, &f_val);
      list_add_tail(this->infix_list, result, LIST_OBJ_TYPE_OPERAND);
    }

    stack_delete(tmp_stack);
  }

  return retcode;
}

/* This function is the workhorse of the class.  It converts an infix equation
 * to postfix.
 *
 * Input:
 *   this = A pointer to the calculator object.  The infix equation is already
 *          stored in infix_list.
 *
 * Output:
 *   true = success.  You can feed the postfix list into the postfix evaluator.
 *   false = failure.
 */
static bool
calculator_infix2postfix(calculator *this)
{
  bool retcode = true;

  /* A stack to use while converting infix to postfix. */
  stack *tmp_stack;
  if((tmp_stack = stack_new()) == (stack *) 0)
  {
    printf("Unable to create infix2postfix stack.\n");
    retcode = false;
  }

  else
  {
    if((this->postfix_list = list_new()) == (list *) 0)
    {
      printf("Unable to create infix2postfix list.\n");
      stack_delete(tmp_stack);
      retcode = false;
    }

    else
    {
      /* Used while processing the infix equation. */
      void *cur_obj;
      int type;
      operator *stk_operator;

      while((retcode == true) && (list_rem_head(this->infix_list, &cur_obj, &type) == true))
      {
        /* Operands go straight to the postfix equation. */
        if(type == LIST_OBJ_TYPE_OPERAND)
        {
          retcode = list_add_tail(this->postfix_list, cur_obj, LIST_OBJ_TYPE_OPERAND);
          continue;
        }

        /* Operators are a little trickier. */
        else if(type == LIST_OBJ_TYPE_OPERATOR)
        {
          operator *cur_operator = (operator *) cur_obj;

          /* Keep popping operators until we encounter an operator that is a lower
           * precedence or we hit the bottom of the stack. */
          while(stack_peek(tmp_stack, &stk_operator) == true)
          {
            int cur_input, cur_stack;
            int stk_input, stk_stack;
            if( (operator_precedence(cur_operator, &cur_input, &cur_stack) == 0) &&
                (operator_precedence(stk_operator, &stk_input, &stk_stack) == 0) )
            {
              if(stk_stack > cur_input)
              {
                break;
              }

              if(stack_pop(tmp_stack, &stk_operator) == false)
              {
                printf("Line %d: Failed to pop.\n", __LINE__);
                break;
              }

              /* Add the operator to the postfix. */
              retcode = list_add_tail(this->postfix_list, stk_operator, LIST_OBJ_TYPE_OPERATOR);
            }
            else
            {
              printf("Unable to get precedence information for operator(s).\n");
              break;
            }
          }

          /* Push the new operator after we're done popping. */
          if(retcode == true)
          {
            retcode = stack_push(tmp_stack, cur_operator);
          }
        }

        /* Unknown object type. */
        else
        {
          printf("Unknown data type (%d).\n", type);
        }
      }

      /* When we're done, pop the rest of the stack and add it to the postfix. */
      while((retcode == true) && (stack_pop(tmp_stack, &stk_operator) == true))
      {
        retcode = list_add_tail(this->postfix_list, stk_operator, LIST_OBJ_TYPE_OPERATOR);
      }
    }

    stack_delete(tmp_stack);

    retcode = true;
  }

  return retcode;
}

/* This is the callback function for a list traversal.  When we want to create
 * an ASCII string that contains the entire infix or postfix equation, we call
 * the list::list_traverse() member, and it passes each operand/operator object
 * to this member.  This member then builds a string that contains the ASCII
 * equation.
 *
 * Input:
 *   this_void = A pointer to the calculator object.
 *
 *   object    = An opaque value that represents an object.
 *
 *   type      = This defines what type of object is contained in object.  We
 *               use the type to help us identify object.
 *
 * Output:
 *   N/A.
 */
static void
calculator_list_traverse_cb(const void *ctx,
                            const void *object,
                            int type)
{
  calculator *this = (calculator *) ctx;

  if(this != (calculator *) 0)
  {
    size_t buf_len      = strlen(this->console_buf);
    size_t buf_dst_max  = sizeof(this->console_buf) - buf_len;
    char  *buf_dst      = &this->console_buf[buf_len];
    bool   buf_is_empty = (buf_len == 0);

    switch(type)
    {
      case LIST_OBJ_TYPE_OPERAND:
        {
          operand *cur_operand = (operand *) object;
          bool   is_fp;
          int    i_val;
          double f_val;
          operand_get_val(cur_operand, &is_fp, &i_val, &f_val);

          if(is_fp)
          {
            snprintf(buf_dst, buf_dst_max, buf_is_empty ? "%f" : " %f", f_val);
          }
          else
          {
            snprintf(buf_dst, buf_dst_max, buf_is_empty ? "%d" : " %d", i_val);
          }
        }
        break;

      case LIST_OBJ_TYPE_OPERATOR:
        {
          operator *cur_operator = (operator *) object;
          const char *op_name;
          operator_get_name(cur_operator, &op_name);
          snprintf(buf_dst, buf_dst_max, buf_is_empty ? "%s" : " %s", op_name);
        }
        break;

      default:
        printf("UNKNOWN: \n");
        break;
    }
  }
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

  /* Initialize. */
  if(this != (calculator *) 0)
  {
    if((this->infix_list = list_new()) == (list *) 0)
    {
      free(this);
      this = (calculator *) 0;
    }
  }

  return this;
}

/* Delete a calculator object that was created by calculator_new().
 *
 * Input:
 *   this = A pointer to the calculator object.
 *
 * Output:
 *   Returns 0 if successful.
 *   Returns 1 if not successful.
 */
bool
calculator_delete(calculator *this)
{
  bool retcode = false;

  if(this != (calculator *) 0)
  {
    list_delete(this->infix_list);

    free(this);
    retcode = true;
  }

  return retcode;
}

/* Add a character to the current equation.  As the user enters their equation
 * the data is passed to the calculator object via this member.
 *
 * Input:
 *   this = A pointer to the calculator object.
 *
 *   c = The character to add.  Note that we only support 8-bit characters.
 *
 * Output:
 *   Returns true for success.  The character was added to the equation.
 *   Returns false if the character wasn't processed.
 */
bool
calculator_add_char(calculator *this,
                    const char c)
{
  bool retcode = true;

  switch(c)
  {
    /* Backspace. */
    case 0x7F:
    case 0x08:
      list_del_tail(this->infix_list);
      break;

    case '=':
      if((retcode = calculator_infix2postfix(this)) == true)
      {
        retcode = calculator_postfix(this);
      }
      break;

    default:
      {
        /* Get the last operator/operand object that we processed. */
        void *obj = (void *) 0;
        int type = LIST_OBJ_TYPE_NONE;
        list_get_tail(this->infix_list, &obj, &type);

        /* If the most recent operand/operator object from the end of the
         * infix list isn't an operand, create a new operand object now. */
        operand *cur_operand = (type == LIST_OBJ_TYPE_OPERAND) ? obj : operand_new();

        /* Try to add the current character as an operand. */
        if(operand_add_char(cur_operand, c) == 0)
        {
          /* If we just created a new operand object, add it to the list. */
          if(cur_operand != obj)
          {
            if(list_add_tail(this->infix_list, cur_operand, LIST_OBJ_TYPE_OPERAND) == false)
            {
              operand_delete(cur_operand);
            }
          }
        }

        /* If it's not an operand, then it's an operator. */
        else
        {
          operator *cur_operator = operator_new(c);
          if(cur_operator != (operator *) 0)
          {
            if(list_add_tail(this->infix_list, cur_operator, LIST_OBJ_TYPE_OPERATOR) == false)
            {
              /* Unable to add the operand to the list. */
              retcode = false;
            }
          }
        }
      }
      break;
  }

  return retcode;
}

/* This member will create an ASCII string that represents the value that
 * should be displayed by the calculator.
 *
 * Input:
 *   this     = A pointer to the calculator object.
 *
 *   buf      = The caller-supplied buffer that will receive the message.
 *
 *   buf_size = An int that specifies the size of buf.  This function will only
 *              place up to (buf_size - 1) characters in the buffer, allowing
 *              one byte for the NULL terminator.  If the entire string would
 *              be longer than (buf_size - 1), then the END of the string will
 *              be placed in the buffer.  This gives the illusion of a message
 *              that is scrolling horizontally.
 *
 * Output:
 *   Returns true for success.  The message was created and placed in buf.
 *   Returns false if an error occurs.  The contents of buf is undefined.
 */
bool
calculator_get_console(calculator *this,
                       char *buf,
                       size_t buf_size)
{
  bool retcode = false;

  if(this != (calculator *) 0)
  {
    memset(this->console_buf, 0, sizeof(this->console_buf));
    list_traverse(this->infix_list, calculator_list_traverse_cb, this);

    /* Pass it back to the caller. */
    if(strlen(this->console_buf) > (buf_size - 1))
    {
      /* Allow room for the NULL terminator. */
      char *p = this->console_buf + (strlen(this->console_buf) - (buf_size - 1));
      memcpy(buf, p, (buf_size - 1));
      buf[buf_size - 1] = 0;
    }
    else
    {
      strcpy(buf, this->console_buf);
    }
  }

  return retcode;
}

/******************************************************************************
 ********************************** TEST API **********************************
 *****************************************************************************/

#if defined(TEST)
bool
calculator_test(void)
{
  bool retcode = false;

  calculator *this = calculator_new();
  if(this != (calculator *) 0)
  {
    list_print(this->infix_list, calculator_list_traverse_cb, this);
    calculator_delete(this);
    retcode = true;
  }

  return retcode;
}
#endif // TEST


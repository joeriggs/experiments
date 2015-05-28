/* An implementation of a simple math calculator.
 *
 * Refer to "Data Structures and Problem Solving in C++" by Mark Allen Weiss
 * for a really good description of an infix-to-postfix calculator.
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

/* These are the types of objects that we store in the infix/postfix lists. */
#define LIST_OBJ_TYPE_OPERAND  1
#define LIST_OBJ_TYPE_OPERATOR 2
#define LIST_OBJ_TYPE_NONE     3

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/* This is the calculator class. */
struct calculator {
  /* As we accept operands and operators from the outside world, we store them
   * on this list. */
  list *infix_list;

  /* We convert infix-to-postfix on this list. */
  list *postfix_list;

  /* A buffer that's used to build the console output. */
  char console_buf[1024];

  /* The number base we're currently configured to use.  This refers to things
   * like base_10 or base_16. */
  calculator_base base;
};
  
/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

/* This function evaluates the postfix equation.
 *
 * Input:
 *   this = A pointer to the calculator object.  The postfix equation is
 *          already stored in postfix_list.  You can get it onto that list by
 *          calling the calculator_infix2postfix() member.
 *
 * Output:
 *   true  = success.  The result is stored at the tail of the infix list.
 *   false = failure.  No result.  infix and postfix lists are empty.
 */
static bool
calculator_postfix(calculator *this)
{
  bool retcode = false;

  /* A stack to use while processing the postfix. */
  stack *tmp_stack;
  if((retcode = ((tmp_stack = stack_new()) != (stack *) 0)) == false)
  {
    return retcode;
  }

  /* These are used while processing the infix equation. */
  void *cur_obj;
  int type;
  while((retcode == true) &&
        (retcode = list_rem_head(this->postfix_list, &cur_obj, &type)) == true)
  {
    /* Operands immediately go onto the stack. */
    if(type == LIST_OBJ_TYPE_OPERAND)
    {
      retcode = stack_push(tmp_stack, cur_obj);
    }

    /* Operators are processed immediately. */
    else if(type == LIST_OBJ_TYPE_OPERATOR)
    {
      operator *cur_operator = (operator *) cur_obj;
      operator_type op_type;
      if((retcode = operator_get_op_type(cur_operator, &op_type)) == true)
      {
        if(op_type == op_type_unary)
        {
          void *op;
          if(((retcode = stack_pop(tmp_stack, &op)) == true) &&
             ((retcode = operator_do_unary(cur_operator, op)) == true))
          {
            retcode = stack_push(tmp_stack, op);
          }
        }

        else if(op_type == op_type_binary)
        {
          void *op1, *op2;
          if(((retcode = stack_pop(tmp_stack, &op2)) == true) &&
             ((retcode = stack_pop(tmp_stack, &op1)) == true) &&
             ((retcode = operator_do_binary(cur_operator, op1, op2)) == true))
          {
            retcode = stack_push(tmp_stack, op1);
          }
        }

        else
        {
          retcode = false;
          break;
        }
      }
    }

    else
    {
      retcode = false;
    }
  }

  /* If the equation was completed successfully, then the postfix_list will be
   * empty and the result will be the only entry on the stack. */
  if(list_get_tail(this->postfix_list, &cur_obj, &type) == false)
  {
    void *result;
    if(((retcode = stack_pop(tmp_stack, &result)) == true) &&
       ((retcode = stack_pop(tmp_stack, &result)) == false))
    {
      /* If we were totally successful, this is where we set retcode. */
      retcode = list_add_tail(this->infix_list, result, LIST_OBJ_TYPE_OPERAND);
    }
  }

  /* If we failed, purge the list. */
  else
  {
    list_del_all(this->postfix_list);
    retcode = false;
  }

  stack_delete(tmp_stack);

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
 *   true  = success.  You can feed the postfix list into the postfix evaluator.
 *   false = failure.  infix and postfix lists are empty.
 */
static bool
calculator_infix2postfix(calculator *this)
{
  bool retcode = true;
  stack *tmp_stack;

  /* Start with an empty stack and a brand new postfix_list. */
  if(this->postfix_list != (list *) 0)
  {
    retcode = (list_delete(this->postfix_list) == true);
    this->postfix_list = (list *) 0;
  }
  if( (retcode == false) ||
     ((retcode = ((tmp_stack = stack_new()) != (stack *) 0)) == false) ||
     ((retcode = ((this->postfix_list = list_new()) != (list *) 0)) == false) )
  {
    stack_delete(tmp_stack);
    list_delete(this->postfix_list);
    return retcode;
  }

  /* Used while processing the infix equation. */
  void *cur_obj;
  int type;
  void *stk_operator;

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

      /* Keep popping operators until we encounter one that is a lower
       * precedence or we hit the bottom of the stack. */
      while((retcode == true) && (stack_peek(tmp_stack, &stk_operator) == true))
      {
        int cur_input, cur_stack;
        int stk_input, stk_stack;
        if( ((retcode = operator_precedence(cur_operator, &cur_input, &cur_stack)) == true) &&
            ((retcode = operator_precedence(stk_operator, &stk_input, &stk_stack)) == true) )
        {
          if(stk_stack > cur_input)
          {
            break;
          }

          if((retcode = stack_pop(tmp_stack, &stk_operator)) == false)
          {
            break;
          }

          /* Add the operator to the postfix. */
          retcode = list_add_tail(this->postfix_list, stk_operator, LIST_OBJ_TYPE_OPERATOR);
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
      retcode = false;
    }
  }

  /* When we're done, pop the rest of the stack and add it to the postfix. */
  while((retcode == true) && (stack_pop(tmp_stack, &stk_operator) == true))
  {
    retcode = list_add_tail(this->postfix_list, stk_operator, LIST_OBJ_TYPE_OPERATOR);
  }

  /* The stack and the infix_list should be empty. */
  if((list_rem_head(this->infix_list, &cur_obj, &type) == true) ||
     (stack_peek(tmp_stack, &stk_operator) == true))
  {
    retcode = false;
  }

  /* Get rid fo the stack and flush the infix_list. */
  stack_delete(tmp_stack);
  list_del_all(this->infix_list);

  return retcode;
}

/* This is a callback function for a list traversal.  When we want to create an
 * ASCII string that contains the entire infix or postfix equation, we call
 * list::list_traverse(), and it passes each operand/operator object to this
 * member.  This member then builds a string that contains the ASCII equation.
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
 *   true  = success.  The operand or operator is added to this->console_buf.
 *   false = failure.
 */
static bool
calculator_get_console_trv_cb(const void *ctx,
                              const void *object,
                              int type)
{
  bool retcode = false;
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
      if(buf_is_empty == false)
      {
        *(buf_dst++) = ' ';
        buf_is_empty = (--buf_dst_max == 0) ? true : false;
      }
      retcode = operand_to_str((operand *) object, buf_dst, buf_dst_max);
      break;

    case LIST_OBJ_TYPE_OPERATOR:
      {
        operator *cur_operator = (operator *) object;
        const char *op_name;
        if((retcode = operator_get_name(cur_operator, &op_name)) == true)
        {
          snprintf(buf_dst, buf_dst_max, buf_is_empty ? "%s" : " %s", op_name);
        }
      }
      break;

    default:
      retcode = false;
      break;
    }
  }

  return retcode;
}

/* This is a callback function for a list traversal.  When the user switches the
 * calculator base (for example, switching from base10 to base16), we need to
 * walk through the infix data and convert each operand to the new base.  We use
 * list::list_traverse() to walk the list, and it passes each operand object to
 * this member.  This member then tells the operand class to convert the
 * operand.
 *
 * Input:
 *   ctx    = A pointer to the calculator object.
 *
 *   object = An opaque value that represents an object.
 *
 *   type   = This defines what type of object is contained in object.  We
 *            use the type to help us identify the object.
 *
 * Output:
 *   true  = success.  The base is correct for the specified operand.
 *   false = failure.  The base setting for the operand is undefined.
 */
static bool
calculator_set_base_trv_cb(const void *ctx,
                           const void *object,
                           int type)
{
  bool retcode = false;

  if(ctx != (const void *) 0)
  {
    calculator *this = (calculator *) ctx;
    if(type == LIST_OBJ_TYPE_OPERAND)
    {
      /* Set the new base.  Don't worry about setting it to the same value.
       * Let the operand object fend for itself. */
      switch(this->base)
      {
      case calculator_base_10:
        retcode = operand_set_base((operand *) object, operand_base_10);
        break;

      case calculator_base_16:
        retcode = operand_set_base((operand *) object, operand_base_16);
        break;

      default:
        break;
      }
    }

    /* Operators are a NOP. */
    else
    {
      retcode = true;
    }
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
  calculator *this = (calculator *) 0;

  /* Initialize. */
  if((this = (calculator *) malloc(sizeof(*this))) != (calculator *) 0)
  {
    /* Create an empty infix list. */
    if((this->infix_list = list_new()) != (list *) 0)
    {
      /* Okay, we have successfully allocated everything that needs to be
       * allocated for a new calculator object.  We are successful.  Go ahead
       * and initialize the object. */

      /* The default base is decimal (base_10). */
      this->base = calculator_base_10;
    }

    else
    {
      calculator_delete(this);
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
 *   true  = success.  The object is deleted.
 *   false = failure.
 */
bool
calculator_delete(calculator *this)
{
  bool retcode = false;

  if(this != (calculator *) 0)
  {
    retcode = list_delete(this->infix_list);

    free(this);
  }

  return retcode;
}

/* Get the current number base that the calculator is configured for.  Note
 * that this refers to the setting of the calculator object.  It doesn't
 * necessarily mean all of the internal operand objects are the same.  They
 * should be correct, but it's worth noting that calculator base settings and
 * operand base settings are not absolutely tied together.
 *
 * Input:
 *   this = A pointer to the calculator object.
 *
 *   base = A pointer to a variable that is set to the current base.
 *
 * Output:
 *   true  = success.  *base = the current base.
 *   false = failure.  *base = calculator_base_unknown (if base != 0).
 */
bool
calculator_get_base(calculator *this,
                    calculator_base *base)
{
  bool retcode = false;

  if(base != (calculator_base *) 0)
  {
    if(this != (calculator *) 0)
    {
      *base = this->base;
      retcode = true;
    }
    else
    {
      *base = calculator_base_unknown;
    }
  }

  return retcode;
}

/* Set the number base that the calculator should use.  Also walk through the
 * list of operands that are currently stored in the infix list and set them to
 * the specified base.
 *
 * Input:
 *   this = A pointer to the calculator object.
 *
 *   base = The new base that we should set.
 *
 * Output:
 *   true  = success.  The calculator is now using the new base.
 *   false = failure.  The calculator is NOT using the new base.  State unknown.
 */
bool
calculator_set_base(calculator *this,
                    calculator_base base)
{
  bool retcode = false;

  if(this != (calculator *) 0)
  {
    switch(base)
    {
    case calculator_base_10:
    case calculator_base_16:
      /* This is a known base.  Save it and then walk the infix list and set
       * all of the operands to the specified base. */
      this->base = base;
      retcode = list_traverse(this->infix_list, calculator_set_base_trv_cb, this);
      break;

    default:
      break;
    }
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
 *   true  = success.  The character has been added to the equation.  Note that
 *                     invalid characters are silently dropped.
 *   false = failure.  The character wasn't processed.  Some type of internal
 *                     error occurred.
 */
bool
calculator_add_char(calculator *this,
                    const char c)
{
  bool retcode = false;

  switch(c)
  {
  /* Backspace. */
  case 0x7F:
  case 0x08:
    /* Delete the last item in the list. */
    retcode = list_del_tail(this->infix_list);
    break;

  /* Complete the equation.  Save the result. */
  case '=':
    if((retcode = calculator_infix2postfix(this)) == true)
    {
      retcode = calculator_postfix(this);
    }
    break;

  /* This is where most characters end up.  If they get to here, then we
   * process them as regular calculator input. */
  default:
    {
      void *obj = (void *) 0;
      int type = LIST_OBJ_TYPE_NONE;

      /* Get the last operator/operand object that we processed.  If the most
       * recent object from the end of the infix list isn't an operand, create
       * a new operand object now. */
      operand *cur_operand;
      if((list_get_tail(this->infix_list, &obj, &type) == true) && (type == LIST_OBJ_TYPE_OPERAND))
      {
        cur_operand = (operand *) obj;
      }
      else
      {
        cur_operand = operand_new(this->base);
      }

      /* Try to add the character as an operand. */
      if(cur_operand != (operand *) 0)
      {
        if((retcode = operand_add_char(cur_operand, c)) == true)
        {
          /* If we just created a new operand object, add it to the list. */
          if(cur_operand != obj)
          {
            if((retcode = list_add_tail(this->infix_list, cur_operand, LIST_OBJ_TYPE_OPERAND)) == false)
            {
              operand_delete(cur_operand);
            }
          }
        }
      }

      /* If we weren't able to add it as an operand, then try to add it as an
       * operator. */
      if(retcode == false)
      {
        operator *cur_operator = operator_new(c);
        if(cur_operator != (operator *) 0)
        {
          retcode = list_add_tail(this->infix_list, cur_operator, LIST_OBJ_TYPE_OPERATOR);
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
 *   true  = success.  The message has been created and placed in buf.
 *   false = failure.  The contents of buf is undefined.
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
    if((retcode = list_traverse(this->infix_list, calculator_get_console_trv_cb, this)) == true)
    {
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
  /* Make sure an empty calculator_delete() fails. */
  DBG_PRINT("calculator_delete(0)\n");
  if(calculator_delete((calculator *) 0) != false)                                   return false;

  /* Create a calculator object. */
  DBG_PRINT("calculator_new()\n");
  calculator *this = calculator_new();
  if(this == (calculator *) 0)                                                       return false;

  /* Test switching the base back and forth. */
  DBG_PRINT("calculator_get_base()\n");
  calculator_base base;
  if((calculator_get_base(this, &base) != true) || (base != calculator_base_10))     return false;
  DBG_PRINT("calculator_set_base(calculator_base_16)\n");
  if(calculator_set_base(this, calculator_base_16) != true)                          return false;
  DBG_PRINT("calculator_set_base(calculator_base_10)\n");
  if(calculator_set_base(this, calculator_base_10) != true)                          return false;

  /* Loop through some math problems.  This tests the basic functionality of
   * the calculator.  We're checking to make sure it can do math. */
  typedef struct calculator_test {
    const char *infix;
    bool        postfix_retcode;
    bool        console_retcode;
    const char *result;
  } calculator_test;
  calculator_test tests[] = {
    { "1+2*3",           true,  true,      "7"            }, // Order of operations.
    { "\b10+20*30",      true,  true,    "610"            }, // Order of operations.
//    { "\b10/0+20*30",   false, false,       ""            }, // Divide by zero.
//    { "(1+2)*3",         true,  true,      "9"            }, // Parentheses override order.
//    { "*3",              true,  true,     "27"            }, // Follow-on to the previous result.
    { "\b7/10",          true,  true,      "0.7"          }, // int / int = float.
    { "\b7.4/10",        true,  true,      "0.74"         }, // float / int.
    { "\b2.5*2",         true,  true,      "5"            }, // float * int.
    { "\b2^3",           true,  true,      "8"            }, // int ^ int.
    { "\b2^3s",          true,  true,      "0.125000"     }, // int ^ -int.
    { "\b2.34^5",        true,  true,     "70.1583371424" }, // float ^ int.
    { "\b3^12.345",      true,  true, "776357.74428398"   }, // int ^ float.
    { "\b2.34^5.678",    true,  true,    "124.8554885559" }, // float ^ float.
  };
  size_t calculator_test_size = (sizeof(tests) / sizeof(calculator_test));

  int x;
  for(x = 0; x < calculator_test_size; x++)
  {
    calculator_test *t = &tests[x];

    const char *infix = t->infix;
    printf("%s\n", infix);
    while(*infix)
    {
      if(calculator_add_char(this, *(infix++)) != true)                              return false;
    }

    /* Traverse the infix list as decimal. */
    DBG_PRINT("calculator_get_console(1)\n");
    char buf[1024];
    this->console_buf[0] = 0;
    if(calculator_get_console(this, buf, sizeof(buf)) != true)                       return false;
    DBG_PRINT("'%s'\n", buf);

    if(calculator_infix2postfix(this) != true)                                       return false;
    if(calculator_postfix(this) != t->postfix_retcode)                               return false;
    if(calculator_get_console(this, buf, sizeof(buf)) != t->console_retcode)         return false;
    if(t->console_retcode == true) printf(" = '%s'\n", buf);
    if(strcmp(buf, t->result) != 0)                                                  return false;
  }

  DBG_PRINT("calculator_delete(this)\n");
  if(calculator_delete(this) != true)                                                return false;

  return true;
}
#endif // TEST


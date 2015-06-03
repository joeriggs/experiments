/* This is a very simple implementation of a stack.  Each item on the stack is
 * a single void *.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "stack.h"

/* Set a max size for the stack.  We don't want uncontrolled growth. */
#define STACK_DEPTH_MAX 4096

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/* This is the stack class. */
struct stack {
  void **stack_data;
  int    stack_index;
  size_t stack_depth;
};
  
/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

/******************************************************************************
 ********************************* PUBLIC API *********************************
 *****************************************************************************/

/* Create a new stack object.  This object can be used to access the stack
 * class.
 *
 * Input:
 *   N/A.
 *
 * Output:
 *   Returns a pointer to the object.
 *   Returns 0 if unable to create the object.
 */
stack *
stack_new(void)
{
  stack *this = malloc(sizeof(*this));

  if(this != (stack *) 0)
  {
    this->stack_data  = (void *) 0;
    this->stack_index = -1;
    this->stack_depth = 0;
  }

  return this;
}

/* Delete a stack object that was created by stack_new().
 *
 * Input:
 *   this = A pointer to the stack object.
 *
 * Output:
 *   true  = success.  this is deleted.
 *   false = failure.  this is undefined.
 */
bool
stack_delete(stack *this)
{
  bool retcode = false;

  if(this != (stack *) 0)
  {
    if(this->stack_data != (void *) 0)
    {
      free(this->stack_data);
    }

    free(this);
    retcode = true;
  }

  return retcode;
}

/* Push a single item onto the stack.
 *
 * Input:
 *   this = A pointer to the stack object.
 *
 *   i    = The value to push.
 *
 * Output:
 *   true  = success.  i is on the top of the stack.
 *   false = failure.  The value is not on the stack, and the stack is unchanged.
 */
bool
stack_push(stack *this,
           void *i)
{
  bool retcode = false;

  if( (this != (stack *) 0) && (i != (void *) 0) )
  {
    /* If the stack is full, grow it. */
    if((this->stack_index + 1) == this->stack_depth)
    {
      /* Only grow the stack if it isn't at its max size. */
      if(this->stack_depth < STACK_DEPTH_MAX)
      {
        /* If this is the first push allocate the first stack. */
        if(this->stack_data == (void **) 0)
        {
          this->stack_depth = 1;
          this->stack_data = (void **) malloc(this->stack_depth * sizeof(void *));
        }

        /* We already have a stack.  We need to grow it.  So we'll double it. */
        else
        {
          this->stack_depth = this->stack_depth << 1;
          this->stack_data = realloc(this->stack_data, this->stack_depth * sizeof(void *));
        }

        /* If we don't have a stack, fail now. */
        if(this->stack_data == (void **) 0)
        {
          this->stack_depth = 0;
          this->stack_index = -1;
        }
        else
        {
          /* We successfully created or grew the stack.  It is safe to push. */
          retcode = true;
        }
      }
    }
    else
    {
      /* There is room in the stack.  Okay to proceed. */
      retcode = true;
    }

    /* If we have a stack, save the value on the stack. */
    if(retcode == true)
    {
      this->stack_index++;
      this->stack_data[this->stack_index] = i;
    }
  }

  DBG_PRINT("%s() returning %d.\n", __func__, retcode);
  return retcode;
}

/* Pop a single item from the stack.
 *
 * Input:
 *   this = A pointer to the stack object.
 *
 *   dest = Pointer to the location to pop into.
 *
 * Output:
 *   true  = success.  i contains the item from the top.  It is popped.
 *   false = failure.  The stack is empty.
 */
bool
stack_pop(stack *this,
          void **dest)
{
  bool retcode = false;

  if( (this != (stack *) 0) && (this->stack_index >= 0) && (dest != (void **) 0) )
  {
    *dest = this->stack_data[this->stack_index--];
    retcode = true;
  }

  DBG_PRINT("%s() returning %d.\n", __func__, retcode);
  return retcode;
}

/* Peek at the top item on the stack.
 *
 * Input:
 *   this = A pointer to the stack object.
 *
 *   dest = Pointer to the location to store the value into.
 *
 * Output:
 *   true  = success.  i contains the item from the top.  It is NOT popped.
 *   false = failure.  The stack is empty.
 */
bool
stack_peek(stack *this,
           void **dest)
{
  bool retcode = false;

  if( (this != (stack *) 0) && (this->stack_index >= 0) && (dest != (void **) 0) )
  {
    *dest = this->stack_data[this->stack_index];
    retcode = true;
  }

  DBG_PRINT("%s() returning %d.\n", __func__, retcode);
  return retcode;
}

/******************************************************************************
 ********************************** TEST API **********************************
 *****************************************************************************/

#ifdef TEST

bool
stack_test(void)
{
  bool retcode = false;

  stack *this = stack_new();
  if(this != (stack *) 0)
  {
    stack_delete(this);
    retcode = true;
  }

  return retcode;
}

#endif // TEST


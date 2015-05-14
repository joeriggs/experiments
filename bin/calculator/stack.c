/* This is a very simple implementation of a stack.  Each item on the stack is
 * a single char.  It will be extended in the future.
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

/* This is the operand class. */
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
 *   Returns 0 if successful.
 *   Returns 1 if not successful.
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

/* Push a single int onto the stack.
 *
 * Input:
 *   this = A pointer to the stack object.
 *
 *   i    = The value to push.
 *
 * Output:
 *   Returns true if success.  i is on the top of the stack.
 *   Returns false if failure.  The stack is unchanged.
 */
bool
stack_push(stack *this,
           void *i)
{
  bool retcode = true;

  /* If the stack is full, grow it. */
  if((this->stack_index + 1) == this->stack_depth) {
    /* If the stack is at full size, return an error. */
    if(this->stack_depth >= STACK_DEPTH_MAX) {
      printf("Stack is full.  Failing to push.\n");
      retcode = false;
    }

    /* If this is the first push allocate the first stack. */
    else if(this->stack_data == (void **) 0) {
      this->stack_depth = 1;
      this->stack_data = (void **) malloc(this->stack_depth * sizeof(int));
    }

    /* We already have a stack.  We need to grow it.  So we'll double it. */
    else {
      this->stack_depth = this->stack_depth << 1;
      this->stack_data = realloc(this->stack_data, this->stack_depth * sizeof(int));
    }

    /* If we don't have a stack, fail now. */
    if(this->stack_data == (void **) 0) {
      printf("We don't have a stack.\n");
      retcode = false;
    }
  }

  /* If we have a stack, save the value on the stack. */
  if(retcode == true)
  {
    this->stack_index++;
    this->stack_data[this->stack_index] = i;
  }

  return retcode;
}

/* Pop a single int from the stack.
 *
 * Input:
 *   this = A pointer to the stack object.
 *
 *   dest = Pointer to the location to pop into.
 *
 * Output:
 *   true  = success.  i contains the int from the top.  It is popped.
 *   false = failure.  The stack is empty.
 */
bool
stack_pop(stack *this,
          void **dest)
{
  bool retcode = false;

  if(this->stack_index >= 0) {
    *dest = this->stack_data[this->stack_index--];
    retcode = true;
  }

  return retcode;
}

/* Peek at the top int on the stack.
 *
 * Input:
 *   this = A pointer to the stack object.
 *
 *   dest = Pointer to the location to pop into.
 *
 * Output:
 *   Returns 0 if success.  i contains the int from the top.  It is NOT popped.
 *   Returns 1 if failure.  The stack is empty.
 */
bool
stack_peek(stack *this,
           void **dest)
{
  bool retcode = false;

  if(this->stack_index >= 0)
  {
    *dest = this->stack_data[this->stack_index];
    retcode = true;
  }

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


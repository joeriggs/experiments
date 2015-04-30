/* This is a very simple implementation of a stack.  Each item on the stack is
 * a single char.  It will be extended in the future.
 */
#include <stdio.h>
#include <stdlib.h>

#include "stack.h"

/* Set a max size for the stack.  We don't want uncontrolled growth. */
#define STACK_DEPTH_MAX 4096
static char  *stack = (char *) 0;
static int    stack_index = -1;
static size_t stack_depth = 0;

/* Push a single character onto the stack.
 *
 * Input:
 *   c = The character to push.
 *
 * Output:
 *   Returns 0 if success.  c is on the top of the stack.
 *   Returns 1 if failure.  The stack is unchanged.
 */
int push(char c)
{
  /* If the stack is full, grow it. */
  if((stack_index + 1) == stack_depth) {
    /* If the stack is at full size, return an error. */
    if(stack_depth >= STACK_DEPTH_MAX) {
      printf("Stack is full.  Failing to push.\n");
      return 1;
    }

    /* If this is the first push allocate the first stack. */
    if(stack == (char *) 0) {
      stack_depth = 1;
      stack = (char *) malloc(stack_depth);
    }

    /* We already have a stack.  We need to grow it.  So we'll double it. */
    else {
      stack_depth = stack_depth << 1;
      stack = realloc(stack, stack_depth);
    }

    /* If we don't have a stack, fail now. */
    if(stack == (char *) 0) {
      printf("We don't have a stack.\n");
      return 1;
    }
  }

  /* Save the value on the stack. */
  stack_index++;
  stack[stack_index] = c;

  return 0;
}

/* Pop a single character from the stack.
 *
 * Input:
 *   c = Pointer to the location to pop into.
 *
 * Output:
 *   Returns 0 if success.  c contains the character from the top.  It is popped.
 *   Returns 1 if failure.  The stack is empty.
 */
int pop(char *c)
{
  if(stack_index == -1) {
    return 1;
  }

  *c = stack[stack_index];
  stack_index--;

  return 0;
}

/* Peek at the top character on the stack.
 *
 * Input:
 *   c = Pointer to the location to pop into.
 *
 * Output:
 *   Returns 0 if success.  c contains the character from the top.  It is popped.
 *   Returns 1 if failure.  The stack is empty.
 */
int peek(char *c)
{
  if(stack_index == -1) {
    return 1;
  }

  *c = stack[stack_index];

  return 0;
}


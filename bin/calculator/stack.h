/* This is the external API for the simple stack implementation.
 */
#ifndef __STACK_H__
#define __STACK_H__

/****************************** CLASS DEFINITION ******************************/

typedef struct stack stack;

/********************************* PUBLIC API *********************************/

stack *stack_new(void);

bool stack_delete(stack *this);

bool stack_push(stack *this, void *i);

bool stack_pop( stack *this, void *i);

bool stack_peek(stack *this, void *i);

/********************************** TEST API **********************************/

#if defined(DEBUG) || defined(TEST)

bool stack_test(void);

#endif // DEBUG || TEST

#endif // __STACK_H__


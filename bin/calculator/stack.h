/* This is the external API for the simple stack implementation.
 */
#ifndef __STACK_H__
#define __STACK_H__

/****************************** CLASS DEFINITION ******************************/

typedef struct stack stack;

/********************************* PUBLIC API *********************************/

stack *stack_new(void);

bool stack_delete(stack *this);

bool stack_push(stack *this, void *src);

bool stack_pop( stack *this, void **dest);

bool stack_peek(stack *this, void **dest);

/********************************** TEST API **********************************/

#if defined(TEST)

bool stack_test(void);

#endif // TEST

#endif // __STACK_H__


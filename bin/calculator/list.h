/* This is a simple implementation of a doubly-linked list.
 */
#ifndef __LIST_H__
#define __LIST_H__

/****************************** CLASS DEFINITION ******************************/

typedef struct list list;

/********************************* PUBLIC API *********************************/

list *list_new(void);

bool list_delete(list *this);

bool list_add_tail(list *this, void *object, int type);

bool list_get_tail(list *this, void **object, int *type);

bool list_del_tail(list *this);

bool list_rem_head(list *this, void **object, int *type);

typedef bool (*list_traverse_cb)(const void *cb_ctx, const void *object, int type);

bool list_traverse(list *this, list_traverse_cb cb, void *cb_obj);

/********************************** TEST API **********************************/

#if defined(DEBUG) || defined(TEST)

bool list_test(void);

#endif // DEBUG || TEST

#endif // __LIST_H__


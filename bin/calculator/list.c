/* This is a simple implementation of a doubly-linked list.
 * 
 * Each item in the list contains an item "type" and a void pointer to an
 * object that contains the list item.
 */
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#include "list.h"

/******************************************************************************
 ****************************** CLASS DEFINITION ******************************
 *****************************************************************************/

/* This is a single item in the list. */
typedef struct list_item {
  struct list_item *next;
  struct list_item *prev;

  int type;
  void *object;
} list_item;

/* This is the list class. */
struct list {
  list_item *head;
  list_item *tail;
};
  
/******************************************************************************
 ******************************** PRIVATE API *********************************
 *****************************************************************************/

/******************************************************************************
 ********************************* PUBLIC API *********************************
 *****************************************************************************/

/* Create a new list object.  This object can be used to access the list class.
 *
 * Input:
 *   N/A.
 *
 * Output:
 *   Returns a pointer to the object.
 *   Returns 0 if unable to create the object.
 */
list *
list_new(void)
{
  list *this = malloc(sizeof(*this));

  if(this != (list *) 0)
  {
    this->head = (list_item *) 0;
    this->tail = (list_item *) 0;
  }

  return this;
}

/* Delete a list object that was created by list_new().
 *
 * Input:
 *   this = A pointer to the list object.
 *
 * Output:
 *   Returns 0 if successful.
 *   Returns 1 if not successful.
 */
bool
list_delete(list *this)
{
  bool retcode = false;

  if(this != (list *) 0)
  {
    while(this->tail != (list_item *) 0)
    {
      list_del_tail(this);
    }

    free(this);
    retcode = true;
  }

  return retcode;
}

/* Add an entry to the end of the list.
 *
 * Input:
 *   this    = A pointer to the list object.
 *
 *   object  = The object to put on the list.
 *
 *   type    = The type of the object.  It means nothing to this class.  It's
 *             used by the caller.
 *
 * Output:
 *   Returns true if the object was added to the list.
 *   Returns false if the object was NOT added to the list.
 */
bool
list_add_tail(list *this, void *object, int type)
{
  bool retval = false;

  /* Make sure there is an object. */
  if(this != (list *) 0)
  {
    list_item *i = malloc(sizeof(list_item));
    if(i != (list_item *) 0)
    {
      i->type = type;
      i->object = object;

      /* The first item in the list requires special processing. */
      if(this->head == (list_item *) 0)
      {
        /* Next and Prev point to zero. */
        i->next = i->prev = 0;

        /* There's only one item on the list.  Head and tail both point to it. */
        this->head = this->tail = i;
      }

      /* Items 2 - N all work the same. */
      else
      {
        /* Add the entry to the end of the list. */
        i->next = 0;
        i->prev = this->tail;
        this->tail->next = i;
        this->tail = i;
      }

      /* Success. */
      retval = true;
    }
  }

  return retval;
}

/* Get the entry that is on the end of the list.
 *
 * Input:
 *   this    = A pointer to the list object.
 *
 *   object  = The location to store the object.
 *
 *   type    = The location to store the type of the object.
 *
 * Output:
 *   Returns 0 if the last object was returned to the caller.
 *   Returns 1 if the last object was NOT returned.
 */
int list_get_tail(list *this, void **object, int *type)
{
  int retval = 1;

  /* Make sure there is an object, and make sure the list isn't empty. */
  if( (this != (list *) 0) && (this->tail != (list_item *) 0) )
  {
    list_item *tail = this->tail;
    *object = tail->object;
    *type   = tail->type;
    retval = 0;
  }

  return retval;
}

/* Delete the last entry from the list.
 *
 * Input:
 *   this = A pointer to the list object.
 *
 * Output:
 *   Returns 0 if success.  If the list was NOT empty, the last entry is gone.
 *   Returns 1 if an error occurred.
 */
int list_del_tail(list *this)
{
  int retval = 0;

  /* Make sure there is an object, and make sure the list isn't empty. */
  if( (this != (list *) 0) && (this->tail != (list_item *) 0) )
  {
    /* l = the last item in the list.
     * p = the item before l (could be NULL). */
    list_item *l = this->tail;
    list_item *p = l->prev;

    /* The last item on the list requires special processing. */
    if(p == (list_item *) 0)
    {
      this->head = this->tail = (list_item *) 0;
    }

    /* p->next currently points to l.  Change it to point to NULL (indicating
     * the end of the list), and set tail to point to p.  We now have a
     * shortened list. */
    else
    {
      p->next = (list_item *) 0;
      this->tail = p;
    }

    /* Get rid of l. */
    free(l);
  }

  return retval;
}

/* Remove the first entry from the list and return to the caller.
 *
 * Input:
 *   this    = A pointer to the list object.
 *
 *   object  = The location to store the object.
 *
 *   type    = The location to store the type of the object.
 *
 * Output:
 *   true  = success.  The list was NOT empty and the first entry was returned.
 *   false = an error occurred.
 */
bool
list_rem_head(list *this, void **object, int *type)
{
  bool retval = false;

  /* Make sure there is an object, and make sure the list isn't empty. */
  if( (this != (list *) 0) && (this->head != (list_item *) 0) )
  {
    /* f = the head item in the list.
     * n = the item after f (could be NULL). */
    list_item *f = this->head;
    list_item *n = f->next;

    /* The last item on the list requires special processing. */
    if(n == (list_item *) 0)
    {
      this->head = this->tail = (list_item *) 0;
    }

    /* "head" currently points to f.  Change it to point to n.  We now have a
     * shortened list. */
    else
    {
      this->head = n;
    }

    /* Return the contents of the entry, and then get rid of f. */
    *object = f->object;
    *type   = f->type;
    free(f);

    retval = true;
  }

  return retval;
}

/* Traverse the list.  This could be useful for debug/test, or it could be used
 * to just scan the list to see what's in there.
 *
 * Input:
 *   this   = A pointer to the list object.
 *
 *   cb     = A callback function.  The list class doesn't know what's stored in
 *            the list, so this method just passes each list_item object back to
 *            the class owner and allows the owner to analyze it.
 *
 *            If cb == NULL, then we return immediately.
 *
 *   cb_ctx = A context that is passed to the callback (It's probably the obj
 *            of the list owner).  cb_ctx is opaque to us. We just pass it to
 *            the cb function.
 *
 * Output:
 *   N/A.
 */
void
list_traverse(list *this,
              list_traverse_cb cb,
              void *cb_ctx)
{
  if(cb != (list_traverse_cb) 0)
  {
    list_item *i = this->head;
    while(i != (list_item *) 0)
    {
      cb(cb_ctx, i->object, i->type);
      i = i->next;
    }
  }
}

/******************************************************************************
 ********************************** TEST API **********************************
 *****************************************************************************/

#ifdef TEST
bool
list_test(void)
{
  bool retcode = false;

  list *this = list_new();
  if(this != (list *) 0)
  {
    list_delete(this);
    retcode = true;
  }

  return retcode;
}
#endif // TEST


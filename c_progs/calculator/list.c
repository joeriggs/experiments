/* This is an implementation of a doubly-linked list.
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
    /* Start with an empty list. */
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
 *   Returns true if successful.
 *   Returns false if not successful.
 */
bool
list_delete(list *this)
{
  bool retcode = false;

  if(this != (list *) 0)
  {
    retcode = list_del_all(this);

    free(this);
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
 *   true  = success.  The object was added to the list.
 *   false = failure.  The object was NOT added to the list.
 */
bool
list_add_tail(list *this, void *object, int type)
{
  bool retcode = false;

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
      retcode = true;
    }
  }

  return retcode;
}

/* Get the entry that is on the end of the list.  This is a nondestructive
 * operation.  The entry is returned, but it's also left on the list.
 *
 * Input:
 *   this    = A pointer to the list object.
 *
 *   object  = The location to store the object.
 *
 *   type    = The location to store the type of the object.
 *
 * Output:
 *   true  = success.  The last object was returned to the caller.
 *   false = failure.  The last object was NOT returned.  Perhaps the list is
 *                     empty.
 */
bool
list_get_tail(list *this, void **object, int *type)
{
  bool retcode = false;

  /* Make sure there is an object, and make sure the list isn't empty. */
  if( (this != (list *) 0) && (this->tail != (list_item *) 0) )
  {
    list_item *tail = this->tail;
    *object = tail->object;
    *type   = tail->type;
    retcode = true;
  }

  return retcode;
}

/* Delete the last entry from the list.
 *
 * Input:
 *   this = A pointer to the list object.
 *
 * Output:
 *   true  = success.  If the list was NOT empty, the last entry is gone.
 *   false = failure.
 */
bool
list_del_tail(list *this)
{
  bool retcode = false;

  /* Make sure there is an object, and make sure the list isn't empty. */
  if( (this != (list *) 0) && (this->tail != (list_item *) 0) )
  {
    /* l = the last item in the list (definitely NOT NULL).
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

    /* l has been removed from the list.  Get rid of it. */
    free(l);
    retcode = true;
  }

  return retcode;
}

/* Delete all of the entries from the list.  The list will still exist, but it
 * will be empty.
 *
 * Input:
 *   this = A pointer to the list object.
 *
 * Output:
 *   true  = success.  The list is empty.
 *   false = failure.  The state of the list is undefined.
 */
bool
list_del_all(list *this)
{
  bool retcode = false;

  /* Loop here and delete everything from the list. */
  while(list_del_tail(this) == true);

  /* Make sure the list is empty. */
  void *object;
  int type;
  retcode = (list_get_tail(this, &object, &type) == false);
  return retcode;
}

/* Remove the first entry from the list and return it to the caller.
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
 *   false = no entry is returned.  Perhaps the list is empty.
 */
bool
list_rem_head(list *this, void **object, int *type)
{
  bool retcode = false;

  /* Make sure there is an object, and make sure the list isn't empty. */
  if( (this != (list *) 0) && (this->head != (list_item *) 0) )
  {
    /* f = the first item in the list (definitely NOT NULL).
     * n = the item after f (could be NULL). */
    list_item *f = this->head;
    list_item *n = f->next;

    /* The final item on the list requires special processing. */
    if(n == (list_item *) 0)
    {
      this->head = this->tail = (list_item *) 0;
    }

    /* "head" currently points to f.  Change it to point to n.  We now have a
     * shortened list. */
    else
    {
      this->head = n;
      n->prev = (list_item *) 0;
    }

    /* Return the contents of the entry, and then get rid of it. */
    *object = f->object;
    *type   = f->type;
    free(f);

    retcode = true;
  }

  return retcode;
}

/* Traverse the list.  This method will traverse the list (from head to tail)
 * and pass each entry in the list to a callback function.
 *
 * Note that it's okay if the cb deletes the entry from the list.
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
 *   true  = success.  We traversed the entire list without problems.
 *   false = failure.  We ran into a problem somewhere along the way.
 */
bool
list_traverse(list *this,
              list_traverse_cb cb,
              void *cb_ctx)
{
  bool retcode = true;

  if(cb != (list_traverse_cb) 0)
  {
    list_item *i = this->head;
    while(i != (list_item *) 0)
    {
      list_item *n = i->next;
      retcode = cb(cb_ctx, i->object, i->type);
      i = n;
    }
  }

  return retcode;
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


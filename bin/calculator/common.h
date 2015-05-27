/* This head file contains information that is used in a lot of places in the
 * calculator class.
 */

#ifndef __COMMON_H__
#define __COMMON_H__

/* Boolean type. */
typedef enum { false, true }  bool;

#if defined(DEBUG)
#define DBG_PRINT printf
#else
#define DBG_PRINT(args...)
#endif

#endif // __COMMON_H__


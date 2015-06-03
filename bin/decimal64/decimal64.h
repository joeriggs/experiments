/* An implementation of the decimal64 data representation. */

#ifndef __DECIMAL64_H__
#define __DECIMAL64_H__

/****************************** CLASS DEFINITION ******************************/

typedef struct decimal64 decimal64;

/********************************* PUBLIC API *********************************/

decimal64 *decimal64_new(void);

bool decimal64_delete(decimal64 *this);

/********************************** TEST API **********************************/

#if defined(TEST)

bool decimal64_test(void);

#endif // TEST

#endif /* __DECIMAL64_H__ */


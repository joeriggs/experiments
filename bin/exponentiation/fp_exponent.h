/* An implementation of floating-point exponentiation. */

#ifndef __FP_EXPONENT_H__
#define __FP_EXPONENT_H__

/****************************** CLASS DEFINITION ******************************/

typedef enum { false, true } bool;

/********************************* PUBLIC API *********************************/

bool fp_exponent_calc(double base, double exp, double *result);

/********************************** TEST API **********************************/

#if defined(TEST)

bool fp_exponent_test(void);

#endif // TEST

#endif /* __FP_EXPONENT_H__ */


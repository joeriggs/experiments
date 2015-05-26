/* An implementation of floating-point exponentiation. */

#ifndef __FP_EXPONENT_H__
#define __FP_EXPONENT_H__

/****************************** CLASS DEFINITION ******************************/

typedef struct fp_exponent fp_exponent;

/********************************* PUBLIC API *********************************/

fp_exponent *fp_exponent_new(double base, double exp);

bool fp_exponent_delete(fp_exponent *this);

bool fp_exponent_calc(fp_exponent *this);

bool fp_exponent_get_result(fp_exponent *this, double *result);

/********************************** TEST API **********************************/

#if defined(TEST)

bool fp_exponent_test(void);

#endif // TEST

#endif /* __FP_EXPONENT_H__ */


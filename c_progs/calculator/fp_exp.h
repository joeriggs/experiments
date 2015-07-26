/* An implementation of floating-point exponentiation. */

#ifndef __FP_EXP_H__
#define __FP_EXP_H__

#include "bcd.h"

/****************************** CLASS DEFINITION ******************************/

typedef struct fp_exp fp_exp;

/********************************* PUBLIC API *********************************/

fp_exp *fp_exp_new(bcd *base, bcd *exp);

bool fp_exp_delete(fp_exp *this);

bool fp_exp_calc(fp_exp *this);

bool fp_exp_get_result(fp_exp *this, bcd *result);

/********************************** TEST API **********************************/

#if defined(TEST)

bool fp_exp_test(void);

#endif // TEST

#endif /* __FP_EXP_H__ */


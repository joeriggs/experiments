#pragma once

/*******************************************************************************
 *
 * External definition of big_number_base_????.c.  There are a couple different
 * implementations of this class:
 *
 * - big_number_base_test.c is based on uint64_t.  For test/debug.
 *
 * - big_number_base_full.c is based on a very large number. For production.
 *
 ******************************************************************************/

/******************************* CLASS DEFINITION *****************************/

typedef struct big_number_base big_number_base;

/********************************** CONSTANTS *********************************/

const big_number_base *big_number_base_1(void);

/********************************** PUBLIC API ********************************/

/********** Create, Delete, Initialize Methods */

big_number_base *big_number_base_new(void);

void big_number_base_delete(big_number_base *this);

void big_number_base_copy(const big_number_base *src, big_number_base *dst);

/********** Math Operations */

void big_number_base_add(const big_number_base *addend1, const big_number_base *addend2, big_number_base *sum);

void big_number_base_subtract(const big_number_base *minuend, const big_number_base *subtrahend, big_number_base *difference);

void big_number_base_multiply(const big_number_base *factor1, const big_number_base *factor2, big_number_base *product);

void big_number_base_divide(const big_number_base *dividend, const big_number_base *divisor, big_number_base *quotient);

void big_number_base_modulus(const big_number_base *this, const big_number_base *modulus, big_number_base *result);

/********** Comparison Methods */

int big_number_base_compare(const big_number_base *a, const big_number_base *b);

/********** Test/Debug Methods */

int big_number_base_test(void);

const char *big_number_base_to_str(const big_number_base *this);


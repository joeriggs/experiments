#pragma once

/*******************************************************************************
 *
 * External definition of big_number.c.
 *
 ******************************************************************************/

/******************************* CLASS DEFINITION *****************************/

typedef struct big_number big_number;

/********************************** CONSTANTS *********************************/

const big_number *big_number_0(void);

const big_number *big_number_1(void);

const big_number *big_number_2(void);

const big_number *big_number_10(void);

const big_number *big_number_1000(void);

/********************************** PUBLIC API ********************************/

big_number *big_number_new(const char *num);

void big_number_delete(big_number *this);

void big_number_copy(const big_number *src, big_number *dst);

const char *big_number_to_str(big_number *this);

int big_number_compare(const big_number *a, const big_number *b);

int big_number_is_zero(big_number *this);


void big_number_add(const big_number *addend1, const big_number *addend2, big_number *sum);

void big_number_subtract(const big_number *minuend, const big_number *subtrahend, big_number *difference);

void big_number_multiply(const big_number *factor1, const big_number *factor2, big_number *product);

void big_number_divide(const big_number *dividend, const big_number *divisor, big_number *quotient);


void big_number_increment(big_number *this);

void big_number_decrement(big_number *this);


void big_number_modulus(const big_number *this, const big_number *modulus, big_number *result);

int big_number_modulus_is_zero(big_number *this, big_number *modulus);

void big_number_exponent(big_number *base, big_number *exp, big_number *res);


/*******************************************************************************
 * Test the big_number.c ADT.
 ******************************************************************************/
int big_number_test(void);


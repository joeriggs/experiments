#pragma once

/*******************************************************************************
 *
 * External definition of big_number.c.
 *
 ******************************************************************************/

/******************************* CLASS DEFINITION *****************************/

typedef struct big_number big_number;

/********************************** PUBLIC API ********************************/

big_number *big_number_new(const char *num);

void big_number_delete(big_number *this);

const char *big_number_to_str(big_number *this);

int big_number_compare(big_number *a, big_number *b);

int big_number_is_zero(big_number *this);

void big_number_modulus(big_number *this, big_number *modulus, big_number *result);

void big_number_increment(big_number *this);

void big_number_multiply(big_number *this, big_number *factor);

void big_number_decrement(big_number *this);


/*******************************************************************************
 * Test the big_number.c ADT.
 ******************************************************************************/
int big_number_test(void);


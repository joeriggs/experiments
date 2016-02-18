/*******************************************************************************
 *
 * This module provides a simple simulation of the big_number library.  The big
 * number library is pretty fancy.  By comparison this module is very simple.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "big_number_base.h"

/*******************************************************************************
 ******************************* CLASS DEFINITION ******************************
 ******************************************************************************/

/* This is the big_number_base class. */
struct big_number_base {
	int64_t num;
};

/********************************** CONSTANTS *********************************/

/*******************************************************************************
 * Singleton: Return a pointer to a big_number_base object that contains 1.
 ******************************************************************************/
const big_number_base *big_number_base_1(void)
{
	static big_number_base *this = (big_number_base *) 0;
	if(this == (big_number_base *) 0) {
		if((this = big_number_base_new()) != (big_number_base *) 0) {
			this->num = 1;
		}
	}
	return this;
}

/********************************** PUBLIC API ********************************/

big_number_base *big_number_base_new(void)
{
	big_number_base *this = (big_number_base *) malloc(sizeof(*this));
	if(this != (big_number_base *) 0)
	{
		this->num = 0;
	}
	return this;
}

void big_number_base_delete(big_number_base *this)
{
	if(this != (big_number_base *) 0) {
		free(this);
	}
}

void big_number_base_copy(const big_number_base *src, big_number_base *dst)
{
	if((src != (big_number_base *) 0) && (dst != (big_number_base *) 0)) {
		dst->num = src->num;
	}
}

/********** Math Operations */

void big_number_base_add(const big_number_base *addend1, const big_number_base *addend2, big_number_base *sum)
{
	if((addend1 != (big_number_base *) 0) && (addend2 != (big_number_base *) 0) && (sum != (big_number_base *) 0)) {
		sum->num = addend1->num + addend2->num;
	}
}

void big_number_base_subtract(const big_number_base *minuend, const big_number_base *subtrahend, big_number_base *difference)
{
	if((minuend != (big_number_base *) 0) && (subtrahend != (big_number_base *) 0) && (difference != (big_number_base *) 0)) {
		difference->num = minuend->num - subtrahend->num;
	}
}

void big_number_base_multiply(const big_number_base *factor1, const big_number_base *factor2, big_number_base *product)
{
	if((factor1 != (big_number_base *) 0) && (factor2 != (big_number_base *) 0) && (product != (big_number_base *) 0)) {
		product->num = factor1->num * factor2->num;
	}
}

void big_number_base_divide(const big_number_base *dividend, const big_number_base *divisor, big_number_base *quotient)
{
	if((dividend != (big_number_base *) 0) && (divisor != (big_number_base *) 0) && (quotient != (big_number_base *) 0)) {
		quotient->num = dividend->num / divisor->num;
	}
}

void big_number_base_modulus(const big_number_base *this, const big_number_base *modulus, big_number_base *result)
{
	if((this != (big_number_base *) 0) && (modulus != (big_number_base *) 0) && (result != (big_number_base *) 0)) {
		result->num = this->num % modulus->num;
	}
}

/********** Comparison Methods */

int big_number_base_compare(const big_number_base *a, const big_number_base *b)
{
	int rc = 2;

	if((a != (big_number_base *) 0) && (b != (big_number_base *) 0)) {
		if(a->num < b->num) {
			rc = -1;
		}
		else if(a->num == b->num) {
			rc = 0;
		}
		else {
			rc = 1;
		}
	}

	return rc;
}


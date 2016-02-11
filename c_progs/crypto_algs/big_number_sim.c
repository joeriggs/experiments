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

#include "big_number.h"

/*******************************************************************************
 ******************************* CLASS DEFINITION ******************************
 ******************************************************************************/

#define MAX_BIG_NUMBER (0xFFFFFFFFFFFFFFFFULL)

/* This is the big_number class. */
struct big_number {
	int64_t num;

	char str[128];
};

/********************************** CONSTANTS *********************************/

/*******************************************************************************
 * Singleton: Return a pointer to a big_number object that contains 0.
 ******************************************************************************/
const big_number *
big_number_0(void)
{
	static big_number *this = (big_number *) 0;
	if(this == (big_number *) 0) {
		this = big_number_new(0);
	}
	return this;
}

/*******************************************************************************
 * Singleton: Return a pointer to a big_number object that contains 1.
 ******************************************************************************/
const big_number *
big_number_1(void)
{
	static big_number *this = (big_number *) 0;
	if(this == (big_number *) 0) {
		if((this = big_number_new(0)) != (big_number *) 0) {
			big_number_increment(this);
		}
	}
	return this;
}

/*******************************************************************************
 * Singleton: Return a pointer to a big_number object that contains 2.
 ******************************************************************************/
const big_number *
big_number_2(void)
{
	static big_number *this = (big_number *) 0;
	if(this == (big_number *) 0) {
		if((this = big_number_new(0)) != (big_number *) 0) {
			big_number_increment(this);
			big_number_increment(this);
		}
	}
	return this;
}

/*******************************************************************************
 * Singleton: Return a pointer to a big_number object that contains 10.
 ******************************************************************************/
const big_number *
big_number_10(void)
{
	static big_number *this = (big_number *) 0;
	if(this == (big_number *) 0) {
		if((this = big_number_new(0)) != (big_number *) 0) {
			big_number_add(big_number_2(), big_number_2(), this);
			big_number_add(this, this, this);
			big_number_add(this, big_number_2(), this);
		}
	}
	return this;
}

/*******************************************************************************
 * Singleton: Return a pointer to a big_number object that contains 1.
 ******************************************************************************/
const big_number *
big_number_1000(void)
{
	static big_number *this = (big_number *) 0;
	if(this == (big_number *) 0) {
		if((this = big_number_new(0)) != (big_number *) 0) {
			big_number_copy(big_number_10(), this);
			big_number_multiply(this, big_number_10(), this);
			big_number_multiply(this, big_number_10(), this);
		}
	}
	return this;
}

/********************************** PUBLIC API ********************************/

big_number *big_number_new(const char *num)
{
	big_number *this = (big_number *) malloc(sizeof(*this));
	if(this != (big_number *) 0)
	{
		this->num = 0;

		if(num != (const char *) 0)
		{
			const char *p = num;
			while(*p != 0)
			{
				char c = *(p++);

				this->num *= 10;
				this->num += (c - 0x30);
			}
		}
	}
	return this;
}

void big_number_delete(big_number *this)
{
	if(this != (big_number *) 0) {
		free(this);
	}
}

void big_number_copy(const big_number *src,
                     big_number *dst)
{
	dst->num = src->num;
	memcpy(dst->str, src->str, sizeof(dst->str));
}

const char *big_number_to_str(big_number *this)
{
	/* Chop the number down by the thousands. */
	const char *str = (char *) 0;
	big_number *tmp = (big_number *) 0;
	if(big_number_compare(this, big_number_1000()) >= 0) {
		tmp = big_number_new(0);
		big_number_divide(this, big_number_1000(), tmp);
		str = big_number_to_str(tmp);
		big_number_modulus(this, big_number_1000(), tmp);
		snprintf(this->str, sizeof(this->str), "%s,%03jd", str, tmp->num);
		big_number_delete(tmp);
	}
	else {
		snprintf(this->str, sizeof(this->str), "%jd", this->num);
	}

	return this->str;
}

int big_number_compare(const big_number *a, const big_number *b)
{
	if(a->num < b->num) {
		return -1;
	}
	else if(a->num == b->num) {
		return 0;
	}
	else
		return 1;
}

int big_number_is_zero(big_number *this)
{
	int rc = this->num == 0 ? 1 : 0;
	return rc;
}

void big_number_modulus(const big_number *this, const big_number *modulus, big_number *result)
{
	result->num = this->num % modulus->num;
}

/* Return 1 if modulus is zero.
 * Return 0 if modulus is non-zero.
 */
int big_number_modulus_is_zero(big_number *this, big_number *modulus)
{
	int64_t result = (this->num % modulus->num);
	return (result == 0) ? 1 : 0;
}

void big_number_add(const big_number *addend1, const big_number *addend2, big_number *sum)
{
	sum->num = addend1->num + addend2->num;
}

void big_number_subtract(const big_number *minuend, const big_number *subtrahend, big_number *difference)
{
	difference->num = minuend->num - subtrahend->num;
}

void big_number_multiply(const big_number *factor1, const big_number *factor2, big_number *product)
{
	product->num = factor1->num * factor2->num;
}

void big_number_divide(const big_number *dividend, const big_number *divisor, big_number *quotient)
{
	quotient->num = dividend->num / divisor->num;
}

void big_number_exponent(big_number *base, big_number *exp, big_number *res)
{
	int64_t e = exp->num;
	if(e == 0) {
		res->num = 1;
	}
	else {
		res->num = base->num;
		while(e-- > 0) {
			res->num *= base->num;
		}
	}
}

void big_number_increment(big_number *this)
{
	this->num += 1;
}

void big_number_decrement(big_number *this)
{
	this->num -= 1;
}

/*********************************** TEST API *********************************/

int big_number_test(void)
{
	int rc = 0;

	return rc;
}


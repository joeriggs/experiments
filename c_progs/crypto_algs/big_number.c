/*******************************************************************************
 *
 * This module manages big integers and does math on them.
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

/* This is the big_number class. */
struct big_number {
	int64_t num;

	char str[128];
};

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

void big_number_copy(big_number *src,
                     big_number *dst)
{
	dst->num = src->num;
	memcpy(dst->str, src->str, sizeof(dst->str));
}

const char *big_number_to_str(big_number *this)
{
	snprintf(this->str, sizeof(this->str), "%jd", this->num);
	return this->str;
}

int big_number_compare(big_number *a, big_number *b)
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

void big_number_modulus(big_number *this, big_number *modulus, big_number *result)
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

void big_number_add(big_number *addend1, big_number *addend2, big_number *sum)
{
	sum->num = addend1->num + addend2->num;
}

void big_number_subtract(big_number *minuend, big_number *subtrahend, big_number *difference)
{
	difference->num = minuend->num - subtrahend->num;
}

void big_number_multiply(big_number *factor1, big_number *factor2, big_number *product)
{
	product->num = factor1->num * factor2->num;
}

void big_number_divide(big_number *dividend, big_number *divisor, big_number *quotient)
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


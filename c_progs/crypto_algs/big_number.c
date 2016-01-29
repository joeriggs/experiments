/*******************************************************************************
 *
 * This module manages big integers and does math on them.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

		const char *p = num;
		while(*p != 0)
		{
			char c = *(p++);

			this->num *= 10;
			this->num += (c - 0x30);
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

void big_number_increment(big_number *this)
{
	this->num += 1;
}

void big_number_multiply(big_number *this, big_number *factor)
{
	this->num = this->num * factor->num;
}

void big_number_decrement(big_number *this)
{
	this->num = this->num - 1;
}

/*********************************** TEST API *********************************/

int big_number_test(void)
{
	int rc = 0;

	return rc;
}


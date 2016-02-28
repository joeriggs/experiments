/*******************************************************************************
 *
 * This module manages big integers and does math on them.  It performs a lot
 * of high-level stuff.  The nuts and bolts big_number stuff is performed by the
 * big_number_base_????.c classes.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "big_number.h"
#include "big_number_base.h"

/*******************************************************************************
 ******************************* CLASS DEFINITION ******************************
 ******************************************************************************/

/* The nuts and bolts data for the big_number class is defined in
 * big_number_base_???.c.
 *
 * There are a couple different ways to build the big_number class.  Each way
 * has its own data definition.  The members in this file merely use the
 * primitives defined in the big_number_base_???.c files to perform higher-layer
 * operations on big_number objects.
 */

/* This is the big_number class. */
struct big_number {

	big_number_base *num;

	char str[8192];
};

/********************************* PRIVATE API ********************************/

/*******************************************************************************
 * This is a worker function.  It converts (this % 1,000) to an ASCII string.
 * It is only called from big_number_to_str().
 *
 * Input:
 *   this - A pointer to the big_number object that contains the number.
 *   zero_fill - 1 == Prepend zeroes if the number < 100.
 *
 * Output:
 *   Returns a pointer to this->str.
 ******************************************************************************/
static const char *big_number_to_str_worker(big_number *this, int zero_fill)
{
	big_number *tmp1 = big_number_new();
	big_number *tmp2 = big_number_new();

	memset(this->str, 0, sizeof(this->str));

	/* Convert 1,234,567,890 to 890. */
	big_number_modulus(this, big_number_1000(), tmp1);

	/* Convert the hundreds. */
	big_number_divide(tmp1, big_number_100(), tmp2);
	char c = 0;
	while(big_number_is_zero(tmp2) == 0) {
		c++;
		big_number_decrement(tmp2);
	}
	if((c > 0) || (zero_fill == 1)) {
		if(strlen(this->str) < sizeof(this->str)) {
			this->str[strlen(this->str)] = (c + '0');
		}
	}
	big_number_modulus(tmp1, big_number_100(), tmp1);

	/* Convert the tens. */
	big_number_divide(tmp1, big_number_10(), tmp2);
	c = 0;
	while(big_number_is_zero(tmp2) == 0) {
		c++;
		big_number_decrement(tmp2);
	}
	if((c > 0) || (zero_fill == 1) || (strlen(this->str) > 0)) {
		if(strlen(this->str) < sizeof(this->str)) {
			this->str[strlen(this->str)] = (c + '0');
		}
	}
	big_number_modulus(tmp1, big_number_10(), tmp2);

	/* Convert the ones. */
	c = 0;
	while(big_number_is_zero(tmp2) == 0) {
		c++;
		big_number_decrement(tmp2);
	}
	if(strlen(this->str) < sizeof(this->str)) {
		this->str[strlen(this->str)] = (c + '0');
	}

	big_number_delete(tmp2);
	big_number_delete(tmp1);

	return this->str;
}

/********************************** CONSTANTS *********************************/

/*******************************************************************************
 * Singleton: Return a pointer to a big_number object that contains 0.
 *
 * Output:
 *   Success - Returns a pointer to the big_number object.
 *   Failure - Returns 0.
 ******************************************************************************/
const big_number *
big_number_0(void)
{
	static big_number *this = (big_number *) 0;
	if(this == (big_number *) 0) {
		this = big_number_new();
	}
	return this;
}

/*******************************************************************************
 * Singleton: Return a pointer to a big_number object that contains 1.
 *
 * Output:
 *   Success - Returns a pointer to the big_number object.
 *   Failure - Returns 0.
 ******************************************************************************/
const big_number *
big_number_1(void)
{
	static big_number *this = (big_number *) 0;
	if(this == (big_number *) 0) {
		if((this = big_number_new()) != (big_number *) 0) {
			if((this->num = big_number_base_new()) != (big_number_base *) 0) {
				big_number_base_add(this->num, big_number_base_1(), this->num);
			}

			else {
				big_number_delete(this);	
				this = (big_number *) 0;
			}
		}
	}
	return this;
}

/*******************************************************************************
 * Singleton: Return a pointer to a big_number object that contains 2.
 *
 * Output:
 *   Success - Returns a pointer to the big_number object.
 *   Failure - Returns 0.
 ******************************************************************************/
const big_number *
big_number_2(void)
{
	static big_number *this = (big_number *) 0;
	if(this == (big_number *) 0) {
		if((this = big_number_new()) != (big_number *) 0) {
			big_number_increment(this);
			big_number_increment(this);
		}
	}
	return this;
}

/*******************************************************************************
 * Singleton: Return a pointer to a big_number object that contains 10.
 *
 * Output:
 *   Success - Returns a pointer to the big_number object.
 *   Failure - Returns 0.
 ******************************************************************************/
const big_number *
big_number_10(void)
{
	static big_number *this = (big_number *) 0;
	if(this == (big_number *) 0) {
		if((this = big_number_new()) != (big_number *) 0) {
			big_number_add(big_number_2(), big_number_2(), this);
			big_number_add(this, this, this);
			big_number_add(this, big_number_2(), this);
		}
	}
	return this;
}

/*******************************************************************************
 * Singleton: Return a pointer to a big_number object that contains 100.
 *
 * Output:
 *   Success - Returns a pointer to the big_number object.
 *   Failure - Returns 0.
 ******************************************************************************/
const big_number *
big_number_100(void)
{
	static big_number *this = (big_number *) 0;
	if(this == (big_number *) 0) {
		if((this = big_number_new()) != (big_number *) 0) {
			big_number_copy(big_number_10(), this);
			big_number_multiply(this, big_number_10(), this);
		}
	}
	return this;
}

/*******************************************************************************
 * Singleton: Return a pointer to a big_number object that contains 1,000.
 *
 * Output:
 *   Success - Returns a pointer to the big_number object.
 *   Failure - Returns 0.
 ******************************************************************************/
const big_number *
big_number_1000(void)
{
	static big_number *this = (big_number *) 0;
	if(this == (big_number *) 0) {
		if((this = big_number_new()) != (big_number *) 0) {
			big_number_copy(big_number_10(), this);
			big_number_multiply(this, big_number_10(), this);
			big_number_multiply(this, big_number_10(), this);
		}
	}
	return this;
}

/********************************** PUBLIC API ********************************/

/********** Create, Delete, Initialize Methods */

/*******************************************************************************
 * Create a big_number object.
 *
 * Output:
 *   Success - Returns a pointer to the big_number object.
 *   Failure - Returns 0 if failure.
 ******************************************************************************/
big_number *big_number_new(void)
{
	big_number *this = (big_number *) malloc(sizeof(*this));
	if(this != (big_number *) 0) {
		if((this->num = big_number_base_new()) == (big_number_base *) 0) {
			big_number_delete(this);
			this = (big_number *) 0;
		}
	}

	return this;
}

/*******************************************************************************
 * Destroy a big_number object that was created by big_number_new().
 *
 * Input:
 *   this - A pointer to the object.
 ******************************************************************************/
void big_number_delete(big_number *this)
{
	if(this != (big_number *) 0) {
		big_number_base_delete(this->num);
		free(this);
	}
}

/*******************************************************************************
 * Reset a big_number object to zero.
 *
 * Input:
 *   this - A pointer to the object.
 ******************************************************************************/
void big_number_reset(big_number *this)
{
	if(this != (big_number *) 0) {
		big_number_copy(big_number_0(), this);
	}
}

/*******************************************************************************
 * Copy the contents of one big_number object to another.
 *
 * Input:
 *   src - A pointer to the source object.
 *   dst - A pointer to the destination object.
 ******************************************************************************/
void big_number_copy(const big_number *src, big_number *dst)
{
	if((src != (big_number *) 0) && (dst != (big_number *) 0)) {
		big_number_base_copy(src->num, dst->num);
	}
}

/********** Math Operations */

/*******************************************************************************
 * Add 2 big_number objects.
 *
 * Input:
 *   addend1 - One of the values to add.
 *   addend2 - One of the values to add.
 *   sum - A pointer to the object that will receive the sum.
 ******************************************************************************/
void big_number_add(const big_number *addend1, const big_number *addend2, big_number *sum)
{
	if((addend1 != (big_number *) 0) && (addend2 != (big_number *) 0) && (sum != (big_number *) 0)) {
		big_number_base_add(addend1->num, addend2->num, sum->num);
	}
}

/*******************************************************************************
 * Subtract 2 big_number objects.
 *
 * Input:
 *   minuend    - Value 1.
 *   subtrahend - Value 2.
 *   difference - A pointer to the object that will receive the result.
 ******************************************************************************/
void big_number_subtract(const big_number *minuend, const big_number *subtrahend, big_number *difference)
{
	if((minuend != (big_number *) 0) && (subtrahend != (big_number *) 0) && (difference != (big_number *) 0)) {
		big_number_base_subtract(minuend->num, subtrahend->num, difference->num);
	}
}

/*******************************************************************************
 * Multiply 2 big_number objects.
 *
 * Input:
 *   factor1 - One of the factors.
 *   factor2 - One of the factors.
 *   product - A pointer to the object that will receive the result.
 ******************************************************************************/
void big_number_multiply(const big_number *factor1, const big_number *factor2, big_number *product)
{
	if((factor1 != (big_number *) 0) && (factor2 != (big_number *) 0) && (product != (big_number *) 0)) {
		big_number_base_multiply(factor1->num, factor2->num, product->num);
	}
}

/*******************************************************************************
 * Divide 2 big_number objects.
 *
 * Input:
 *   dividend - Value 1.
 *   divisor  - Value 2.
 *   quotient - A pointer to the object that will receive the result.
 ******************************************************************************/
void big_number_divide(const big_number *dividend, const big_number *divisor, big_number *quotient)
{
	if((dividend != (big_number *) 0) && (divisor != (big_number *) 0) && (quotient != (big_number *) 0)) {
		big_number_base_divide(dividend->num, divisor->num, quotient->num);
	}
}

/*******************************************************************************
 * Increment the value of a big_number object.  Doesn't check for wrap.
 *
 * Input:
 *   this - The object to increment.
 ******************************************************************************/
void big_number_increment(big_number *this)
{
	if(this != (big_number *) 0) {
		big_number_add(this, big_number_1(), this);
	}
}

/*******************************************************************************
 * Decrement the value of a big_number object.  Doesn't check for wrap.
 *
 * Input:
 *   this - The object to decrement.
 ******************************************************************************/
void big_number_decrement(big_number *this)
{
	if(this != (big_number *) 0) {
		big_number_subtract(this, big_number_1(), this);
	}
}

/*******************************************************************************
 * Perform a modulus operation on a big_number object.  The operation is done
 * as: result = this % modulus.
 *
 * Input:
 *   this    - The object to perform the modulus against.
 *   modulus - The modulus.
 *   result  - A pointer to the big_number object that receives the result.
 ******************************************************************************/
void big_number_modulus(const big_number *this, const big_number *modulus, big_number *result)
{
	if((this != (big_number *) 0) && (modulus != (big_number *) 0) && (result != (big_number *) 0)) {
		big_number_base_modulus(this->num, modulus->num, result->num);
	}
}

/*******************************************************************************
 * Perform an exponentiation operation.  The opeation is done as follows:
 * result = base ^^ exp.
 *
 * Input:
 *   base - The base of the exponentiation.
 *   exp  - The exponent.
 *   res  - A pointer to the big_number object that receives the result.
 ******************************************************************************/
void big_number_exponent(const big_number *base, const big_number *exp, big_number *result)
{
	if((base != (big_number *) 0) && (exp != (big_number *) 0) && (result != (big_number *) 0)) {
		/* Special case.  (base ^ 0) equals 1. */
		if(big_number_is_zero(exp) == 1) {
			big_number_copy(big_number_1(), result);
			return;
		}

		/* 1. Make local copies of the base and exponent.
		 * 2. Start with res = 0.
		 * 3. Create *tmp (A big_number object for misc tasks).
		 */
		big_number *b   = big_number_new();
		big_number_copy(base, b);
		big_number *e   = big_number_new();
		big_number_copy(exp,  e);
		big_number_reset(result);
		big_number *tmp = big_number_new();

		while(big_number_is_zero(e) == 0) {
			big_number_modulus(e, big_number_2(), tmp);
			if(big_number_is_zero(tmp) == 0) {
				big_number_add(result, b, result);
			}

			big_number_multiply(b, big_number_2(), b);
			big_number_divide(e, big_number_2(), e);
		}

		big_number_delete(tmp);
		big_number_delete(e);
		big_number_delete(b);
	}
}

/********** Comparison Methods */

/*******************************************************************************
 * Performs a modulus operation and checks to see if the result is zero.  It
 * essentially performs (this % modulus) and checks to see if the result is
 * zero.
 *
 * Input:
 *   this    - The value to perform the modulus against.
 *   modulus - The modulus value.
 *
 * Output:
 *   Return 1 if the modulus result is zero.
 *   Return 0 if the modulus result is non-zero.
 ******************************************************************************/
int big_number_modulus_is_zero(big_number *this, big_number *modulus)
{
	int rc = -1;
	if((this != (big_number *) 0) && (modulus != (big_number *) 0)) {
		big_number *tmp = big_number_new();
		big_number_modulus(this, modulus, tmp);
		rc = big_number_is_zero(tmp);
		big_number_delete(tmp);
	}

	return rc;
}

/*******************************************************************************
 * Compares 2 big_number objects to see if the internal values are the same.
 *
 * Input:
 *   a - Value 1.
 *   b - Value 2.
 *
 * Output:
 *   Returns <0 if (a < b).
 *   Returns  0 if (a == b).
 *   Returns >0 if (a > b).
 ******************************************************************************/
int big_number_compare(const big_number *a, const big_number *b)
{
	int res = 2;

	if((a != (big_number *) 0) && (b != (big_number *) 0)) {
		res = big_number_base_compare(a->num, b->num);
	}

	return res;
}

/*******************************************************************************
 * Check to see if a big_number object equals zero.  It checks the value of the
 * object.
 *
 * Input:
 *   this - The object to check.
 *
 * Output:
 *   1 - The number is zero.
 *   0 - The number is not zero.
 ******************************************************************************/
int big_number_is_zero(const big_number *this)
{
	int rc = 2;

	if(this != (big_number *) 0) {
		rc = (big_number_compare(this, big_number_0()) == 0);
	}

	return rc;
}

/********** Test Methods */

/*******************************************************************************
 * Run the big_number tests.
 *
 * Output:
 *   Success - 0.
 *   Failure - 1.
 ******************************************************************************/
int big_number_test(void)
{
	int rc = 1;

	printf("%s(): Starting.\n", __func__);
	big_number *this = big_number_new();
	if(this != (big_number *) 0) {
		rc = big_number_from_str(this, "123");
		printf("big_number_from_str() returned %d.\n", rc);

		big_number_delete(this);
	}
	printf("%s(): Returning %d.\n", __func__, rc);
	return rc;
}

/*******************************************************************************
 * This function is used by test code.  It allows a test function to convert a
 * string ("123456") to a big_number.
 *
 * Input:
 *   this - The object to store the value in.
 *   str  - The string to convert to a big_number value.
 *
 * Output:
 *   Success - 0.
 *   Failure - 1.
 ******************************************************************************/
int big_number_from_str(big_number *this, const char *str)
{
	int rc = 1;
	if((this != (big_number *) 0) && (str != (const char *) 0)) {
		big_number *tmp = big_number_new();

		if(tmp != (big_number *) 0) {
			/* Walk through the string and convert to a big_number. */
			big_number_reset(this);
			while(*str) {
				/* Build a big_number that equals the next digit. */
				char c = *(str++) - 0x30;
				big_number_reset(tmp);
				while(c--) {
					big_number_increment(tmp);
				}

				/* Multiply by 10, then add the digit. */
				big_number_multiply(this, big_number_10(), this);
				big_number_add(this, tmp, this);
			}

			big_number_delete(tmp);
			rc = 0;
		}
	}

	return rc;
}

/*******************************************************************************
 * Produce an ASCII string from a big_number object.
 *
 * Input:
 *   this - The big_number object to convert.
 *
 * Output:
 *   Returns a pointer to the ASCII string.
 *   Returns 0 if an error occurs.
 ******************************************************************************/
const char *big_number_to_str(big_number *this)
{
	const char *rc = (const char *) 0;
	if(this != (big_number *) 0) {
		/* Chop the number down by the thousands. */
		if(big_number_compare(this, big_number_1000()) >= 0) {
			big_number *tmp = big_number_new();
			big_number_divide(this, big_number_1000(), tmp);
			snprintf(this->str, sizeof(this->str), "%s,", big_number_to_str(tmp));

			big_number_modulus(this, big_number_1000(), tmp);
			strncat(this->str, big_number_to_str_worker(tmp, 1), (sizeof(this->str) - strlen(this->str)));
			big_number_delete(tmp);
		}
		else {
			strncpy(this->str, big_number_to_str_worker(this, 0), sizeof(this->str));
		}

		rc = this->str;
	}

	return rc;
}


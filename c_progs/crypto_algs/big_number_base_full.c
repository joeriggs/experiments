/*******************************************************************************
 *
 * This module provides the primitives required by the big_number class.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "big_number_base.h"

/*******************************************************************************
 ******************************* CLASS DEFINITION ******************************
 ******************************************************************************/

/* This is the big_number_base class. */
struct big_number_base {
	uint8_t num[8];
};

/****************************** PRIVATE CONSTANTS *****************************/

/*******************************************************************************
 * Singleton: Return a pointer to a big_number_base object that contains 1.
 *
 * Output:
 *   Success - Returns a pointer to the big_number object.  The object is
 *   treated as a read only object.
 *   Failure - Returns 0.
 ******************************************************************************/
static const big_number_base *big_number_base_0(void)
{
	static big_number_base *this = (big_number_base *) 0;
	if(this == (big_number_base *) 0) {
		this = big_number_base_new();
	}
	return this;
}

/*******************************************************************************
 * Singleton: Return a pointer to a big_number_base object that contains 1.
 *
 * Output:
 *   Success - Returns a pointer to the big_number object.  The object is
 *   treated as a read only object.
 *   Failure - Returns 0.
 ******************************************************************************/
const big_number_base *big_number_base_1(void)
{
	static big_number_base *this = (big_number_base *) 0;
	if(this == (big_number_base *) 0) {
		if((this = big_number_base_new()) != (big_number_base *) 0) {
			this->num[0] = 1;
		}
	}
	return this;
}

/********************************* PRIVATE API ********************************/

/*******************************************************************************
 * Divide 2 big_number_base objects.  Return quotient and/or remainder.
 *
 * Input:
 *   dividend  - Value 1.
 *   divisor   - Value 2.
 *   quotient  - (optional) pointer to the dest result.
 *   remainder - (optional) pointer to the dest remainder (a.k.a. modulus).
 *
 * Output:
 *   quotient and remainder contain the result.
 ******************************************************************************/
static void
big_number_base_div_mod(const big_number_base *dividend,
                        const big_number_base *divisor,
                        big_number_base *quotient,
                        big_number_base *remainder)
{
	if((dividend != (big_number_base *) 0) && (divisor != (big_number_base *) 0)) {
		//printf("dividend     = %s\n", big_number_base_to_str(dividend));
		//printf("divisor      = %s\n", big_number_base_to_str(divisor));

		big_number_base rem;
		big_number_base_copy(dividend, &rem);

		big_number_base d2;
		big_number_base_copy(divisor, &d2);

		big_number_base q;
		big_number_base_copy(big_number_base_0(), &q);

		int shift_count = 0;
		while(d2.num[sizeof(d2.num) - 1] == 0) {
			int x;
			for(x = (sizeof(d2.num) - 1); x > 0; x--) {
				d2.num[x] = d2.num[x - 1];
			}
			d2.num[0] = 0;
			shift_count++;
		}

		int digit;
		for(digit = 0; (digit < sizeof(d2.num)) && (d2.num[digit] == 0); digit++);

		//printf("divisor_mod  = %s.  Shifted %d times.  digit = %d\n", big_number_base_to_str(&d2), shift_count, digit);

		while(shift_count >= 0) {
			while(big_number_base_compare(&rem, &d2) >= 0) {
				big_number_base_subtract(&rem, &d2, &rem);
				q.num[digit]++;
			}

			/* Shift the number one byte to the right. */
			int x;
			for(x = 1; x < sizeof(d2.num); x++) {
				d2.num[x - 1] = d2.num[x];
			}
			d2.num[sizeof(d2.num) - 1] = 0;

			digit--;
			shift_count--;
		}

		//printf("quotient = %s.  Remainder = %s.\n", big_number_base_to_str(&q), big_number_base_to_str(&rem));

		if(quotient != (big_number_base *) 0) {
			big_number_base_copy(&q, quotient);
		}
		if(remainder != (big_number_base *) 0) {
			big_number_base_copy(&rem, remainder);
		}
	}
}

/********************************** PUBLIC API ********************************/

/*******************************************************************************
 * Create a big_number_base object.
 *
 * Output:
 *   Success - Returns a pointer to the big_number_base object.
 *   Failure - Returns 0 if failure.
 ******************************************************************************/
big_number_base *big_number_base_new(void)
{
	big_number_base *this = (big_number_base *) malloc(sizeof(*this));
	if(this != (big_number_base *) 0)
	{
		memset(this->num, 0, sizeof(this->num));
	}
	return this;
}

/*******************************************************************************
 * Destroy a big_number_base object that was created by big_number_base_new().
 *
 * Input:
 *   this - A pointer to the object.
 ******************************************************************************/
void big_number_base_delete(big_number_base *this)
{
	if(this != (big_number_base *) 0) {
		free(this);
	}
}

/*******************************************************************************
 * Copy the contents of one big_number_base object to another.
 *
 * Input:
 *   src - A pointer to the source object.
 *   dst - A pointer to the destination object.
 ******************************************************************************/
void big_number_base_copy(const big_number_base *src, big_number_base *dst)
{
	if((src != (big_number_base *) 0) && (dst != (big_number_base *) 0)) {
		memcpy(dst->num, src->num, sizeof(dst->num));
	}
}

/********** Math Operations */

/*******************************************************************************
 * Add 2 big_number_base objects.
 *
 * Input:
 *   addend1 - One of the values to add.
 *   addend2 - One of the values to add.
 *   sum - A pointer to the object that will receive the sum.
 ******************************************************************************/
void big_number_base_add(const big_number_base *addend1, const big_number_base *addend2, big_number_base *sum)
{
	if((addend1 != (big_number_base *) 0) && (addend2 != (big_number_base *) 0) && (sum != (big_number_base *) 0)) {
		/* Make copies of the addends.  So if (sum == addend[12]), then
		 * we won't overwrite the addend as we're working. */
		big_number_base a1, a2;
		big_number_base_copy(addend1, &a1);
		big_number_base_copy(addend2, &a2);

		/* Clear the result before we start. */
		big_number_base_copy(big_number_base_0(), sum);

		int i;
		for(i = 0; i < sizeof(a1.num); i++) {
			/* Calculate the value and check for carry. */
			uint8_t value = (uint8_t) ((sum->num[i] + a1.num[i] + a2.num[i]) & 0xFF);
			uint8_t carry = (uint8_t) ((sum->num[i] + a1.num[i] + a2.num[i]) >> 8);

			sum->num[i] = value;
			if((carry > 0) && ((i + i) < sizeof(a1.num))) {
				sum->num[i + 1] += 1;
			}
		}
	}
}

/*******************************************************************************
 * Subtract 2 big_number_base objects.
 *
 * Input:
 *   minuend    - Value 1.
 *   subtrahend - Value 2.
 *   difference - A pointer to the object that will receive the result.
 ******************************************************************************/
void big_number_base_subtract(const big_number_base *minuend, const big_number_base *subtrahend, big_number_base *difference)
{
	if((minuend != (big_number_base *) 0) && (subtrahend != (big_number_base *) 0) && (difference != (big_number_base *) 0)) {
		/* Make a copy of the minuend.  We might need to modify if as we
		 * work our way through the number (if we have to borrow). */
		big_number_base min;
		big_number_base_copy(minuend, &min);

		/* Clear the result before we start. */
		big_number_base_copy(big_number_base_0(), difference);

		int i;
		for(i = 0; i < sizeof(min.num); i++) {
			uint16_t val1 = min.num[i];
			uint16_t val2 = subtrahend->num[i];

			/* Check to see if we need to borrow. */
			if(val2 > val1) {
				int x;
				for(x = (i + 1); x < sizeof(min.num); x++) {
					min.num[x] -= 1;
					if(min.num[x] != 0xFF) {
						val1 += 0x100;
						break;
					}
				}
				if(val1 >= val2) {
					difference->num[i] = val1 - val2;
				}

				/* We couldn't borrow.  Give up. */
				else {
					break;
				}
			}

			/* No need to borrow.  Straight subtraction. */
			else {
				difference->num[i] = val1 - val2;
			}
		}
	}
}

/*******************************************************************************
 * Multiply 2 big_number_base objects.
 *
 * Input:
 *   factor1 - One of the factors.
 *   factor2 - One of the factors.
 *   product - A pointer to the object that will receive the result.
 ******************************************************************************/
void big_number_base_multiply(const big_number_base *factor1, const big_number_base *factor2, big_number_base *product)
{
	if((factor1 != (big_number_base *) 0) && (factor2 != (big_number_base *) 0) && (product != (big_number_base *) 0)) {
		/* Use a temp product, in case product == factor1 or factor2. */
		big_number_base tmp_product;
		big_number_base_copy(big_number_base_0(), &tmp_product);

		int x;
		for(x = 0; x < sizeof(factor1->num); x++) {
			/* Results for one pass go into here. */
			big_number_base loop_prod;
			big_number_base_copy(big_number_base_0(), &loop_prod);

			/* Overflows for one pass go into here. */
			big_number_base loop_ovfl;
			big_number_base_copy(big_number_base_0(), &loop_ovfl);

			int y;
			for(y = 0; y < sizeof(factor1->num); y++) {
				if((factor1->num[x] > 0) && (factor2->num[y] > 0)) {
					uint16_t res = factor1->num[x] * factor2->num[y];

					if((x + y) < sizeof(loop_prod.num)) {
						loop_prod.num[x + y] = (uint8_t) ((res >> 0) & 0xFF);

						if((x + y + 1) < sizeof(loop_ovfl.num)) {
							loop_ovfl.num[x + y + 1] = (uint8_t) ((res >> 8) & 0xFF);
						}
					}
				}
			}

			/* Add to the running total. */
			big_number_base_add(&tmp_product, &loop_prod, &tmp_product);
			big_number_base_add(&tmp_product, &loop_ovfl, &tmp_product);
		}

		/* Copy our internal product to the caller's product. */
		big_number_base_copy(&tmp_product, product);
	}
}

/*******************************************************************************
 * Divide 2 big_number_base objects.
 *
 * Input:
 *   dividend  - Value 1.
 *   divisor   - Value 2.
 *   quotient  - A pointer to the object that will receive the result.
 ******************************************************************************/
void big_number_base_divide(const big_number_base *dividend,
                            const big_number_base *divisor,
                            big_number_base *quotient)
{
	big_number_base_div_mod(dividend, divisor, quotient, 0);
}

/*******************************************************************************
 * Perform a modulus operation on a big_number_base object.  The operation is
 * done as: result = this % modulus.
 *
 * Input:
 *   this    - The object to perform the modulus against.
 *   modulus - The modulus.
 *   result  - A pointer to the big_number object that receives the result.
 ******************************************************************************/
void big_number_base_modulus(const big_number_base *this, const big_number_base *modulus, big_number_base *result)
{
	big_number_base_div_mod(this, modulus, 0, result);
}

/********** Comparison Methods */

/*******************************************************************************
 * Compares 2 big_number_base objects to see if the internal values are the
 * same.
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
int big_number_base_compare(const big_number_base *a, const big_number_base *b)
{
	int rc = 2;

	if((a != (big_number_base *) 0) && (b != (big_number_base *) 0)) {

		//printf("Compare %s and %s\n", big_number_base_to_str(a), big_number_base_to_str(b));

		int index = sizeof(a->num);
		rc = 0;

		for(index = (sizeof(a->num) - 1); (index >= 0) && (rc == 0); index--) {

			if(a->num[index] < b->num[index]) {
				rc = -1;
			}

			else if(a->num[index] > b->num[index]) {
				rc = 1;
			}

		} while((index > 0) && (rc == 0));
	}

	return rc;
}

/********** Test/Debug Methods */

#ifdef TEST
/*******************************************************************************
 * Run the big_number_base tests.
 *
 * Output:
 *   Success - 0.
 *   Failure - 1.
 ******************************************************************************/
int big_number_base_test(void)
{
	int rc = 1;

	printf("%s(): Starting.\n", __func__);
	big_number_base *obj = big_number_base_new();
	if(obj != (big_number_base *) 0) {
		big_number_base_delete(obj);
	}

	big_number_base num1 = {
		.num[0] = 0xF8,
		.num[1] = 0xF8
	};
	big_number_base num2 = {
		.num[0] = 0x15,
		.num[1] = 0x22
	};
	big_number_base num3;

	/* Addition test. */
	big_number_base_copy(big_number_base_0(), &num3);
	big_number_base_add(&num1, &num2, &num3);
	printf("%s\n", big_number_base_to_str(&num3)); // 01:1B:0D

	/* Subtraction test. */
	big_number_base_copy(big_number_base_0(), &num1);
	big_number_base_subtract(&num3, &num2, &num1);
	printf("%s\n", big_number_base_to_str(&num1)); // 00:F8:F8

	/* Multiplication test. */
	big_number_base_copy(big_number_base_0(), &num3);
	big_number_base_multiply(&num1, &num2, &num3);
	printf("%s\n", big_number_base_to_str(&num3)); // 21:25:5C:58

	/* Division test. */
	big_number_base_copy(big_number_base_0(), &num1);
	big_number_base_divide(&num3, &num2, &num1);
	printf("%s\n", big_number_base_to_str(&num1)); // 00:F8:F8

	printf("%s(): Returning %d.\n", __func__, rc);
	rc = 0;

	return rc;
}
#endif /* TEST */

/*******************************************************************************
 * Returns a string that contains the contents of a big_number_base object.
 *
 * NOTE: This function can only handle a few strings at a time.  So don't load
 *       too many into a printf().
 *
 * Input:
 *   this - The big_number_base object to convert to a string.
 *
 * Output:
 *   A string that contains the number.
 ******************************************************************************/
const char *
big_number_base_to_str(const big_number_base *this)
{
	static char strings[5][1024];
	static int  strings_index = 0;

	char *str = strings[strings_index];
	char *str_tmp = str;

	if(++strings_index == 5) {
		strings_index = 0;
	}

	int i;
	for(i = (sizeof(this->num) - 1); i >= 0; i--) {
		sprintf(str_tmp, "%02X", this->num[i]);
		if(i > 0) {
			strcat(str_tmp, ":");
		}
		str_tmp += strlen(str_tmp);
	}

	return str;
}


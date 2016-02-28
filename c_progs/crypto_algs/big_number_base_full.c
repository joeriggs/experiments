/*******************************************************************************
 *
 * This module provides the primitives required by the big_number class.
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
	uint8_t num[8];
};

/********************************* PRIVATE API ********************************/

/********************************** CONSTANTS *********************************/

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
		/* Clear the result before we start. */
		big_number_base_copy(big_number_base_0(), sum);

		int i;
		for(i = 0; i < sizeof(addend1->num); i++) {
			/* Calculate the value and check for carry. */
			uint8_t value = (uint8_t) ((sum->num[i] + addend1->num[i] + addend2->num[i]) & 0xFF);
			uint8_t carry = (uint8_t) ((sum->num[i] + addend1->num[i] + addend2->num[i]) >> 8);

			sum->num[i] = value;
			if((carry > 0) && ((i + i) < sizeof(addend1->num))) {
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
			printf("val1 %d (%02X): val2 %d (%02X).\n", val1, val1, val2, val2);

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
	}
}

/*******************************************************************************
 * Divide 2 big_number_base objects.
 *
 * Input:
 *   dividend - Value 1.
 *   divisor  - Value 2.
 *   quotient - A pointer to the object that will receive the result.
 ******************************************************************************/
void big_number_base_divide(const big_number_base *dividend, const big_number_base *divisor, big_number_base *quotient)
{
	if((dividend != (big_number_base *) 0) && (divisor != (big_number_base *) 0) && (quotient != (big_number_base *) 0)) {
	}
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
	if((this != (big_number_base *) 0) && (modulus != (big_number_base *) 0) && (result != (big_number_base *) 0)) {
	}
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
//		if(a->num < b->num)
		{
			rc = -1;
		}
//		else if(a->num == b->num)
		{
			rc = 0;
		}
//		else
		{
			rc = 1;
		}
	}

	return rc;
}

#ifdef TEST
/********** Test Methods */

/*******************************************************************************
 * Print the contents of a big_number_base object.
 *
 * Input:
 *   this - The big_number_base object to print.
 *
 * Output:
 *   N/A.
 ******************************************************************************/
static void
big_number_base_print(big_number_base *this)
{
	int i;
	for(i = (sizeof(this->num) - 1); i >= 0; i--) {
		printf("%02x:", this->num[i]);
	}
	printf("\n");
}

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
	big_number_base_copy(big_number_base_0(), &num3);
	big_number_base_add(&num1, &num2, &num3); // 01:1B:0D
	big_number_base_print(&num3);

	printf("Subtracting\n  ");
	big_number_base_print(&num3);
	printf("- ");
	big_number_base_print(&num2);

	big_number_base_copy(big_number_base_0(), &num1);
	big_number_base_subtract(&num3, &num2, &num1); // 00:F8:F8
	big_number_base_print(&num1);

	printf("%s(): Returning %d.\n", __func__, rc);
	return rc;
}
#endif /* TEST */


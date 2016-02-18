/*******************************************************************************
 *
 * This module calculates an RSA Public and Private key pair.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>

#include "big_number.h"
#include "rsa.h"

#define PRINTF printf

/*******************************************************************************
 * Given 2 numbers (assumed to be prime), calcuate phi ((p - 1 * (q - 1)).
 ******************************************************************************/
static void
calculate_phi(big_number *p,
              big_number *q,
              big_number *phi)
{
	big_number *p_temp = big_number_new();
	big_number_copy(p, p_temp);
	big_number_decrement(p_temp);

	big_number *q_temp = big_number_new();
	big_number_copy(q, q_temp);
	big_number_decrement(q_temp);

	big_number_copy(p_temp, phi);
	big_number_multiply(phi, q_temp, phi);

	big_number_delete(p_temp);
	big_number_delete(q_temp);
}

/*******************************************************************************
 * Given p, q, and e, calculate d.
 *
 * Returns 0 if success.
 * Returns 1 if failure.
 ******************************************************************************/
static int
calculate_d(big_number *p,
            big_number *q,
            big_number *e,
            big_number *d)
{
	PRINTF("Calculate d from p %s, q  %s, e %s.\n",
	       big_number_to_str(p),
	       big_number_to_str(q),
	       big_number_to_str(e));

	/* FYI: n = (p * q). */

	/* Calculate phi (p - 1) * (q - 1). */
	big_number *phi = big_number_new();
	calculate_phi(p, q, phi);
	PRINTF("          phi = %s.\n", big_number_to_str(phi));

	/* Make sure e doesn't share a factor with phi. */
	if(big_number_modulus_is_zero(phi, e) == 1) {
		PRINTF("e won't work (%s mod %s != 0).\n",
		       big_number_to_str(phi),
		       big_number_to_str(e));
		return 1;
	}

	/* Calculate d using the Extended Euclidian Algorithm. */
	big_number *tmp =  big_number_new();
	big_number *zero = big_number_new();
	{
		big_number *val1a = big_number_new();
		big_number_copy(phi, val1a);

		big_number *val1b = big_number_new();
		big_number_copy(e, val1b);

		big_number *val2a = big_number_new();
		big_number_copy(phi, val2a);

		big_number *val2b = big_number_new();
		big_number_copy(big_number_1(), val2b);

		do {
			big_number *x = big_number_new();
			big_number_divide(val1a, val1b, x);

			big_number *val1c = big_number_new();
			big_number_multiply(x, val1b, tmp);
			big_number_subtract(val1a, tmp, val1c);

			big_number *val2c = big_number_new();
			big_number_multiply(x, val2b, tmp);
			big_number_subtract(val2a, tmp, val2c);

			while(big_number_compare(val1c, zero) < 0) {
				big_number_add(val1c, phi, val1c);
			}
			while(big_number_compare(val2c, zero) < 0) {
				big_number_add(val2c, phi, val2c);
			}

			big_number_copy(val1b, val1a);
			big_number_copy(val1c, val1b);

			big_number_copy(val2b, val2a);
			big_number_copy(val2c, val2b);
		} while(big_number_compare(val1b, big_number_1()) != 0);

		big_number_copy(val2b, d);
		PRINTF("          d = %s.\n", big_number_to_str(d));
	}

	/* Test e and d.  (e * d) mod phi = 1. */
	big_number_multiply(e, d, tmp);
	big_number_modulus(tmp, phi, tmp);
	if(big_number_compare(tmp, big_number_1()) != 0) {
		PRINTF("e and d don't work (%s & %s) %s.\n",
		        big_number_to_str(e),
		        big_number_to_str(d),
		        big_number_to_str(phi));
		return 1;
	}

	PRINTF("Done: p %s: q %s: phi %s: e %s: d %s.\n",
	         big_number_to_str(p),
	         big_number_to_str(q),
	         big_number_to_str(phi),
	         big_number_to_str(e),
	         big_number_to_str(d));

	return 0;
}

/*******************************************************************************
 * Given p, q, and e, calculate d.
 *
 * Returns 0 on success.
 * Returns 1 on failure.
 ******************************************************************************/
int rsa_calculate_d(big_number *p,
                    big_number *q,
                    big_number *e,
                    big_number *d)
{
	return calculate_d(p, q, e, d);
}

#ifdef TEST
/*******************************************************************************
 *
 * Here are a couple sets of test data:
 *
 * p =       5, q =      11, e =  7, d =             23
 * p =      61, q =      53, e = 17, d =          2,753
 * p =     113, q =      91, e = 17, d =            593
 * p = 170,497, p = 170,503, e =  5, d = 11,627,963,597
 * p = 170,497, p = 170,503, e = 11, d = 18,499,032,995
 *
 ******************************************************************************/
int rsa_test(void)
{
	int rc = 1;

	typedef struct test_data {
		const char *p;
		const char *q;
		const char *e;
	} test_data;
	test_data tests[] = {
		{      "5",     "11",  "7" },
		{     "61",     "53", "17" },
		{    "113",     "91", "17" },
		{ "170497", "170503",  "5" },
		{ "170497", "170503", "11" }
	};
	int test_data_size = (sizeof(tests) / sizeof(test_data));

	/* These are the p, q, e, and d values used by RSA. */
	big_number *p = big_number_new();
	big_number *q = big_number_new();
	big_number *e = big_number_new();
	big_number *d = big_number_new();

	if(p && q && e && d) {
		int i;
		for(i = 0; i < test_data_size; i++) {
			big_number_from_str(p, tests[i].p);
			big_number_from_str(q, tests[i].q);
			big_number_from_str(e, tests[i].e);

			int result = calculate_d(p, q, e, d);
			printf("Test %3d %s.\n", i, (result == 0) ? "PASSED" : "FAILED");
		}

		big_number_delete(p);
		big_number_delete(q);
		big_number_delete(e);
		big_number_delete(d);
	}

	return rc;
}
#endif /* TEST */


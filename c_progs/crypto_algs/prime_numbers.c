/*******************************************************************************
 *
 * This module calculates some prime numbers.  Nothing fancy here.  It just
 * tries every possible factor to see if it can find a prime number.  So don't
 * try to discover large primes with this program.
 *
 ******************************************************************************/

#include <stdio.h>
#include <time.h>

#include "big_number.h"
#include "prime_numbers.h"

/* Styles of testing. */
#ifdef TEST_REGRESSION
#undef DISPLAY_ONLY_PRIMES
#undef TEST_ALL_INTEGERS
#else /* TEST_REGRESSION */
#define DISPLAY_ONLY_PRIMES
#define TEST_ALL_INTEGERS
#endif /* TEST_REGRESSION */

#ifdef TEST
/*******************************************************************************
 * Given p, try to determine if it's prime by trying all possible factors.  This
 * is essentially a brute force test.  We'll test up to (p / 2) to see if we can
 * find a factor.
 *
 * Returns 1 if p is prime.
 * Returns 0 if p is NOT prime.
 ******************************************************************************/
static int
prime_numbers_is_prime(big_number *p, clock_t *elapsed_time)
{
	int rc = 1;

	/* Test up to ((p / 2) + 1). */
	big_number *limit = big_number_new();
	big_number_divide(p, big_number_2(), limit);
	big_number_increment(limit);

	/* Start with modulus 2. Increment each time through the loop. */
	big_number *i = big_number_new();
	big_number_copy(big_number_2(), i);

	big_number *result = big_number_new();
	clock_t start_time = clock();
	while(big_number_compare(i, limit) < 0) {
		big_number_modulus(p, i, result);
		if(big_number_is_zero(result)) {
			rc = 0;
			break;
		}
		big_number_increment(i);
	}
	clock_t end_time = clock();
	*elapsed_time = end_time - start_time;

	big_number_delete(result);
	big_number_delete(i);
	big_number_delete(limit);
	return rc;
}
#endif /* TEST */

#ifdef TEST
/*******************************************************************************
 *
 * Spin in a loop and look for prime numbers.  Print the ones that are prime.
 *
 ******************************************************************************/
int prime_numbers_test(void)
{
	printf("%s(): Starting\n", __func__);
	int rc = 1;
	clock_t elapsed_time;

	do {
		big_number *p = big_number_new();
		if(p == 0) { break; }

#if defined(TEST_REGRESSION)
		big_number_copy(big_number_10(), p);
		if(prime_numbers_is_prime(p, &elapsed_time) != 0) { break; }

		big_number_add(p, big_number_1(), p);
		if(prime_numbers_is_prime(p, &elapsed_time) != 1) { break; }

#elif defined(TEST_ALL_INTEGERS)
		/* Start with 1. */
		big_number_copy(big_number_1(), p);

		/* Test up to 1,000,000,000. */
		big_number *p_end = big_number_new();
		big_number_copy(big_number_1(), p_end);
		big_number_multiply(p_end, big_number_1000(), p_end);
		big_number_multiply(p_end, big_number_1000(), p_end);
		big_number_multiply(p_end, big_number_1000(), p_end);

		while(big_number_compare(p, p_end) < 0) {
			int is_prime = prime_numbers_is_prime(p, &elapsed_time);

#ifdef DISPLAY_ONLY_PRIMES
			if(is_prime) {
				printf("%10d ticks: %s.\n", (int) elapsed_time, big_number_to_str(p));
			}
#else
			printf("%10d ticks: %s %s prime.\n", (int) elapsed_time, big_number_to_str(p), (is_prime) ? "is" : "is not");
#endif
			big_number_increment(p);
		}

		big_number_delete(p_end);
#endif /* TEST_ALL_INTEGERS */

		big_number_delete(p);

		/* Complete.  Pass. */
		rc = 0;
		
	} while(0);

	printf("%s(): %s.\n", __func__, (rc == 0) ? "PASS" : "FAIL");
	return rc;
}
#endif /* TEST */


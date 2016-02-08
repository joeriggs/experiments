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
#define TEST_ALL_INTEGERS

#define DISPLAY_ONLY_PRIMES

/*******************************************************************************
 * Given p, try to determine if it's prime by trying all possible factors.  This
 * is essentially a brute force test.  We'll test up to (p / 2) to see if we can
 * find a factor.
 *
 * Returns 1 if p is prime.
 * Returns 0 if p is NOT prime.
 ******************************************************************************/
static int
prime_test(big_number *p, clock_t *elapsed_time)
{
	int rc = 1;

	/* Test up to ((p / 2) + 1). */
	big_number *limit = big_number_new(0);
	big_number *i = big_number_new("2");
	big_number_divide(p, i, limit);
	big_number_increment(limit);

	big_number *result = big_number_new("0");
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
	return rc;
}

/*******************************************************************************
 *
 * Spin in a loop and look for prime numbers.  Print the ones that are prime.
 *
 ******************************************************************************/
int prime_numbers_test(void)
{
	big_number *p     = big_number_new("1");
	big_number *p_end = big_number_new("1000000000"); // 1,000,000,000

	while(big_number_compare(p, p_end) < 0) {
		clock_t elapsed_time;
		prime_test(p, &elapsed_time);
		int is_prime = prime_test(p, &elapsed_time);

#ifdef DISPLAY_ONLY_PRIMES
		if(is_prime) {
			printf("%10d ticks: %s.\n", (int) elapsed_time, big_number_to_str(p));
		}
#else
		printf("%10d ticks: %s %s prime.\n", (int) elapsed_time, big_number_to_str(p), (is_prime) ? "is" : "is not");
#endif

#ifdef TEST_ALL_INTEGERS
		big_number_increment(p);
#else
		big_number *three = big_number_new("3");
		big_number_multiply(p, three, p);
		big_number_decrement(p);
#endif
	}

	return 0;
}


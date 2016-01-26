/*******************************************************************************
 *
 * This program calculates some prime numbers.  Nothing fancy here.  It just
 * tries every possible factor to see if it can find a prime number.  So don't
 * try to discover large primes with this program.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <time.h>

/*******************************************************************************
 * Given p, try to determine if it's prime by trying all possible factors.
 *
 * Returns 0 if p is prime.
 * Returns 1 if p is NOT prime.
 ******************************************************************************/
static int
prime_test(int64_t p, clock_t *elapsed_time)
{
	int rc = 0;

	int64_t i = 2;
	clock_t start_time = clock();
	while(i < p) {
		if((p % i++) == 0) {
			rc = 1;
			break;
		}
	}
	clock_t end_time = clock();
	*elapsed_time = clock() - start_time;
	return rc;
}

/*******************************************************************************
 *
 * Spin in a loop and look for prime numbers.  Print the ones that are prime.
 *
 ******************************************************************************/
int main(int argc, char **argv)
{
	int64_t p     =       1;
	int64_t p_end = 1000000000;

	while(p < p_end) {
		clock_t elapsed_time;
		if(prime_test(++p, &elapsed_time) == 0) {
			printf("%10d ticks: %9jd\n", elapsed_time, p);
		}
	}

	return 0;
}


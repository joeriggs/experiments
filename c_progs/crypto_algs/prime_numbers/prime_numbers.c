/*******************************************************************************
 *
 * This program calculates some prime numbers.  Nothing fancy here.  It just
 * tries every possible factor to see if it can find a prime number.  So don't
 * try to discover large primes with this program.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

/*******************************************************************************
 * Given p, try to determine if it's prime by trying all possible factors.
 *
 * Returns 0 if p is prime.
 * Returns 1 if p is NOT prime.
 ******************************************************************************/
static int64_t
prime_test(int64_t p)
{
	int64_t i = 2;
	while(i < p) {
		if((p % i++) == 0) {
			return 1;
		}
	}

	return 0;
}

/*******************************************************************************
 *
 * Given 2 struct timeval data structures, calculate the elapsed time.
 *
 ******************************************************************************/
static const char *
calc_elapsed_time(struct timeval *bef, struct timeval *aft)
{
	static char res[128];

	int64_t elapsed_tvsec  = (aft->tv_sec  - bef->tv_sec);
	int64_t elapsed_tvusec = (aft->tv_usec - bef->tv_usec);

	int64_t elapsed_usec = (elapsed_tvsec * 1000000) + elapsed_tvusec;

	int64_t sec = elapsed_usec / 1000000;
	int64_t usec = elapsed_usec % 1000000;

	snprintf(res, sizeof(res) - 1, "%3jd.%06jd", sec, usec);
	return res;
}

/*******************************************************************************
 *
 * Spin in a loop and look for prime numbers.  Print the ones that are prime.
 *
 ******************************************************************************/
int main(int argc, char **argv)
{
	int64_t p = 0;
	while(1) {
		struct timeval bef;
		gettimeofday(&bef, 0);
		if(prime_test(++p) == 0) {
			struct timeval aft;
			gettimeofday(&aft, 0);

			printf("%s seconds: %9jd is prime.\n",
			        calc_elapsed_time(&bef, &aft), p);
		}
	}

	return 0;
}


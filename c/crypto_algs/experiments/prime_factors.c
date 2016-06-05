/*******************************************************************************
 *
 * This module does a brute force attempt to find the prime factors for a
 * number.  Note that it's only used against small numbers.  It's a brute force
 * test.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "prime_factors.h"

/* Styles of testing. */
#undef FIXED_DATA_TEST
#undef TIMES_TWO_TEST
#define DISPLAY_ONLY_PRIMES

typedef uint64_t num_t;

/*******************************************************************************
 *
 * Look for the prime factors for the specified number.
 *
 ******************************************************************************/
static void prime_factors(num_t num)
{
	num_t n = num;
	num_t factor = 1;
	num_t count = 0;
	char buf[2048] = { 0 }, *b = buf;
	size_t b_size = sizeof(buf);

	snprintf(b, b_size, "%20jd:", num); b_size -= strlen(b); b += strlen(b);
	clock_t start_time = clock();
	while(factor < n) {
		num_t mod;

		factor++;
		count = 0;

		while((mod = (n % factor)) == 0) {
			count++;
			n /= factor;
		}
		if(count > 0) {
			snprintf(b, b_size, " (%jd ^ %jd)", factor, count); b_size -= strlen(b); b += strlen(b);
		}
	}
	clock_t elapsed_time = clock() - start_time;

#ifdef DISPLAY_ONLY_PRIMES
	if(factor == num) {
		printf("%10d: %jd\n", elapsed_time, num);
	}
#else
	printf("%10d: %s %s\n", elapsed_time, buf, (factor == num) ? " PRIME" : "");
#endif
}

/*******************************************************************************
 *
 ******************************************************************************/
int prime_factors_test(void)
{
	printf("Testing prime factors brute force.\n");

	int rc = 0;

#ifdef FIXED_DATA_TEST
	typedef struct test_data {
		num_t num;
	} test_data;
	test_data tests[] = {
		{                   3ULL },
		{                   8ULL },
		{                  30ULL },
		{                  60ULL },
		{                 900ULL },
		{                  99ULL },
		{            20394401ULL }, // PRIME
                {    2251799813685248ULL }, // (2 ^ 51)
                {    2251799813685247ULL }, // (2 ^ 51) - 1
                {    9007199254740992ULL }, // (2 ^ 53)
                {    9007199254740991ULL }, // (2 ^ 53) - 1
                {   90071992547409912ULL }, // Big
                {  900719925474099123ULL }, // Bigger
                { 9007199254740991234ULL }, // Biggest
	};
	int num_tests = sizeof(tests) / sizeof(test_data);

	int i;
	for(i = 0; i < num_tests; i++)
	{
		num_t num = tests[i].num;
		prime_factors(num);
	}
#elif defined(TIMES_TWO_TEST)
	num_t i = 1;
	num_t j = 0;
	while(1) {
		/* Calculate the prime factors (brute force). */
		prime_factors(i);

		/* Alternate adjusting i as follows:
		 * 1. i = (i * 2).
		 * 2. i = ((i * 2) - 1).
		 */
		i = (i * 2) - j;
		j ^= 1;
	}
#else
	num_t i = 1;
	while(1) {
		/* Calculate the prime factors (brute force). */
		prime_factors(i++);
	}
#endif

	return rc;
}


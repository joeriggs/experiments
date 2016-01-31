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

typedef uint64_t num_t;

/*******************************************************************************
 *
 * Look for the prime factors for the specified number.
 *
 ******************************************************************************/
static void prime_factors(num_t num)
{
	num_t n = num;
	num_t factor = 2;

	printf("Testing %jd:", num);
	while(factor < n) {
		num_t mod = n % factor;
		if(mod == 0) {
			printf(" %jd", factor);
			n /= factor;
		}
		else {
			factor++;
		}
	}
	if(num != n) {
		printf(" %jd.\n", factor);
	}
	else {
		printf(" PRIME.\n");
	}
}

/*******************************************************************************
 *
 ******************************************************************************/
int prime_factors_test(void)
{
	printf("Testing prime factos brute force.\n");

	int rc = 0;

	typedef struct test_data {
		num_t num;
	} test_data;
	test_data tests[] = {
		{                3ULL },
		{                8ULL },
		{               30ULL },
		{               99ULL },
		{         20394401ULL }, // PRIME
                { 2251799813685248ULL }, // (2 ^ 51)
                { 2251799813685247ULL }, // (2 ^ 51) - 1
                { 9007199254740992ULL }, // (2 ^ 53)
                { 9007199254740991ULL }, // (2 ^ 53) - 1
	};
	int num_tests = sizeof(tests) / sizeof(test_data);

	int i;
	for(i = 0; i < num_tests; i++)
	{
		num_t num = tests[i].num;
		prime_factors(num);
	}

	return rc;
}


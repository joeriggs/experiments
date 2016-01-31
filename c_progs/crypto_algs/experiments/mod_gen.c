/*******************************************************************************
 *
 * This module tests a generator and modulus to see how well they will work
 * together in a Diffie Hellman exchange.  Each ((gen ^ x) % mod) must result
 * in a different value between 0 and (mod - 1).
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mod_gen.h"

typedef uint64_t mod_t;
typedef uint64_t gen_t;
typedef uint64_t exp_t;
typedef uint64_t res_t;

/*******************************************************************************
 *
 * Do a really simple exponentiation.
 *
 ******************************************************************************/
static res_t do_exponentiation(gen_t base, exp_t exp)
{
	res_t result = 1;

	//printf("%jd ^ %jd = ", base, exp);
	while(exp-- > 0)
	{
		result *= base;
	}
	//printf("%ju\n", result);

	return result;
}

/*******************************************************************************
 *
 ******************************************************************************/
int mod_gen_test(void)
{
	printf("Testing generator and modulus.\n");

	int rc = 0;

	typedef struct test_data {
		mod_t mod;
		gen_t gen;
	} test_data;
	test_data tests[] = {
		{ 19,  3 },
		{ 11,  7 },
		{ 23, 13 }
	};
	int num_tests = sizeof(tests) / sizeof(test_data);

	int i;
	for(i = 0; i < num_tests; i++)
	{
		mod_t mod = tests[i].mod;
		gen_t gen = tests[i].gen;
		printf("Testing Mod %jd and gen %jd.\n", mod, gen);

		int list_size = (sizeof(gen) * mod);
		res_t *list = (res_t *) malloc(list_size);
		if(list == (res_t *) 0)
		{
			printf("Unable to allocate buffer.\n");
			rc = 1;
		}
		memset(list, 0, list_size);

		if(rc == 0)
		{
			exp_t i;
			for(i = 0; (rc == 0) && (i < mod); i++)
			{
				res_t res = do_exponentiation(gen, i) % mod;
				//printf("%jd ^ %jd mod %jd = %jd\n", gen, i, mod, res);
				if(res >= mod)
				{
					printf("Result looks weird.\n");
					rc = 2;
				}
				else if(list[res] != 0)
				{
					printf("Multiple exponents got the same result (%jd vs %jd).\n", list[res], i);
					rc = 3;
				}
				else
				{
					list[res] = i;
				}
			}
		}

		if(rc == 0)
		{
			int i;
			for(i = 0; i < mod; i++)
			{
				int x;
				for(x = 0; x < mod; x++)
				{
					if(list[x] == i)
					{
						break;
					}
				}

				printf("%3d = (%3jd ^ %3jd) mod %3jd :: %3d = (%3jd ^ %3jd) mod %3jd\n",
				       i, gen, list[i], mod,
				       x, gen, list[x], mod);
			}
		}
	}

	return rc;
}


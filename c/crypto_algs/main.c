
#include <stdio.h>

#include "big_number.h"
#include "big_number_base.h"
#include "diffie_hellman.h"
#include "prime_numbers.h"
#include "rsa.h"

#ifdef TEST
static int test(void)
{
	int rc = 0;

	int loop_count;
	for(loop_count = 0; (loop_count < 1000000) && (rc == 0); loop_count++) {
		printf("Loop #%d:\n", loop_count);

		rc = big_number_base_test();

		if(rc == 0) {
			rc = big_number_test();
		}

		if(rc == 0) {
			rc = diffie_hellman_test();
		}

		if(rc == 0) {
			rc = prime_numbers_test();
		}

		if(rc == 0) {
			rc = rsa_test();
		}
	}

	return rc;
}
#else
#define test() 0
#endif

int main(int argc, char **argv)
{
	int rc = 0;

	rc = test();

	return rc;
}


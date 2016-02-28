
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

	printf("Starting Big Number Base test.\n");
	rc = big_number_base_test();

	if(rc == 0) {
		printf("Starting Big Number test.\n");
		rc = big_number_test();
	}

	if(rc == 0) {
		printf("Starting Diffie Hellman test.\n");
		rc = diffie_hellman_test();
	}

	if(rc == 0) {
		printf("Starting Prime Numbers test.\n");
		rc = prime_numbers_test();
	}

	if(rc == 0) {
		printf("Starting RSA test.\n");
		rc = rsa_test();
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


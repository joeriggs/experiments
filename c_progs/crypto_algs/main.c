
#include <stdio.h>

#include "diffie_hellman.h"
#include "prime_numbers.h"
#include "rsa.h"

int main(int argc, char **argv)
{
	diffie_hellman_test();
	prime_numbers_test();
	rsa_test();

	return 0;
}


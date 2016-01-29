
#include <stdio.h>

#include "diffie_hellman.h"
#include "mod_gen.h"
#include "prime_numbers.h"
#include "rsa.h"

int main(int argc, char **argv)
{
	diffie_hellman_test();
	mod_gen_test();
	prime_numbers_test();
	rsa_test();
}


/* Experimental program that does fast integer exponentiation.  The idea is to
 * avoid looping exp times to do a large exponentiation. */

#include <inttypes.h>
#include <stdio.h>

/* The lower and upper ranges for the base and exponent. */
#define BASE_MIN 3
#define BASE_MAX 7
#define EXP_MIN 3
#define EXP_MAX 7

static uint64_t
do_exp(uint64_t base,
       uint64_t exp)
{
  /* Special case.  0^exp = 0. */
  if(base == 0ll) return 0;

  /* Special case.  base^0 = 1. */
  if(exp == 0ll) return 1;

  uint64_t result = 1;
  while(exp != 0)
  {
    if((exp & 1) != 0)
    {
      result *= base;
    }

    /* Prepare for next round. */
    base *= base;
    exp >>= 1;
  }

  return result;
}

int main(int argc, char **argv)
{
  printf("Fast exponentiation test\n");

  uint64_t base;
  for(base = BASE_MIN; base <= BASE_MAX; base++)
  {
    uint64_t exp;
    for(exp = EXP_MIN; exp <= EXP_MAX; exp++)
    {
      uint64_t result = do_exp(base, exp);
      printf("%lld ^ %lld = %lld.\n", base, exp, result);
    }
  }

  return 0;
}


/*******************************************************************************
 *
 * This program tests a generator and modulus to see how well they will work
 * together in a Diffie Hellman exchange.  Each ((gen ^ x) % mod) must result
 * in a different value between 0 and (mod - 1).
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*******************************************************************************
 *
 * Do a really simple exponentiation.
 *
 ******************************************************************************/
static int64_t do_exponentiation(int64_t base, int64_t exp)
{
  int64_t result = 1;

  while(exp-- > 0) {
    result *= base;
  }

  return result;
}

/*******************************************************************************
 *
 ******************************************************************************/
int main(int argc, char **argv)
{
  int rc = 0;

  int64_t mod = 19;
  int64_t gen =  3;

  printf("Testing generator and modulus.\n");

  int list_size = (sizeof(gen) * mod);
  int64_t *list = (int64_t *) malloc(list_size);
  if(list == (int64_t *) 0) {
    printf("Unable to allocate buffer.\n");
    rc = 1;
  }
  memset(list, 0, list_size);

  if(rc == 0) {
    int64_t i;
    for(i = 0; (rc == 0) && (i < mod); i++) {
      int64_t res = do_exponentiation(gen, i) % mod;
      if(res >= mod) {
        printf("Result looks weird.\n");
        rc = 2;
      }
      else if(list[res] != 0) {
        printf("Multiple exponents got the same result.\n");
        rc = 3;
      }
      else {
        list[res] = i;
      }
    }
  }

  if(rc == 0) {
    int i;
    for(i = 0; i < mod; i++) {
      int x;
      for(x = 0; x < mod; x++) {
        if(list[x] == i) {
          break;
        }
      }

      printf("%3jd = (%3jd ^ %3d) mod %3jd :: %3d = (%3jd ^ %3d) mod %3jd\n",
             list[i], gen, i, mod,
             i, gen, x, mod);
    }
  }

  return rc;
}


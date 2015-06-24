/* Quick test of the correct C syntax for initializing a data structure at
 * build-time.
 */
#include <time.h>
#include <stdio.h>

typedef struct {
  int a;
  int b;
  int c;
} test_struct;

int
main(int argc, char **argv)
{
  test_struct testing = {
    .a = 1,
    .b = 2,
    .c = 3
  };

  printf("a = %d. b = %d. c = %d.\n", testing.a, testing.b, testing.c);

  return 0;
}


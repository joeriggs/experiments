/******************************************************************************
 * This program will test malloc() and realloc() to see how realloc() grows the
 * originally-allocated memory.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  printf("Running realloc() test...\n");

  /* Start with a regular malloc(). */
  size_t alloc_size = 1;
  unsigned char *buf = (unsigned char *) malloc(alloc_size);
  printf("malloc(%d) = %p.\n", alloc_size, buf);

  /* Loop around and see how well realloc() works. */
  while(alloc_size && buf) {
    /* Reallocate a new buffer. */
    size_t new_alloc_size = alloc_size << 1;
    unsigned char *buf_new = realloc(buf, new_alloc_size);
    if(buf_new) {
      printf("realloc(%p, %d) = %p.\n", buf, new_alloc_size, buf_new);
      if(buf_new != buf) {
        buf = buf_new;
        int cmp_result = memcmp(buf, buf_new, alloc_size);
        printf("memcmp(%p, %p, %d) returned %d.\n", buf, buf_new, alloc_size, cmp_result);

        int i;
        for(i = 0; i < new_alloc_size; i++) {
          buf_new[i] = i;
        }
      }
    }
    alloc_size = new_alloc_size;
  } while(alloc_size);

  printf("End of test program.\n");
  return 0;
}


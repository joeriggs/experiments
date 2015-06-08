
/******************************************************************************
 * This program will allow you to play around with fork()/exec().
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/time.h>

#define MAX_MALLOCS 32
static unsigned char *ptrs[MAX_MALLOCS];
static unsigned long long total_malloc_bytes = 0ll;
static unsigned long long total_malloc_calls = 0ll;

int main(int argc, char **argv)
{
  struct timeval start_time;
  struct timeval cur_time;
  int elapsed_time;
  gettimeofday(&start_time, 0);

  pid_t pid = getpid();
  int run_time = ((pid % 10) + 1);
  printf("%s(): This is child %d.  It's going to run for %d seconds.\n", __func__, pid, run_time);

  do
  {
    int i;
    for(i = 0; i < MAX_MALLOCS; i++)
    {
      int malloc_size = pid + (random() % 65536);
      total_malloc_bytes += malloc_size;
      total_malloc_calls++;

      ptrs[i] = malloc(malloc_size);
      if(ptrs[i] == 0)
      {
        printf("ERROR: malloc(%d) failed.\n", malloc_size);
      }
    }

    for(i = 0; i < MAX_MALLOCS; i++)
    {
      free(ptrs[i]);
    }

    gettimeofday(&cur_time, 0);
    elapsed_time = cur_time.tv_sec - start_time.tv_sec;
  } while(elapsed_time < run_time);

  printf("Child %d ran for %d seconds.  Allocated %lld bytes in %lld malloc() calls.\n",
           pid, elapsed_time, total_malloc_bytes, total_malloc_calls);
  return 0;
}


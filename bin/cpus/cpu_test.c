/******************************************************************************
 * Get some CPU info about the current process.
 *****************************************************************************/
#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  printf("Running CPU test...\n");

  int cpu = sched_getcpu();
  printf("CPU is %d.\n", cpu);

  cpu_set_t mask;
  int retcode = sched_getaffinity(0, sizeof(mask), &mask);
  printf("sched_getaffinity() returned %d.\n", retcode);

  printf("There are %d CPUs.\n", CPU_COUNT(&mask));

  printf("End of CPU test.\n");
  return 0;
}


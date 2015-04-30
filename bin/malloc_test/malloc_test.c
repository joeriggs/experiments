/******************************************************************************
 * This program will create several threads and distribute them to different
 * CPUs.  Then it will have each thread spin in a loop and call malloc()/free()
 * until they're told to stop.
 *
 * You can build with a command like:
 *     gcc malloc_test.c -o malloc_test -lpthread
 *****************************************************************************/

#define _GNU_SOURCE
#define __USE_GNU
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

/* These are the thread IDs returned from pthread_create(). */
static pthread_t thread_objs[CPU_SETSIZE] = { 0 };

/* These are return codes that a thread returns to main(). */
static int cpu_retcodes[CPU_SETSIZE] = { 0 };

/* These are the return codes that main() reads from pthread_join(). */
static int retcodes[CPU_SETSIZE] = { 0 };

/* The number of times each CPU called malloc(). */
static unsigned int malloc_counters[CPU_SETSIZE] = { 0 };

/* This is set to 1 when main() wants the tests to stop on each CPU. */
static unsigned int stop_flag = 0;

/******************************************************************************
 * This function runs an iteration of the malloc/free test.
 *
 * Input:  cpu is the CPU the thread is supposed to run on.
 *
 * Output: Returns 0 if the test passed.
 *         Returns 1 if the test failed.
 *****************************************************************************/
static int perform_test(int cpu)
{
  int retcode = 0;
  while(1) {
    int i;
    for(i = 1; i < 65536; i++) {
      unsigned char *p = malloc(i);
      if(p == (unsigned char *) 0) {
        printf("%s(%d): malloc() failed\n", __func__, cpu);
        retcode = 1;
        break;
      }
      else {
        malloc_counters[cpu]++;
        free(p);
      }
    }

    if(stop_flag == 1) {
      break;
    }
  }

  return retcode;
}

/******************************************************************************
 * This is the thread function.  It runs an iteration of the test.
 * 1. It pins the thread to the specified CPU.
 * 2. It runs the test.
 * 3. It exits the thread.
 *
 * Input:  arg is the CPU the thread is supposed to run on.
 *
 * Output: Exits the thread with a retcode of 0 if the test passed.
 *         Exits the thread with a retcode of 1 if the test failed.
 *****************************************************************************/
static void *thread_func(void *arg)
{
  int cpu = (int) arg;
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(cpu, &set);
  pthread_setaffinity_np(pthread_self(), sizeof(set), &set);

  /* Run the test until main() tells us to stop. */
  printf("CPU %2d: Thread is pinned to CPU.\n", cpu);

  cpu_retcodes[cpu] = perform_test(cpu);

  printf("CPU %2d: Thread is returning %d.\n", cpu, cpu_retcodes[cpu]);
  pthread_exit((void *) cpu_retcodes[cpu]);
}


int main(int argc, char **argv)
{
  int rc;
  printf("Running malloc() test...\n");

  cpu_set_t mask;
  sched_getaffinity(0, sizeof(mask), &mask);

  /* Loop here and create all of the threads. */
  int cpu;
  for(cpu = 0; cpu < CPU_SETSIZE; cpu++) {
    if(CPU_ISSET(cpu, &mask)) {

      /* Initialize the thread attributes object to default settings. */
      pthread_attr_t thread_attr;
      if( (rc = pthread_attr_init(&thread_attr)) != 0 ) {
        printf("CPU %2d: pthread_attr_init() failed: rc = %d (%s).\n", cpu, rc, strerror(rc));
      }

      /* Create the thread. */
      if( (rc = pthread_create(&thread_objs[cpu], &thread_attr, thread_func, (void *)cpu)) != 0 ) {
        printf("CPU %2d: pthread_create() failed: rc = %d (%s).\n", cpu, rc, strerror(rc));
      }
    }
  }

  /* Let the test run for 30 seconds, and then tell the threads to stop. */
  sleep(30);
  stop_flag = 1;

  /* Wait for all of the threads to terminate. */
  for(cpu = 0; cpu < CPU_SETSIZE; cpu++) {
    if(CPU_ISSET(cpu, &mask)) {

      /* Join the thread.  This allows us to block until the thread terminates. */
      if( (rc = pthread_join(thread_objs[cpu], (void **) &retcodes[cpu])) != 0 ) {
        printf("CPU %2d: pthread_join() failed: rc = %d (%s).\n", cpu, rc, strerror(rc));
      }
      else {
        printf("CPU %2d: pthread_join() got retcode = %d.\n", cpu, retcodes[cpu]);
      }
    }
  }

  /* Cycle through the counters. */
  for(cpu = 0; cpu < CPU_SETSIZE; cpu++) {
    if(CPU_ISSET(cpu, &mask)) {
      printf("CPU %2d did %d malloc() calls\n", cpu, malloc_counters[cpu]);
    }
  }

  printf("End of test program.\n");
  return 0;
}


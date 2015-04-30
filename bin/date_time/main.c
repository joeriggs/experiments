/******************************************************************************
 * Experiment with the gettimeofday() function.
 *****************************************************************************/
#include <sys/time.h>
#include <time.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
  time_t cur_time = time(&cur_time);
  char *cur_time_str = ctime(&cur_time);
  printf("%s\n", cur_time_str);

  int i;
  for(i = 0; i < 5; i++) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    printf("sizeof(tv.tv_sec)  = %d.\n", sizeof(tv.tv_sec));
    printf("sizeof(tv.tv_usec) = %d.\n", sizeof(tv.tv_usec));

    printf("%d.%06d\n", tv.tv_sec, tv.tv_usec);

    sleep(1);
  }

  return 0;
}


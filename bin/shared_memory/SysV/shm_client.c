
#include "shm_test.h"

int main(int argc, char **argv)
{
  printf("SHM Client\n");

  int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
  printf("shmget() returned %d.\n", shmid);
  if(shmid != -1)
  {
    unsigned char *m = shmat(shmid, 0, 0);
    printf("shmat() returned %p.\n", m);

    /* Spin real slowly, watching the memory space. */
    unsigned char c = 0;
    while(c != 0xFF)
    {
      if(*m != c)
      {
        printf("New value (%x != %x).\n", *m, c);
        c = *m;
      }

      sleep(1);
    }
  }

  return 0;
}



#include "shm_test.h"

int main(int argc, char **argv)
{
  printf("SHM Server\n");

  int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
  printf("shmget() returned %d.\n", shmid);
  if(shmid != -1)
  {
    unsigned char *m = shmat(shmid, 0, 0);
    printf("shmat() returned %p.\n", m);

    /* Spin real slowly, updating the memory space. */
    int i;
    for(i = 0; i < 0x100; i++)
    {
      printf("%x.\n", i);

      *m = i;
      sleep(1);
    }
  }

  return 0;
}


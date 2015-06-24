
#include "shm_test.h"

int main(int argc, char **argv)
{
  int retval;

  printf("SHM Client\n");

  int fd = shm_open(SHM_PATH, O_RDWR, 0666);
  printf("shm_open() returned %d (%s).\n", fd,
            (fd >= 0) ? "PASS": "FAIL");
  if(fd >= 0)
  {
    unsigned char *m = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(m != MAP_FAILED)
    {
      printf("mmap() returned %p (%s).\n", m, "PASS");

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

      retval = munmap(m, SHM_SIZE);
      printf("unmap() returned %d (%s).\n", retval, (retval == 0) ? "PASS" : "FAIL");
    }
    else
    {
      char *err = strerror(errno);
      printf("mmap() returned %p (%s).\n", m, err);
    }

    retval = shm_unlink(SHM_PATH);
    printf("shm_unlink() returned %d (%s).\n", retval,
            (retval == 0) ? "PASS" : "FAIL");
  }

  return 0;
}


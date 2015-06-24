
#include "socket_test.h"

int main(int argc, char **argv)
{
  int retval;

  printf("Socket Server\n");

  int fd = shm_open(SHM_PATH, O_RDWR | O_CREAT, 0666);
  printf("shm_open() returned %d (%s).\n", fd,
            (fd >= 0) ? "PASS": "FAIL");
  if(fd >= 0)
  {
    ftruncate(fd, SHM_SIZE);

    struct stat s;
    retval = fstat(fd, &s);
    printf("fstat() returned %d (mode %o: uid %d: gid %d: size %ld).\n", retval,
           s.st_mode, s.st_uid, s.st_gid, s.st_size);

    unsigned char *m = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(m != MAP_FAILED)
    {
      printf("mmap() returned %p (%s).\n", m, "PASS");

      memset(m, 0, SHM_SIZE);

      /* Spin real slowly, updating the memory space. */
      int i;
      for(i = 0; i < 0x100; i++)
      {
        printf("%x.\n", i);

        *m = i;
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


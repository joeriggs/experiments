
/*******************************************************************************
 * This program will create a shared memory space that is shared between a
 * parent and child process.
 ******************************************************************************/

#include "shm_test.h"

int main(int argc, char **argv)
{
	int retval;

	printf("SHM Parent/Child example.\n");

	int fd = shm_open(SHM_PATH, O_RDWR | O_CREAT, 0666);
	printf("shm_open() returned %d (%s).\n", fd,
	          (fd >= 0) ? "PASS": "FAIL");
	if(fd >= 0)
	{
		ftruncate(fd, SHM_FORK_SIZE);

		struct stat s;
		retval = fstat(fd, &s);
		printf("fstat() returned %d (mode %o: uid %d: gid %d: size %ld).\n", retval,
		       s.st_mode, s.st_uid, s.st_gid, s.st_size);

		errno = 0;
		unsigned char *m = mmap(0, SHM_FORK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if(m != MAP_FAILED)
		{
			printf("mmap() returned %p (%s).\n", m, strerror(errno));

			memset(m, 0, SHM_FORK_SIZE);

			int pid = fork();
			switch(pid) {
			case 0:
			{
				int childPID = getpid();

				/* This is the child.  Spin real slowly,
				 * watching the shared memory space. */
				unsigned char c = 0;
				while(c != 0xFF)
				{
					if(*m != c)
					{
						printf("Child (%d): new value (%x %x %x).\n",
						        childPID, m[0], m[1000], m[1000000]);
						c = m[0];
					}
	
					sleep(1);
				}
				break;
			}

			case -1: // An error occurred.
				printf("fork() failed.  Returned %d (%s).\n",
				        errno, strerror(errno));
				break;

			default:
			{
				/* This is the parent.  Spin real slowly
				 * and update the memory space. */
				int i;
				for(i = 0; i < 0x100; i++)
				{
					printf("Parent writing %x.\n", i);

					m[      0] = i;
					m[   1000] = i;
					m[1000000] = i;
					sleep(1);
				}
				break;
			}
			}

			retval = munmap(m, SHM_FORK_SIZE);
			printf("unmap() returned %d (%s).\n", retval,
			        (retval == 0) ? "PASS" : "FAIL");
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


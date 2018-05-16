
/*******************************************************************************
 * This program will create a shared memory space that is shared between a
 * parent and child process.
 ******************************************************************************/

#include <semaphore.h>

#include "shm_test.h"

int main(int argc, char **argv)
{
	int retval;

	printf("SHM Parent/Child example.\n");

	int fd = shm_open(SHM_PATH, O_RDWR | O_CREAT, 0666);
	if(fd < 0) {
		printf("shm_open() failed (%s).\n", strerror(errno));
		return 1;
	}
	printf("shm_open() returned %d.\n", fd);

	if(ftruncate(fd, SHM_FORK_SIZE) == -1) {
		printf("ftruncate() failed (%s).\n", strerror(errno));
		return 1;
	}
	printf("ftruncate() succeeded.\n");

	struct stat s;
	if(fstat(fd, &s) == -1) {
		printf("fstat() failed (%s).\n", strerror(errno));
		return 1;
	}
	printf("fstat() returned (mode %o: uid %d: gid %d: size %ld).\n",
	         s.st_mode, s.st_uid, s.st_gid, s.st_size);

	unsigned char *m = mmap(0, SHM_FORK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(m == MAP_FAILED) {
		printf("mmap() failed (%s).\n", strerror(errno));
		return 1;
	}
	printf("mmap() returned %p\n", m);

	memset(m, 0, SHM_FORK_SIZE);

	/* Create a semaphore at offset[0] in the shared mem space. */
	sem_t *sem = (sem_t *) m;
	if(sem_init(sem, 1, 0) == -1) {
		printf("sem_init() failed (%s).\n", strerror(errno));
		return 1;
	}
	printf("sem_init() succeeded.\n");

	int pid = fork();
	switch(pid) {
	case 0:
	{
		int myPID = getpid();
		int parentPID = getppid();

		/* This is the child.  Spin & and update the memory space. */
		int i;
		for(i = 0; i < 0x100; i++)
		{
			printf("Child (PID %d) writing %x to parent PID %d.\n", myPID, i, parentPID);

			m[    100] = i;
			m[   1000] = i + 1;
			m[1000000] = i + 2;
			sem_post(sem);
			sleep(1);
		}
	}
	break;

	case -1: // An error occurred.
		printf("fork() failed.  Returned %d (%s).\n", errno, strerror(errno));
		break;

	default:
	{
		int myPID = getpid();
		int childPID = pid;

		/* This is the parent.  Spin real slowly,
		 * watching the shared memory space. */
		while(m[100] != 0xFF)
		{
			int sem_wait_rc = sem_wait(sem);
			printf("Parent (%d): sem_wait(%p) returned %d.\n", myPID, sem, sem_wait_rc);

			printf("Parent (%d): new values (%x %x %x) from child %d.\n",
			        myPID, m[100], m[1000], m[1000000], childPID);
		}
	}
	break;

	}

	retval = munmap(m, SHM_FORK_SIZE);
	printf("unmap() returned %d (%s).\n", retval, (retval == 0) ? "PASS" : "FAIL");

	retval = shm_unlink(SHM_PATH);
	printf("shm_unlink() returned %d (%s).\n", retval, (retval == 0) ? "PASS" : "FAIL");

	return 0;
}


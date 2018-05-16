
/*******************************************************************************
 * This program will create a shared memory space that is shared between a
 * parent and several child process.  It then uses a rwlock to control access
 * to the shared memory space the parent and children.  The parent has write
 * access to the shared memory, and the children can only read.
 ******************************************************************************/

#include <pthread.h>

#include "shm_test.h"

/* This will eventually point to a shared memory space that is set up between
 * all processes (parent and children). */
static unsigned char *m = NULL;

/* This will eventually point to a read-write lock that is stored in the shared
 * memory space.  It is used to control access to the shared memory space. */
static pthread_rwlock_t *rwlock = NULL;

static void doFork(void)
{
	int childPID = fork();
	switch(childPID) {
	case 0:
	{
		int myPID = getpid();
		int loopCount = 0;

		/* This is a child.  Spin and read the memory space. */
		while(m[100] != 0xFF)
		{
			/* Grab a read lock.  If the parent has it locked, we'll
			 * block until it releases. */
			printf("%4d: Child (PID %d): Waiting for rdlock.\n", ++loopCount, myPID);
			pthread_rwlock_rdlock(rwlock);
			printf("%4d: Child (PID %d): Got the rdlock.\n", loopCount, myPID);

			printf("%4d: Child (PID %d): Current values (%02X %02X %02X).\n", loopCount,
			        myPID, m[100], m[1000], m[1000000]);

			printf("%4d: Child (PID %d): Releasing the rdlock.\n", loopCount, myPID);
			pthread_rwlock_unlock(rwlock);
			printf("%4d: Child (PID %d): Released the rdlock.\n", loopCount, myPID);
			usleep(100000);
		}
	}
		break;

	case -1: // An error occurred.
		printf("fork() failed.  Returned %d (%s).\n", errno, strerror(errno));
		break;

	default:
		printf("Parent PID %d created child PID %d.\n", getpid(), childPID);
		break;

	}
}

int main(int argc, char **argv)
{
	int retval;

	printf("SHM Parent/Children rwlock example.\n");

	int myPID = getpid();

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

	if((m = mmap(0, SHM_FORK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		printf("mmap() failed (%s).\n", strerror(errno));
		return 1;
	}
	printf("mmap() returned %p\n", m);

	memset(m, 0, SHM_FORK_SIZE);

	/* Create a rwlock at offset[0] in the shared mem space. */
	rwlock = (pthread_rwlock_t *) m;
	pthread_rwlockattr_t attr;
	pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	if(pthread_rwlock_init(rwlock, &attr) == -1) {
		printf("pthread_rwlock_init() failed (%s).\n", strerror(errno));
		return 1;
	}
	printf("pthread_rwlock_init() succeeded.\n");

	/* Create a child.  Feel free to call this function more than once in
	 * order to create multiple children. */
	doFork();
	doFork();
	doFork();

	/* The parent returns to this location. Spin and occasionally update the
	 * memory space.  We do the updates VERY SLOWLY, so that the user can
	 * observe the child processes being blocked during the updates.  We
	 * hold the lock for 1 second, and then we release it for 1 second.
	 */
	int i;
	for(i = 0; i < 0x100; i++) {

		printf("Parent (PID %d): Waiting for wrlock.\n", myPID);
		pthread_rwlock_wrlock(rwlock);
		printf("Parent (PID %d): Got the wrlock.\n", myPID);

		printf("Parent (PID %d): Writing %02X to shared memory.\n", myPID, i);
		m[    100] = i;
		m[   1000] = i + 1;
		m[1000000] = i + 2;
		sleep(1);

		printf("Parent (PID %d): Releasing for wrlock.\n", myPID);
		pthread_rwlock_unlock(rwlock);
		printf("Parent (PID %d): Released wrlock.\n", myPID);
		sleep(1);
	}

	retval = munmap(m, SHM_FORK_SIZE);
	printf("unmap() returned %d (%s).\n", retval, (retval == 0) ? "PASS" : "FAIL");

	retval = shm_unlink(SHM_PATH);
	printf("shm_unlink() returned %d (%s).\n", retval, (retval == 0) ? "PASS" : "FAIL");

	return 0;
}


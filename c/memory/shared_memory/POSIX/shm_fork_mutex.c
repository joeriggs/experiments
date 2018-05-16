
/*******************************************************************************
 * This program will create a shared memory space that is shared between a
 * parent and several child process.  It then uses a mutex to control access
 * to the shared memory space.
 ******************************************************************************/

#include <pthread.h>
#include <stdlib.h>

#include "shm_test.h"

/* This will eventually point to a shared memory space that is set up between
 * all processes (parent and children). */
static unsigned char *m = NULL;

/* This will eventually point to a mutex that is stored in the shared memory
 * space.  It is used to control access to the shared memory space. */
static pthread_mutex_t *mutex = NULL;

static void doFork(void)
{
	int childPID = fork();
	switch(childPID) {
	case 0:
	{
		int myPID = getpid();
		int loopCount = 0;

		/* This is a child.  Spin and read the memory space. */
		while(m[100] < 0xFF)
		{
			/* Grab the mutex. */
			printf("%4d: Child (PID %d): Waiting for mutex.\n", ++loopCount, myPID);
			pthread_mutex_lock(mutex);
			printf("%4d: Child (PID %d): Got the mutex.\n", loopCount, myPID);

			printf("%4d: Child (PID %d): Current values (%02X %02X %02X).\n", loopCount,
			        myPID, m[100], m[1000], m[1000000]);
			usleep(100000);

			printf("%4d: Child (PID %d): Releasing the mutex.\n", loopCount, myPID);
			pthread_mutex_unlock(mutex);
			printf("%4d: Child (PID %d): Released the mutex.\n", loopCount, myPID);

			sched_yield();
		}

		printf("Child (PID %d): Exiting.\n", myPID);
		exit(0);
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

	printf("SHM Parent/Children mutex example.\n");

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

	/* Create a mutex at offset[0] in the shared mem space. */
	mutex = (pthread_mutex_t *) m;
	pthread_mutexattr_t attr;
	pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	if(pthread_mutex_init(mutex, &attr) == -1) {
		printf("pthread_mutex_init() failed (%s).\n", strerror(errno));
		return 1;
	}
	printf("pthread_mutex_init() succeeded.\n");

	/* Create a child.  Feel free to call this function more than once in
	 * order to create multiple children. */
	doFork();

	/* The parent returns to this location. Spin and occasionally update the
	 * memory space.  We do the updates slowly, so that the user can
	 * observe the child processes being blocked during the updates.  We
	 * hold the lock for 100ms, and then we release it for 100ms.
	 */
	int i;
	for(i = 0; i < 0x102; i++) {

		printf("Parent (PID %d): Waiting for mutex.\n", myPID);
		pthread_mutex_lock(mutex);
		printf("Parent (PID %d): Got the mutex.\n", myPID);

		printf("Parent (PID %d): Writing %02X to shared memory.\n", myPID, i);
		m[    100] = i;
		m[   1000] = i + 1;
		m[1000000] = i + 2;
		usleep(100000);

		printf("Parent (PID %d): Releasing for mutex.\n", myPID);
		pthread_mutex_unlock(mutex);
		printf("Parent (PID %d): Released mutex.\n", myPID);

		sched_yield();
	}

	retval = pthread_mutex_destroy(mutex);
	printf("pthread_mutex_destroy() returned %d (%s).\n", retval, (retval == 0) ? "PASS" : "FAIL");

	retval = munmap(m, SHM_FORK_SIZE);
	printf("unmap() returned %d (%s).\n", retval, (retval == 0) ? "PASS" : "FAIL");

	retval = shm_unlink(SHM_PATH);
	printf("shm_unlink() returned %d (%s).\n", retval, (retval == 0) ? "PASS" : "FAIL");

	return 0;
}


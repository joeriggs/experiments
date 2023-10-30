
/******************************************************************************
 * This program will allow you to play around with mutexes.
 *****************************************************************************/

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

static pthread_mutex_t my_mutex;

static void *my_thread(void *arg)
{
	int rc;

	printf("%s(): This is the thread function.\n", __func__);
	sleep(1);

	errno = 0;
	printf("%s(): Calling pthread_mutex_unlock().\n", __func__);
	rc = pthread_mutex_unlock(&my_mutex);
	printf("%s(): pthread_mutex_unlock() returned %d (%m).\n", __func__, rc);

	printf("%s(): Calling pthread_mutex_lock().\n", __func__);
	rc = pthread_mutex_lock(&my_mutex);
	printf("%s(): pthread_mutex_lock() returned %d (%m).\n", __func__, rc);

	return (void *) 1357;
}

int main(int argc, char **argv)
{
	int rc;
	pthread_t thread;

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	printf("%s(): Testing pthreads.\n", __func__);

	/* Run a test and try a mutex lock. */

	pthread_mutex_init(&my_mutex, NULL);

	pthread_mutex_lock(&my_mutex);
	rc = pthread_create(&thread, &attr, my_thread, NULL);
	printf("%s(): pthread_create() returned %d.\n", __func__, rc);
	if(rc == 0) {
		sleep(10);
		pthread_mutex_unlock(&my_mutex);
		printf("%s(): Unlocked mutex.\n", __func__);
//		pthread_mutex_destroy(&my_mutex);
//		printf("%s(): Destroyed mutex.\n", __func__);

		void *thread_rc;
		rc = pthread_join(thread, &thread_rc);
		printf("%s(): pthread_join() returned %d.\n", __func__, rc);
		printf("%s(): pthread terminated.  thread_rc = %p.\n", __func__, thread_rc);
	}
	else {
		printf("%s(): Failed to create thread (%d).\n", __func__, rc);
	}

	return 0;
}


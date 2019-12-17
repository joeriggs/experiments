
/******************************************************************************
 * This program will allow you to play around with conditional variables.
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

static pthread_cond_t  cond = PTHREAD_COND_INITIALIZER; 
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


/* This is the function that is executed during the 1st pthread test. */
static void *my_thread1(void *arg)
{
	printf("%s(): This is the thread function.\n", __func__);

	int *variable = (int *) arg;

	printf("%s(): Calling pthread_mutex_lock()\n", __func__);
	pthread_mutex_lock(&lock);
	printf("%s(): Got the mutex.\n", __func__);
	*variable = 0x12345678;
	pthread_mutex_unlock(&lock);

	printf("%s(): Signaling ... \n", __func__);
	pthread_cond_signal(&cond);

	printf("%s(): Sleeping again ...\n", __func__);
	sleep(15);

	printf("%s(): Terminating.\n", __func__);
	return (void *) 1234;
}


int main(int argc, char **argv)
{
	printf("%s(): Testing conditional variables.\n", __func__);

	/* This is where the pthread will store the result. */
	int result;

	int variable;

	printf("%s(): Calling pthread_mutex_lock().\n", __func__);
	pthread_mutex_lock(&lock);

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	pthread_t thread;
	int rc = pthread_create(&thread, &attr, my_thread1, &variable);
	printf("%s(): pthread_create() returned %d.\n", __func__, rc);
	if(rc != 0) {
		printf("%s(): Failed to create thread (%d).\n", __func__, rc);
	}

	else {
		printf("%s(): Sleeping ... \n", __func__);
		sleep(5);

		printf("%s(): Calling pthread_cond_wait().\n", __func__);
		pthread_cond_wait(&cond, &lock);
		printf("%s(): After pthread_cond_wait().\n", __func__);

		printf("%s(): variable = %X\n", __func__, variable);

		printf("%s(): Sleeping ...\n", __func__);
		sleep(20);
	}

	printf("%s(): Terminating.\n", __func__);
	return 0;
}


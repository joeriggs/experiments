/*******************************************************************************
 *
 * This module negotiates a Diffie Hellman exchange between 2 threads.
 *
 ******************************************************************************/

#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/un.h>

#include "diffie_hellman.h"

#define CHANNEL "./unix_socket"

/* Used to synchronize activities between the client and server threads. */
pthread_mutex_t mutex;

/*******************************************************************************
 *
 * Do a really simple exponentiation.
 *
 ******************************************************************************/
static int64_t do_exponentiation(int64_t base, int64_t exp)
{
	int64_t result = 1;

	while(exp-- > 0) {
		result *= base;
	}

	return result;
}

/*******************************************************************************
 *
 * This is the Diffie Hellman server thread.
 *
 ******************************************************************************/
static void *server_thread(void *arg)
{
	int retval = -1;
	int sock = -1;
	int clnt_sock = -1;

	//printf("%s(%p): This is the server thread.\n", __func__, arg);

	do
	{
		int ret;

		unlink(CHANNEL);

		sock = socket(PF_UNIX, SOCK_STREAM, 0);
		//printf("%s(): socket() returned %d.\n", __func__, sock);
		if(sock < 0) break;

		struct sockaddr_un address;
		memset(&address, 0, sizeof(address));

		address.sun_family = PF_UNIX;
		strncpy(address.sun_path, CHANNEL, sizeof(address.sun_path));
		ret = bind(sock, (struct sockaddr *) &address, sizeof(address));
		//printf("Called bind(): ret = %d.\n", ret);
		if(ret != 0) break;

		ret = listen(sock, 10);
		//printf("Called listen().\n");
		if(ret != 0) break;

		ret = pthread_mutex_unlock(&mutex);
		//printf("%s(): pthread_mutex_unlock(%p) returned %d.\n", __func__, &mutex, ret);
		if(ret != 0) { break; }

		struct sockaddr clnt_addr;
		socklen_t       clnt_addr_len = sizeof(clnt_addr);
		clnt_sock = accept(sock, &clnt_addr, &clnt_addr_len);
		//printf("%s(): accept() returned %d.\n", __func__, clnt_sock);
		if(clnt_sock == -1) break;

		int64_t modulus;
		if(read(clnt_sock, &modulus, sizeof(modulus)) != sizeof(modulus)) {
			//printf("Can't read modulus..\n");
		}

		int64_t generator;
		if(read(clnt_sock, &generator, sizeof(generator)) != sizeof(generator)) {
			//printf("Can't read generator..\n");
		}

		/* Create your own private prime number. */
		int64_t pri = 5;

		/* Perform step 1 and send to client. */
		int64_t val1 = do_exponentiation(generator, pri) % modulus;
		//printf("%s(): %jd = (%jd ^ %jd) mod %jd\n", __func__, val1, generator, pri, modulus);
		if(write(clnt_sock, &val1, sizeof(val1)) != sizeof(val1)) break;

		/* Receive val from client, and perform step 2. */
		int64_t val2;
		read(clnt_sock, &val2, sizeof(val2));
		//printf("SERVER: %jd.\n", (do_exponentiation(val2, pri) % modulus));

		/* Success. */
		retval = val2;

	} while(0);

	if(sock != -1) {
		close(sock);
	}
	if(clnt_sock != -1) {
		close(clnt_sock);
	}

	return (void *) retval;
}

/*******************************************************************************
 *
 * This is the Diffie Hellman client thread.
 *
 ******************************************************************************/
static void *client_thread(void *arg)
{
	int retval = -1;
	int sock = -1;

	//printf("%s(%p): This is the client thread.\n", __func__, arg);

	do
	{
		/* Wait for the server thread to create the server-side part
		 * of the socket pair. */
		int ret = pthread_mutex_lock(&mutex);
		//printf("%s(): pthread_mutex_lock(%p) returned %d.\n", __func__, &mutex, ret);
		if(ret != 0) { break; }

		sock = socket(PF_UNIX, SOCK_STREAM, 0);
		//printf("%s(): socket() returned %d.\n", __func__, sock);
		if(sock < 0) break;

		struct sockaddr_un srvr;
		memset(&srvr, 0, sizeof(srvr));

		srvr.sun_family = AF_UNIX;
		strncpy(srvr.sun_path, CHANNEL, sizeof(srvr.sun_path));

		/* Keep trying until we connect to the server.  If we come up before the
		 * server thread we might get to this point before the server is ready.
		 * So just keep trying until the server shows up. */
		while(1) {
			if(connect(sock, (struct sockaddr *) &srvr, sizeof(srvr)) == 0) {
				break;
			}
			//printf("%s(): sleeping\n", __func__);
			sleep(1);
		}

		int64_t   modulus = 17;
		int64_t generator = 3;
		if(write(sock,   &modulus, sizeof(  modulus)) != sizeof(  modulus)) break;
		if(write(sock, &generator, sizeof(generator)) != sizeof(generator)) break;

		/* Create your own private prime number. */
		int64_t pri = 3;

		/* Perform step 1 and send to server. */
		int64_t val1 = do_exponentiation(generator, pri) % modulus;
		//printf("%s(): %jd = (%jd ^ %jd) mod %jd\n", __func__, val1, generator, pri, modulus);
		if(write(sock, &val1, sizeof(val1)) != sizeof(val1)) break;

		/* Receive val from client, and perform step 2. */
		int64_t val2;
		read(sock, &val2, sizeof(val2));
		//printf("CLIENT: %jd.\n", (do_exponentiation(val2, pri) % modulus));

		/* Success. */
		retval = val2;

	} while(0);

	/* Close the socket. */
	if(sock != -1) {
		//printf("%s(): Calling close(%d).\n", __func__, sock);
		close(sock);
	}

	return (void *) retval;
}

/*******************************************************************************
 *
 ******************************************************************************/
int diffie_hellman_test(void)
{
	printf("%s(): Starting\n", __func__);
	int rc = 1;

	do {
		pthread_attr_t attr;
		pthread_attr_init(&attr);

		/* Initialize and lock the mutex now.  Then create the server
		 * thread.  The thread will create a server-side socket, and
		 * then it will release the mutex (thus allowing us to start
		 * the client thread). */
		pthread_mutex_init(&mutex, NULL);
		rc = pthread_mutex_lock(&mutex);
		//printf("%s(): pthread_mutex_lock(%p) returned %d.\n", __func__, &mutex, rc);
		if(rc != 0) { break; }

		pthread_t server;
		rc = pthread_create(&server, &attr, server_thread, (void *) 1);
		if(rc != 0) { break; }

		pthread_t client;
		rc = pthread_create(&client, &attr, client_thread, (void *) 1);
		if(rc != 0) { break; }

		void *server_thread_rc;
		rc = pthread_join(server, &server_thread_rc);
		int server_rc = (int) server_thread_rc;
		//printf("%s(): pthread_join(server) returned %d: retcode = %d.\n", __func__, rc, server_rc);
		if(rc != 0) { break; }

		void *client_thread_rc;
		rc = pthread_join(client, &client_thread_rc);
		int client_rc = (int) client_thread_rc;
		//printf("%s(): pthread_join(client) returned %d: retcode = %d.\n", __func__, rc, client_rc);
		if(rc != 0) { break; }

		/* Compare results (do they match?). */
		if(server_rc != client_rc) { break; }

		/* Success. */
		rc = 0;

	} while(0);

	printf("%s(): %s.\n", __func__, (rc == 0) ? "PASS" : "FAIL");
	return rc;
}


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

#include "diffie_hellman.h"

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
  int value = (int) arg;
  int retval = -1;

  printf("%s(%d): This is the server thread.\n", __func__, value);

  do
  {
    int ret;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) break;

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    inet_aton("127.0.0.1", &sin.sin_addr);
    sin.sin_port = htons(2345);
    ret = bind(sock, (struct sockaddr *) &sin, sizeof(sin));
    if(ret != 0) break;

    ret = listen(sock, 10);
    if(ret != 0) break;

    struct sockaddr clnt_addr;
    socklen_t       clnt_addr_len = sizeof(clnt_addr);
    int clnt_sock = accept(sock, &clnt_addr, &clnt_addr_len);
    if(clnt_sock == -1) break;

    int64_t modulus;
    if(read(clnt_sock, &modulus, sizeof(modulus)) != sizeof(modulus)) {
      printf("Can't read modulus..\n");
    }

    int64_t generator;
    if(read(clnt_sock, &generator, sizeof(generator)) != sizeof(generator)) {
      printf("Can't read generator..\n");
    }

    /* Create your own private prime number. */
    int64_t pri = 5;

    /* Perform step 1 and send to client. */
    int64_t val1 = do_exponentiation(generator, pri) % modulus;
    printf("%s(): %jd = (%jd ^ %jd) mod %jd\n",
            __func__, val1, generator, pri, modulus);
    if(write(clnt_sock, &val1, sizeof(val1)) != sizeof(val1)) break;

    /* Receive val from client, and perform step 2. */
    int64_t val2;
    read(clnt_sock, &val2, sizeof(val2));
    int64_t secret = do_exponentiation(val2, pri) % modulus;
    printf("SERVER: %jd.\n", secret);

    ret = close(sock);
    if(ret != 0) break;

    /* Success.  */
    retval = 0;

  } while(0);

  return (void *) retval;
}

/*******************************************************************************
 *
 * This is the Diffie Hellman client thread.
 *
 ******************************************************************************/
static void *client_thread(void *arg)
{
  int value = (int) arg;
  int retval = -1;

  printf("%s(%d): This is the client thread.\n", __func__, value);

  do
  {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) break;

    struct sockaddr_in srvr;
    srvr.sin_family = AF_INET;
    srvr.sin_port   = htons(2345);
    inet_aton("127.0.0.1", &srvr.sin_addr);

    /* Keep trying until we connect to the server.  If we come up before the
     * server thread we might get to this point before the server is ready.
     * So just keep trying until the server shows up. */
    while(1) {
      if(connect(sock, (struct sockaddr *) &srvr, sizeof(srvr)) == 0) {
        break;
      }
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
    printf("%s(): %jd = (%jd ^ %jd) mod %jd\n",
            __func__, val1, generator, pri, modulus);
    if(write(sock, &val1, sizeof(val1)) != sizeof(val1)) break;

    /* Receive val from client, and perform step 2. */
    int64_t val2;
    read(sock, &val2, sizeof(val2));
    int64_t secret = do_exponentiation(val2, pri) % modulus;
    printf("CLIENT: %jd.\n", secret);

    /* Success.  */
    close(sock);
    retval = 0;

  } while(0);

  return (void *) retval;
}

/*******************************************************************************
 *
 ******************************************************************************/
int diffie_hellman_test(void)
{
  printf("Testing Diffie Hellman.\n");

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  int rc;

  pthread_t server;
  rc = pthread_create(&server, &attr, server_thread, (void *) 1);
  if(rc != 0) {
    printf("Failed to create server thread (%d).\n", rc);
    return rc;
  }

  pthread_t client;
  rc = pthread_create(&client, &attr, client_thread, (void *) 1);
  if(rc != 0) {
    printf("Failed to create client thread (%d).\n", rc);
    return rc;
  }

  void *thread_rc;
  rc = pthread_join(server, &thread_rc);
  printf("server thread exited. rc %d.  thread rc %p.\n", rc, thread_rc);

  rc = pthread_join(client, &thread_rc);
  printf("client thread exited. rc %d.  thread rc %p.\n", rc, thread_rc);

  return rc;
}



/******************************************************************************
 * This program will allow you to play around with pthread functions.
 *****************************************************************************/

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

/* This is the function that is executed on each thread. */
static void *my_thread(void *arg)
{
  int value = (int) arg;

  printf("%s(%d): This is the thread function.\n", __func__, value);

  /* Display some thread_id stuff. */
  pthread_t self = pthread_self();
  pid_t tid = syscall(SYS_gettid);
  printf("%s(%d): self = %d.  tid = %d.\n", __func__, value, self, tid);

  /* Try some pthread locking tests. */
  int rc;
  pthread_rwlock_t my_rwlock;
  pthread_rwlock_init(&my_rwlock, NULL);

  rc = pthread_rwlock_wrlock(&my_rwlock);
  printf("%s(%d): pthread_rwlock_wrlock() returned %d.  __nr_readers = %d.  __writer = %d.\n",
          __func__, value, rc,  my_rwlock.__data.__nr_readers, my_rwlock.__data.__writer);

  rc = pthread_rwlock_rdlock(&my_rwlock);
  printf("%s(%d): pthread_rwlock_rdlock() returned %d (%s).  __nr_readers = %d.  __writer = %d.\n",
          __func__, value, rc, strerror(rc), my_rwlock.__data.__nr_readers, my_rwlock.__data.__writer);

  rc = pthread_rwlock_unlock(&my_rwlock);
  printf("%s(%d): pthread_rwlock_unlock() returned %d (%s).  __nr_readers = %d.  __writer = %d.\n",
          __func__, value, rc, strerror(rc), my_rwlock.__data.__nr_readers, my_rwlock.__data.__writer);

  rc = pthread_rwlock_unlock(&my_rwlock);
  printf("%s(%d): pthread_rwlock_unlock() returned %d (%s).  __nr_readers = %d.  __writer = %d.\n",
          __func__, value, rc, strerror(rc), my_rwlock.__data.__nr_readers, my_rwlock.__data.__writer);

  return (void *) 1357;
}


int main(int argc, char **argv)
{
  printf("%s(): Testing pthreads.\n", __func__);

  pthread_attr_t attr;
  pthread_attr_init(&attr);

  pthread_t thread;
  int rc = pthread_create(&thread, &attr, my_thread, (void *) 1);
  printf("%s(): rc = %d.\n", __func__, rc);
  if(rc != 0) {
    printf("%s(): Failed to create thread (%d).\n", __func__, rc);
  }

  else {
    void *thread_rc;
    rc = pthread_join(thread, &thread_rc);
    printf("%s(): pthread_join() returned %d.\n", __func__, rc);
    printf("%s(): pthread terminated.  thread_rc = %d.\n", __func__, thread_rc);
  }

  return 0;
}


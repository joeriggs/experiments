
/******************************************************************************
 * This program will allow you to play around with pthread functions.
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

typedef struct thread_msg {
  uint32_t value;

  pthread_mutex_t mutex;

} thread_msg_t;
thread_msg_t my_thread_msg;

/* This is the function that is executed during the 1st pthread test. */
static void *my_thread1(void *arg)
{
  thread_msg_t *my_thread_msg = (thread_msg_t *) arg;

  printf("%s(%p): This is the thread function.\n", __func__, arg);

  /* Display some thread_id stuff. */
  pthread_t self = pthread_self();
  pid_t tid = syscall(SYS_gettid);
  printf("%s(%p): value = %d.  self = %d.  tid = %d.\n", __func__, arg, my_thread_msg->value, self, tid);

  /* Try some pthread locking tests. */
  int rc;
  pthread_rwlock_t my_rwlock;
  pthread_rwlock_init(&my_rwlock, NULL);

  rc = pthread_rwlock_wrlock(&my_rwlock);
  printf("%s(%p): pthread_rwlock_wrlock() returned %d.  __nr_readers = %d.  __writer = %d.\n",
          __func__, arg, rc,  my_rwlock.__data.__nr_readers, my_rwlock.__data.__writer);

  rc = pthread_rwlock_rdlock(&my_rwlock);
  printf("%s(%p): pthread_rwlock_rdlock() returned %d (%s).  __nr_readers = %d.  __writer = %d.\n",
          __func__, arg, rc, strerror(rc), my_rwlock.__data.__nr_readers, my_rwlock.__data.__writer);

  rc = pthread_rwlock_unlock(&my_rwlock);
  printf("%s(%p): pthread_rwlock_unlock() returned %d (%s).  __nr_readers = %d.  __writer = %d.\n",
          __func__, arg, rc, strerror(rc), my_rwlock.__data.__nr_readers, my_rwlock.__data.__writer);

  rc = pthread_rwlock_unlock(&my_rwlock);
  printf("%s(%p): pthread_rwlock_unlock() returned %d (%s).  __nr_readers = %d.  __writer = %d.\n",
          __func__, arg, rc, strerror(rc), my_rwlock.__data.__nr_readers, my_rwlock.__data.__writer);

  return (void *) 1357;
}

/* This is the function that is executed during the 2nd pthread test. */
static void *my_thread2(void *arg)
{
  thread_msg_t *my_thread_msg = (thread_msg_t *) arg;

  printf("%s(%p): This is the thread function.\n", __func__, arg);

  /* Display some thread_id stuff. */
  pthread_t self = pthread_self();
  pid_t tid = syscall(SYS_gettid);
  printf("%s(%p): value = %d.  self = %d.  tid = %d.\n", __func__, arg, my_thread_msg->value, self, tid);

  /* main() locked the mutex.  Modify the value, then release it. */
  int rc;
  pthread_mutex_t *mutex = &my_thread_msg->mutex;
  my_thread_msg->value++;
  rc = pthread_mutex_unlock(mutex);
  printf("%s(): pthread_mutex_unlock(%p) returned %d.\n", __func__, mutex, rc);

  return (void *) 2468;
}

int main(int argc, char **argv)
{
  printf("%s(): Testing pthreads.\n", __func__);

  pthread_attr_t attr;
  pthread_attr_init(&attr);

  pthread_t thread;

  int rc;

  /* Run a test and try some read/write locks. */
  printf("%s(): ===============================================================\n", __func__);
  my_thread_msg.value = 1;
  rc = pthread_create(&thread, &attr, my_thread1, &my_thread_msg);
  printf("%s(): pthread_create() returned %d.\n", __func__, rc);
  if(rc == 0) {
    void *thread_rc;
    rc = pthread_join(thread, &thread_rc);
    printf("%s(): pthread_join() returned %d.\n", __func__, rc);
    printf("%s(): pthread terminated.  thread_rc = %d.\n", __func__, thread_rc);
  }
  else {
    printf("%s(): Failed to create thread (%d).\n", __func__, rc);
  }

  /* Run a test and try a mutex lock. */
  printf("%s(): ===============================================================\n", __func__);
  my_thread_msg.value = 2;

  /* Lock the mutex, and then create the thread.  The thread will modify the
   * value and then unlock the mutex. */
  pthread_mutex_init(&my_thread_msg.mutex, NULL);
  pthread_mutex_lock(&my_thread_msg.mutex);
  rc = pthread_create(&thread, &attr, my_thread2, &my_thread_msg);
  printf("%s(): pthread_create() returned %d.\n", __func__, rc);
  if(rc == 0) {
    pthread_mutex_lock(&my_thread_msg.mutex);
    printf("%s(): Releasing mutex.\n", __func__);
    pthread_mutex_unlock(&my_thread_msg.mutex);
    printf("%s(): value = %d.\n", __func__, my_thread_msg.value);

    void *thread_rc;
    rc = pthread_join(thread, &thread_rc);
    printf("%s(): pthread_join() returned %d.\n", __func__, rc);
    printf("%s(): pthread terminated.  thread_rc = %d.\n", __func__, thread_rc);
  }
  else {
    printf("%s(): Failed to create thread (%d).\n", __func__, rc);
  }

  return 0;
}


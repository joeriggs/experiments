
/******************************************************************************
 * This program will allow you to play around with fork()/exec().
 *****************************************************************************/

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#define CONCURRENT_CHILD_PROCESSES  10
#define TOTAL_CHILD_PROCESSES      200
static int starts     = 0;
static int terminates = 0;

/* For and exec a new process. */
static void
fork_exec(void)
{
  pid_t p = fork();
  if(p == 0)
  {
    int rc = execv("./child", 0);
    printf("execv() returned.  rc = %d.\n", rc);
  }
  else
  {
    starts++;
    printf("New Child PID = %d.  Started %d.\n", p, starts);
  }
}

/* The signal handler. */
static void
my_sig_handler(int sig, siginfo_t *info, void *ctx)
{
  terminates++;

  int childStatus;
  waitpid(info->si_pid, &childStatus, 0);

  printf("Got signal %s from %d (UID %d).  Status %d.  Total of %d signals.\n",
          (sig == SIGCHLD) ? "SIGCHLD" : "???", info->si_pid, info->si_uid, childStatus, terminates);
  if(starts < TOTAL_CHILD_PROCESSES)
  {
    fork_exec();
  }
}

int main(int argc, char **argv)
{
  int i;

  printf("%s(): Testing fork().\n", __func__);

  /* Set up a signal handler.  It'll get called when one of our children
   * terminates. */
  struct sigaction oldSA;
  struct sigaction newSA = {
    .sa_sigaction = my_sig_handler,
    .sa_mask = 0,
    .sa_flags = SA_SIGINFO,
  };
  int sigactionRC = sigaction(SIGCHLD, &newSA, &oldSA);
  if(sigactionRC == -1)
  {
    printf("Unable to set signal handler (%s).\n", strerror(errno));
  }

  else
  {
    /* Fork and exec the child processes. */
    for(i = 0; i < CONCURRENT_CHILD_PROCESSES; i++)
    {
      fork_exec();
    }

    /* Wait for child processes to terminate. */
    while(terminates < TOTAL_CHILD_PROCESSES)
    {
      sleep(1);
    }
  }

  printf("PARENT IS DONE.\n");
  return 0;
}


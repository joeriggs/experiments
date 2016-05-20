/******************************************************************************
 * This program will do a bunch of malloc() and free() calls and dump its own
 * memory allocation status to the console.  The program is useful if you're
 * trying to figure out how an application's memory allocation status changes
 * as it allocates and frees memory.
 *
 * You can view the memory status for your program by looking in the kernel
 * file /proc/???/status.  Look for the Vm???? entries.
 *****************************************************************************/

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

static void print_proc_stats(pid_t pid)
{
  /* Get the resource usage. */
  struct rusage usage;
  int rc = getrusage(RUSAGE_SELF, &usage);
  printf("getrusage() returned %d.\n", rc);
  if(rc == 0) {
    printf("ru_utime    = %d.\n", usage.ru_utime);    /* user CPU time used */
    printf("ru_stime    = %d.\n", usage.ru_stime);    /* system CPU time used */
    printf("ru_maxrss   = %d.\n", usage.ru_maxrss);   /* maximum resident set size */
    printf("ru_ixrss    = %d.\n", usage.ru_ixrss);    /* integral shared memory size */
    printf("ru_idrss    = %d.\n", usage.ru_idrss);    /* integral unshared data size */
    printf("ru_isrss    = %d.\n", usage.ru_isrss);    /* integral unshared stack size */
    printf("ru_minflt   = %d.\n", usage.ru_minflt);   /* page reclaims (soft page faults) */
    printf("ru_majflt   = %d.\n", usage.ru_majflt);   /* page faults (hard page faults) */
    printf("ru_nswap    = %d.\n", usage.ru_nswap);    /* swaps */
    printf("ru_inblock  = %d.\n", usage.ru_inblock);  /* block input operations */
    printf("ru_oublock  = %d.\n", usage.ru_oublock);  /* block output operations */
    printf("ru_msgsnd   = %d.\n", usage.ru_msgsnd);   /* IPC messages sent */
    printf("ru_msgrcv   = %d.\n", usage.ru_msgrcv);   /* IPC messages received */
    printf("ru_nsignals = %d.\n", usage.ru_nsignals); /* signals received */
    printf("ru_nvcsw    = %d.\n", usage.ru_nvcsw);    /* voluntary context switches */
    printf("ru_niivcsw  = %d.\n", usage.ru_nivcsw);   /* involuntary context switches */
  }
}

static void print_status_vm_values(pid_t pid)
{
  int rc;

  if(fork() == 0) {
    char file[1024];
    snprintf(file, sizeof(file), "/proc/%d/status", pid);
    rc = execlp("grep", "", "Vm", file, (char *) 0);
    printf("execlp() returned %d (%d)\n", rc, errno);
  }
  else {
    /* Should waitpid() for the child to terminate, but this is easier. */
    sleep(2);
  }
}

int main(int argc, char **argv)
{
  /* Get and display the PID and PPID. */
  pid_t  pid = getpid();
  pid_t ppid = getppid();
  printf("Current PID is %d.  PPID is %d.\n", pid, ppid);

  /* Allocate a hunk of memory.  This should increase VmPeak, VmSize, and VmData. */
  uint32_t malloc_size = (1024 * 1024);
  uint8_t *buf1 = malloc(malloc_size);
  printf("1st malloc() returned %p\n", buf1);
  print_status_vm_values(pid);

  /* Write to the memory.  This should increase VmHWM and VmRSS. */
  memset(buf1, 0x55, malloc_size);
  printf("Wrote to every byte of the 1st allocated block.\n");
  print_status_vm_values(pid);

  /* Allocate another block.  Should increase VmPeak, VmSize, and VmData again. */
  uint8_t *buf2 = malloc(malloc_size);
  printf("2nd malloc() returned %p\n", buf2);
  print_status_vm_values(pid);

  /* Write to that memory.  Increases VmHWM and VmRSS. */
  memset(buf2, 0x55, malloc_size);
  printf("Wrote to every byte of the 2nd allocated block.\n");
  print_status_vm_values(pid);

  /* Free the 1st block.  Should decrease VmSize and VmData. */
  free(buf1);
  printf("Freed the 1st block of memory.\n");
  print_status_vm_values(pid);

  /* Free the 2nd block.  Should decrease VmSize and VmData. */
  free(buf2);
  printf("Freed the 2nd block of memory.\n");
  print_status_vm_values(pid);

  printf("End of test program.\n");
  return 0;
}


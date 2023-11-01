
/*******************************************************************************
 * This program only runs on FreeBSD.  You can easily modify it to run on a
 * different OS.  Just create your own list of things that need to be sorted.
 ******************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/user.h>

/* Return the maximum number of simultaneous processes that can run on this
 * computer.
 *
 * Returns:
 *   -1 = Failure.
 *  >=0 = The maximum number.
 */
int get_max_proc_count(void)
{
	int mib[2], maxproc;
	size_t len;
	int error;

	mib[0] = CTL_KERN;
	mib[1] = KERN_MAXPROC;
	len = sizeof(maxproc);
	error = sysctl(mib, 2, &maxproc, &len, NULL, 0);
	if (error == -1) {
		fprintf(stderr, "sysctl failed with %d.", error);
		return -1;
	}
	fprintf(stdout, "Maximum number of processes: %d.\n", maxproc);

	return maxproc;
}


/* Populate the proc_list with the current list of processes.
 *
 * Return values:
 *    -1 = Failure.
 *   >=0 = The number of entries in the list.
 */
static int get_process_list(struct kinfo_proc *kproc_list, size_t max_count)
{
	int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PROC, 0};
	size_t len;
	int error;

	error = sysctl(mib, 4, NULL, &len, NULL, 0);
	if (error == -1) {
		fprintf(stderr, "sysctl failed with %d.", error);
		return -1;
	}

	if (len > max_count * sizeof(struct kinfo_proc)) {
		fprintf(stderr, "Insufficient buffer size");
		return -1;
	}

	error = sysctl(mib, 4, kproc_list, &len, NULL, 0);
	if (error == -1) {
		fprintf(stderr, "sysctl for process list failed with %d.", error);
		free(kproc_list);
		return -1;
	}

	size_t count = len / sizeof(struct kinfo_proc);

	return count;
}

/* qsort() will pass 2 proc_info objects to this function.  We compare the
 * pids and return the result.
 */
static int cmp_proc_info_objs(const void *p1, const void *p2)
{
	const struct kinfo_proc *pi1 = p1;
	const struct kinfo_proc *pi2 = p2;
	pid_t pid1 = pi1->ki_pid;
	pid_t pid2 = pi2->ki_pid;

	if (pid1 < pid2)
		return -1;
	if (pid1 == pid2)
		return 0;
	return 1;
}

int
main(int argc, char *argv[])
{
	/* Allocate a buffer to hold the list of currently-running processes.
	 */
	int max_proc_count = get_max_proc_count();
	if (max_proc_count < 0) {
		fprintf(stderr, "Failed to get max process count, cannot proceed.");
		exit(EXIT_FAILURE);
	}

	struct kinfo_proc *list = malloc(max_proc_count * sizeof(struct kinfo_proc));

	if (!list) {
		fprintf(stderr, "Failed to allocate a proc_info structure (%m).");
		exit(EXIT_FAILURE);
	}

	int num_procs = get_process_list(list, max_proc_count);
	if (num_procs == -1) {
		fprintf(stderr, "Failed to get the process list.");
		exit(EXIT_FAILURE);
	}

	qsort(list, num_procs, sizeof(struct kinfo_proc), cmp_proc_info_objs);

	int j;
	for (j = 0; j < num_procs; j++) {
		fprintf(stdout, "%7d\n", list[j].ki_pid);
	}

	exit(EXIT_SUCCESS);
}


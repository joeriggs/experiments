
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/mman.h>

#include "breakpoint.h"

static int test_mmap_pre_processor(
	int arg_01, int arg_02, int arg_03, int arg_04, int arg_05,
	int arg_06, int arg_07, int arg_08, int arg_09, int arg_10)
{
	printf("%s(): %d %d %d %d %d %d\n", __FUNCTION__,
	       arg_01, arg_02, arg_03, arg_04, arg_05, arg_06);

	return 0;
}

static int test_mmap_post_processor(uint64_t retcode)
{
	printf("%s(): 0x%lx\n", __FUNCTION__, retcode);

	return 0;
}

int main(int argc, char **argv)
{
	printf("\nStarting test program.  pid %d\n\n", getpid());

	// Initialize the breakpoint handler.
	if (breakpoint_handler_init()) {
		printf("Failed to initialize breakpoint_handler.\n");
		return 1;
	}

	// Set a breakpoint at the beginning of the mmap() function.
	unsigned char *breakpoint_target = (unsigned char *) mmap;
	if (breakpoint_handler_set(breakpoint_target, test_mmap_pre_processor, test_mmap_post_processor)) {
		printf("Failed to set a breakpoint.\n");
		return 1;
	}
	printf("%s(): Breakpoint inserted at %p.\n\n", __FUNCTION__, breakpoint_target);

	// Create a shared memory space.
	int fd = shm_open("my_shm", O_RDWR | O_CREAT, 0666);
	if(fd < 0) {
		printf("%s(): shm_open() failed (%m).\n", __FUNCTION__);
		return 1;
	}

	// Call mmap() to map the memory.
	//
	// This is the point where our breakpoint code should inject itself
	// into the flow.
	unsigned char *m = mmap(0, 65536, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (!m) {
		printf("%s(): mmap() failed (%m).\n", __FUNCTION__);
		close(fd);
		return 1;
	}
	printf("mmap() returned %p.\n", m);

	// Unmap the memory.  Once again we should inject ourselves.
	int rc = munmap(m, 65536);
	if (rc == -1) {
		printf("%s(): munmap() failed (%m).\n", __FUNCTION__);
		close(fd);
		return 1;
	}

	close(fd);
	return 0;
}


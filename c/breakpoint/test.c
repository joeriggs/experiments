
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/mman.h>

#include "breakpoint.h"

static int test_func_pre_processor(
	int arg_01, int arg_02, int arg_03, int arg_04, int arg_05,
	int arg_06, int arg_07, int arg_08, int arg_09, int arg_10)
{
	printf("%s(): %d %d %d %d %d %d %d %d %d %d\n", __FUNCTION__,
	       arg_01, arg_02, arg_03, arg_04, arg_05,
	       arg_06, arg_07, arg_08, arg_09, arg_10);

	return 0;
}

static int test_func_post_processor(uint64_t retcode)
{
	printf("%s(): 0x%lx\n", __FUNCTION__, retcode);

	return 0;
}

int test_func(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j)
{
	printf("%s(): %d %d %d %d %d %d %d %d %d %d\n",
	       __FUNCTION__, a, b, c, d, e, f, g, h, i, j);

	return 0x12345678;
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
	unsigned char *breakpoint_target = (unsigned char *) test_func;
	if (breakpoint_handler_set(breakpoint_target, test_func_pre_processor, test_func_post_processor)) {
		printf("Failed to set a breakpoint.\n");
		return 1;
	}
	printf("%s(): Breakpoint inserted at %p.\n\n", __FUNCTION__, breakpoint_target);

	// Call test_func.  This is the point where our breakpoint code will
	// inject itself into the flow.
	//
	// 0x119f <main+127>:	pushq  $0xa        10th arg
	// 0x11a1 <main+129>:	mov    $0x3,%edx    3rd arg
	// 0x11a6 <main+134>:	mov    $0x2,%esi    2nd arg
	// 0x11ab <main+139>:	pushq  $0x9         9th arg
	// 0x11ad <main+141>:	mov    $0x1,%edi    1st arg
	// 0x11b2 <main+146>:	mov    $0x4,%ecx    4th arg
	// 0x11b7 <main+151>:	mov    $0x6,%r9d    6th arg
	// 0x11bd <main+157>:	pushq  $0x8         8th arg
	// 0x11bf <main+159>:	mov    $0x5,%r8d    5th arg
	// 0x11c5 <main+165>:	pushq  $0x7         7th arg
	// 0x11c7 <main+167>:	callq  0x12d9 <test_func
	int rc = test_func(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
	printf("test_func() returned 0x%x.\n", rc);

	return 0;
}


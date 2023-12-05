
/* *****************************************************************************
 * This program shows how to modify executable code and insert a breakpoint.  It
 * does 2 things:
 *
 * 1. It inserts itself into the beginning of a function, thus allowing us to
 *    perform some pre-processing before the function runs.
 *
 * 2. It inserts itself into the stack so the function will return to us when
 *    it finishes.  This allows us to perform post-processing after the
 *    function.
 *
 * NOTE: This has only been successfully tested on Ubuntu 20.04 x86_64.  Other
 *       platforms might be added later.
 * ****************************************************************************/

#include <execinfo.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <asm-generic/ucontext.h>
#include <unistd.h>

#include <sys/mman.h>

/* ************************************************************************** */
/* ***************** This is the breakpoint handler.  It's  ***************** */
/* ***************** the engine that processes breakpoints. ***************** */
/* ************************************************************************** */

// We will set this to the return address that mmap() will eventually return
// to (after we intercept the call a couple times).  So in other words, in the
// example of this test program, mmap_retaddr points to the instruction that is
// immediately after the call to mmap(), down in main().
static uint64_t mmap_retaddr = 0ULL;

/* breakpoint_handler() inserts the address of this function into the ctx.
 * This function will execute AFTER mmap() completes.  This allows us to
 * intercept that address of the mapped memory.
 */
static void *breakpoint_handler_return(void)
{
	// The mapped address (as returned from mmap()) is in RAX.  Copy it to
	// a local variable.  We need to do this right away.  Otherwise, there
	// is a risk that RAX will get overwritten.
	uint64_t my_rax = 0ULL;
	asm (
		"mov %%rax, %0;"
		:"=r"(my_rax)
		:
		:
	);
	printf("%s(): my_rax %lx\n\n", __FUNCTION__, my_rax);

	// mmap_retattr contains the return address that goes back to the code
	// that originally called mmap().  We do 3 things right here:
	// 1. Push that return address onto the stack.
	// 2. Set RAX to the address of the mapped memory.
	// 3. Return back to that original caller.
	//
	// The caller will expect the address to be in RAX, so we should be
	// all set.
	printf("%s(): mmap_retaddr %lx\n\n", __FUNCTION__, mmap_retaddr);
	asm (
		"leaveq;"
		"mov %0, %%rax;"
		"push %%rax;"
		"mov %1, %%rax;"
		"ret;"
		:
		:"r"(mmap_retaddr),
		 "r"(my_rax)
		:
	);

	// We never reach this point.
	return NULL;
}

struct ucontext my_ctx;

/* This is a SIGTRAP handler.  It's responsible for handling breakpoints.
 */
static void breakpoint_handler(int sig, siginfo_t *info, void *ptr)
{
	struct ucontext *ctx = (struct ucontext *) ptr;
	uint64_t rsp = ctx->uc_mcontext.rsp;
	uint64_t *p_rsp = (uint64_t *) rsp;

	printf("%s(): sig %d : info %p : ctx %p.\n", __FUNCTION__, sig, info, ctx);
	printf("%s(): rsp %lx (%lx).\n\n", __FUNCTION__, rsp, *p_rsp);

	// Save the address that we'll return to after all of the mmap()
	// processing is complete.  This is the address of the instruction
	// that is immediately after the call to mmap(), down in main().
	mmap_retaddr = *p_rsp;

	// Replace the original return address with our return handler.  This
	// will allow us to save the address of the newly-mapped memory.
	*p_rsp = (uint64_t) breakpoint_handler_return;
}

/* Set up a signal handler to catch SIGTRAP signals.  The SIGTRAP handler is
 * essentially our breakpoint handler.
 *
 * Returns:
 *   0 = success.
 *   1 = failure.
 */
static int breakpoint_handler_init(void)
{
	// Set up a signal handler for our breakpoint handler.
	struct sigaction oldSA;
	struct sigaction newSA;
	int sigactionRC;

	memset(&newSA, 0, sizeof(newSA));
	newSA.sa_sigaction = breakpoint_handler;
	newSA.sa_flags = SA_SIGINFO;
	sigactionRC = sigaction(SIGTRAP, &newSA, &oldSA);
	if (sigactionRC == -1) {
		return 1;
	}

	return 0;
}

/* Set a breakpoint.
 *
 * Returns:
 *   0 = success.
 *   1 = failure.
 */
static int breakpoint_handler_set(void *addr)
{
	// Get the memory page size.  It's probably 4,096 bytes.
	int pagesize = sysconf(_SC_PAGE_SIZE);
	if (pagesize == -1) {
		return 1;
	}

	// Create a mask that we'll use to get the base address of the
	// memory page that contains the function that we're going to modify.
	//
	// TODO: If the first 4 bytes of the function straddle 2 memory pages,
	//       then we need to make both of those pages writeable.  At this
	//       time, we only make one page writeable.
	int pagemask = ~(pagesize - 1);

	// Calculate the base address of that memory page that contains the
	// function.
	unsigned char *base_addr = (unsigned char*) ((size_t) addr & pagemask);

	// Make the page writeable.  It's executable code, so we have to
	// enable writing to it in order to modify the code.
	int rc = mprotect(base_addr, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC);
	if (rc != 0) {
		return 1;
	}

	// Modify the code.  Replace the endbr64 instruction with 4 NOPs.
	//
	// TODO: This is how Ubuntu functions look.  They have an endbr64
	//       instruction at the beginning of the function.  This is NOT
	//       the case on other operating systems.  So we need to change
	//       this code if we want it to work on other distributions.
	unsigned char *p = (unsigned char *) addr;
	p[0] = 0x90;
	p[1] = 0x90;
	p[2] = 0x90;
	p[3] = 0xcc;

	return 0;
}

/* ************************************************************************** */
/* ************************************************************************** */
/* ******************* End of the breakpoint handler code ******************* */
/* ************************************************************************** */
/* ************************************************************************** */

int main( )
{
	printf("\nStarting test program.  pid %d\n\n", getpid());

	// Initialize the breakpoint handler.
	if (breakpoint_handler_init()) {
		printf("Failed to initialize breakpoint_handler.\n");
		return 1;
	}

	// Set a breakpoint at the beginning of the mmap() function.
	unsigned char *breakpoint_target = (unsigned char *) mmap;
	if (breakpoint_handler_set(breakpoint_target)) {
		printf("Failed to set a breakpoint.\n");
		return 1;
	}
	printf("%s(): Breakpoint inserted at %p.\n", __FUNCTION__, breakpoint_target);

	// Call mmap().
	unsigned char *m = NULL;
	int fd = shm_open("my_shm", O_RDWR | O_CREAT, 0666);
	printf("shm_open() returned %d (%m).\n", fd);
	if(fd >= 0)
	{
		// mov    -0x1c(%rbp),%eax # fd                     -> eax (32-bits)
		// mov    $0x0,%r9d        # offset                 -> r9d (32-bits)
		// mov    %eax,%r8d        # fd                     -> r8d (32-bits)
		// mov    $0x1,%ecx        # MAP_SHARED             -> ecx (32-bits)
		// mov    $0x3,%edx        # PROT_READ | PROT_WRITE -> edx (32-bits)
		// mov    $0x10000,%esi    # size                   -> esi (32-bits)
		// mov    $0x0,%edi        # addr                   -> edi (32-bits)
		// callq  0x10f0 <mmap@plt>

		m = mmap(0, 65536, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	}
	printf("mmap() returned %p (%m).\n", m);

	return 0;
}



/* *****************************************************************************
 * This program shows how to modify source code and insert a breakpoint.  It
 * also shows how to insert yourself into the flow so that you can intercept
 * a function when it is called, and you can intercept it when it returns.
 *
 * NOTE: This has only been successfully tested on Ubuntu 20.04 x86_64.  Other
 *       platforms will be added later.
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
/* ************************************************************************** */
/* ***************** This is the breakpoint handler.  It's  ***************** */
/* ***************** the engine that processes breakpoints. ***************** */
/* ************************************************************************** */
/* ************************************************************************** */

static void breakpoint_handler(int sig, siginfo_t *info, void *ptr);

static int pagesize = -1;
static int pagemask = -1;

static uint64_t return_address = 0ULL;
static unsigned char *retaddr = NULL;
static uint64_t mmap_retaddr = 0ULL;

/* breakpoint_handler() inserts the address of this function into the ctx.
 */
static void *breakpoint_handler_return(void)
{
#if 1
	uint64_t *my_rax = NULL;
	asm (
		"mov %%rax, %0;"
		:"=r"(my_rax)
		:
		:
	);
	printf("%s(): my_rax %p\n\n", __FUNCTION__, my_rax);
	printf("%s(): mmap_retaddr %lx\n\n", __FUNCTION__, mmap_retaddr);
#endif

#if 1
	{
		void *tracePtrs[100];
		int count = backtrace(tracePtrs, 100);
		int i;
		for(i = 0; i < count; i++) {
			printf("%s(): %p\n", __FUNCTION__, tracePtrs[i]);
		}
		printf("\n");
	}
#endif

#if 1
	asm (
		"leaveq;"
		"mov %0, %%rax;"
		"push %%rax;"
		"ret;"
		:
		:"r"(mmap_retaddr)
		:
	);
#endif

	// We never reach this point.
	return my_rax;
}

struct ucontext my_ctx;

/* This is a SIGTRAP handler.  It's responsible for handling breakpoints.
 */
static void breakpoint_handler(int sig, siginfo_t *info, void *ptr)
{
	int i;
	void *tracePtrs[100];
	int count = backtrace(tracePtrs, 100);
	char **funcNames = backtrace_symbols(tracePtrs, count);
	for(i = 0; i < count; i++) {
		printf("%s(): %p : %s\n", __FUNCTION__, tracePtrs[i], funcNames[i]);
	}
	free(funcNames);
	printf("\n");

	struct ucontext *ctx = (struct ucontext *) ptr;
	uint64_t rip = ctx->uc_mcontext.rip;
	uint64_t rsp = ctx->uc_mcontext.rsp;
	uint64_t *p_rsp = (uint64_t *) rsp;

	uint64_t real_rsp;
	asm (
		"mov %%rsp, %0;"
		:"=r"(real_rsp)
		:
		:
	);


	printf("%s(): sig %d : info %p : ctx %p.\n", __FUNCTION__, sig, info, ctx);
	printf("%s(): return_address %lx : rsp %lx (%lx).\n\n", __FUNCTION__, rip, rsp, *p_rsp);

	mmap_retaddr = *p_rsp;

	uint64_t *p = (uint64_t *) rsp - 2;
	for (i = 0; i < 4; i++, p++) {
		printf("%s(): %p = %lx.\n", __FUNCTION__, p, *p);
	}
	printf("\n");

	return_address = ctx->uc_mcontext.rip;
	//ctx->uc_mcontext.rip = (uint64_t) breakpoint_handler_return;
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
	// Get the memory page size.  It's probably 4,096 bytes.
	//
	// Then create a mask that we'll use to get the base address of the
	// memory page that contains the function that we're going to modify.
	pagesize = sysconf(_SC_PAGE_SIZE);
	if (pagesize == -1) {
		return 1;
	}
	pagemask = ~(pagesize - 1);

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
	// Calculate the base address of that memory page that contains the
	// function.
	unsigned char *addr2 = (unsigned char*) ((size_t) addr & pagemask);

	// Make the page writeable.  It's executable code, so we have to
	// enable writing to it in order to modify the code.
	int rc = mprotect(addr2, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC);
	if (rc != 0) {
		return 1;
	}

	// Modify the code.  Replace the endbr64 instruction with 4 NOPs.
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

	if (breakpoint_handler_init()) {
		printf("Failed to initialize breakpoint_handler.\n");
		return 1;
	}

	unsigned char *breakpoint_target = (unsigned char *) mmap;
	retaddr = breakpoint_target + 4;

	if (breakpoint_handler_set(breakpoint_target)) {
		printf("Failed to set a breakpoint.\n");
		return 1;
	}
	printf("%s(): Breakpoint inserted at %p.\n", __FUNCTION__, breakpoint_target);
	printf("%s():      return_address at %p.\n\n", __FUNCTION__, retaddr);

	/* Call the test function.  Make sure it still works.  And make
	 * sure the breakpoint handler executed.
	 */
	unsigned char *m = NULL;
	int fd = shm_open("my_shm", O_RDWR | O_CREAT, 0666);
	printf("shm_open() returned %d (%m) (%s).\n", fd, (fd >= 0) ? "PASS": "FAIL");
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
	printf("mmap() returned %p.\n", m);

	return 0;
}



#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>

/* ************************************************************************** */
/* ************************************************************************** */
/* ***************** This is the breakpoint handler.  It's  ***************** */
/* ***************** the engine that processes breakpoints. ***************** */
/* ************************************************************************** */
/* ************************************************************************** */

static int pagesize = -1;
static int pagemask = -1;

/* This is a SIGTRAP handler.  It's responsible for handling breakpoints.
 */
static void breakpoint_handler(int sig, siginfo_t *info, void *ctx)
{
	printf("%s(): sig %d : info %p : ctx %p.\n",
	       __FUNCTION__, sig, info, ctx);
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
static int breakpoint_handlet_set(void *addr)
{
	// Calculate the base address of that memory page that contains the
	// function.
	unsigned char *addr2 = (char*) ((size_t) addr & pagemask);

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

static int test_function(void)
{
	printf("%s(): I am in the function.\n", __FUNCTION__);

	return 0x12345678;
}

int main( )
{
	if (breakpoint_handler_init()) {
		printf("Failed to initialize breakpoint_handler.\n");
		return 1;
	}

	if (breakpoint_handlet_set(test_function)) {
		printf("Failed to set a breakpoint.\n");
		return 1;
	}

	/* Call the test function.  Make sure it still works.  And make
	 * sure the breakpoint handler executed.
	 */
	test_function();
	printf("Call returned.\n");

	return 0;
}



/* *****************************************************************************
 * This code shows how to modify executable .text segment and insert an "INT 3"
 * breakpoint.
 *
 * NOTE: This has only been successfully tested on Ubuntu 20.04 x86_64.  Other
 *       platforms might be added later.
 * ****************************************************************************/

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <asm-generic/ucontext.h>

#include <sys/mman.h>

#include "breakpoint.h"
#include "breakpoint_internal.h"

// These are the context for our breakpoints.
#define NUM_BREAKPOINTS 8
breakpoint_context ctx_list[NUM_BREAKPOINTS];

extern __thread breakpoint_context *curr_ctx;

// We will set this to the return address that the tracked function will return
// to (after we intercept the call).  So in other words, in the example of this
// test program, func_retaddr points to the instruction that is immediately
// after the call to the function that we're tracking.
extern __thread uint64_t func_retaddr;

/* This is a SIGTRAP handler.  It's responsible for handling breakpoints.  It is
 * the means by which we register our breakpoint handler with the Linux kernel.
 * Whenever an "INT 3" instruction is executed, this function will be called.
 */
static void breakpoint_handler(int sig, siginfo_t *info, void *ptr)
{
	struct ucontext *ctx = (struct ucontext *) ptr;
	uint64_t rsp = ctx->uc_mcontext.rsp;
	uint64_t *p_rsp = (uint64_t *) rsp;
 
	uint64_t *p_rip = (uint64_t *) ctx->uc_mcontext.rip;
	printf("%s(): p_rip %p.\n", __FUNCTION__, p_rip);

	// We support up to 10 function parameters.
	uint64_t arg_01 = ctx->uc_mcontext.rdi;
	uint64_t arg_02 = ctx->uc_mcontext.rsi;
	uint64_t arg_03 = ctx->uc_mcontext.rdx;
	uint64_t arg_04 = ctx->uc_mcontext.rcx;
	uint64_t arg_05 = ctx->uc_mcontext.r8;
	uint64_t arg_06 = ctx->uc_mcontext.r9;
	uint64_t arg_07 = p_rsp[1];
	uint64_t arg_08 = p_rsp[2];
	uint64_t arg_09 = p_rsp[3];
	uint64_t arg_10 = p_rsp[4];
	printf("%s(): %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
	       __FUNCTION__,
	       arg_01, arg_02, arg_03, arg_04, arg_05,
	       arg_06, arg_07, arg_08, arg_09, arg_10);

	// Locate the context for this particular breakpoint.
	int i;
	for (i = 0, curr_ctx = NULL; i < NUM_BREAKPOINTS; i++) {
		if (p_rip == ctx_list[i].bp_retaddr) {
			curr_ctx = &ctx_list[i];
			break;
		}
	}

	if (curr_ctx) {
		curr_ctx->pre_cb(arg_01, arg_02, arg_03, arg_04, arg_05,
				 arg_06, arg_07, arg_08, arg_09, arg_10);
	}

	// Save the actual return address.
	func_retaddr = *p_rsp;

	// Replace the actual return address with our post-processing handler.
	extern void breakpoint_return(void);
	*p_rsp = (uint64_t) breakpoint_return;
}

/* Set up a signal handler to catch SIGTRAP signals.  The SIGTRAP handler is
 * essentially our breakpoint handler.
 *
 * Returns:
 *   0 = success.
 *   1 = failure.
 */
int breakpoint_handler_init(void)
{
	// Initialize our list of breakpoints.
	int i;
	for (i = 0; i < NUM_BREAKPOINTS; i++) {
		ctx_list[i].used = 0;
	}

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
 * Input:
 *
 *   addr is the address of the function that we will set the breakpoint in.
 *
 *   ctx is the caller-provided context that allows the caller to provide
 *       instructions on how we should process the breakpoint.
 *
 * Returns:
 *   0 = success.
 *   1 = failure.
 */
int breakpoint_handler_set(void *addr,
                           pre_processor_callback pre_cb,
                           post_processor_callback post_cb)
{
	breakpoint_context *ctx = NULL;

	// Reserve a breakpoint context.
	int i;
	for (i = 0; i < NUM_BREAKPOINTS; i++) {
		if (!ctx_list[i].used) {
			ctx = &ctx_list[i];
			ctx->used = 1;
			ctx->pre_cb = pre_cb;
			ctx->post_cb = post_cb;

			ctx->bp_entrypoint = addr;
			unsigned char *ret = (unsigned char *) addr;
			ret += 4;
			ctx->bp_retaddr = ret;

			break;
		}
	}

	// Get the size of each memory page.  It's probably 4,096 bytes.
	int pagesize = sysconf(_SC_PAGE_SIZE);
	if (pagesize == -1) {
		return 1;
	}

	// Create a mask that we'll use to get the base address of the
	// memory page that contains the function we're going to modify.
	//
	// TODO: If the first 4 bytes of the function straddle 2 memory pages,
	//       then we need to make both of those pages writeable.  At this
	//       time, we only make one page writeable.
	int pagemask = ~(pagesize - 1);

	// Calculate the base address of the memory page that contains the
	// function.
	unsigned char *base_addr = (unsigned char*) ((size_t) addr & pagemask);

	// Make the page writeable.  It's executable code, so we have to
	// enable writing to it in order to modify the code.
	int rc = mprotect(base_addr, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC);
	if (rc != 0) {
		return 1;
	}

	// Modify the code.  Replace the endbr64 instruction with 3 NOPs and an
	// "INT 3" instruction.
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


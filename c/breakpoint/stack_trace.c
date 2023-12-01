
#include <execinfo.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>

static void func3(void)
{
	uint64_t *my_rsp = NULL;
	uint64_t *my_rbp = NULL;
	asm (
		"mov %%rsp, %%rax;"
		"mov %%rax, %0;"
		"mov %%rbp, %%rax;"
		"mov %%rax, %1;"
		:"=r"(my_rsp),
		 "=r"(my_rbp)
		:
		:"%eax"
	);
	printf("%s(): my_rsp %p : my_rbp %p.\n\n", __FUNCTION__, my_rsp, my_rbp);

	int i;
	void *tracePtrs[100];
	int count = backtrace(tracePtrs, 100);
	char **funcNames = backtrace_symbols(tracePtrs, count);
	for(i = 0; i < count; i++) {
		printf("%p : %s\n", tracePtrs[i], funcNames[i]);
	}
	free(funcNames);
	printf("\n");

	while (my_rbp) {
		printf("%s(): my_rbp %p : *rbp %16lx : rip %16lx.\n",
		       __FUNCTION__, my_rbp, my_rbp[0], my_rbp[1]);
		my_rbp = (uint64_t *) *my_rbp;
	}
}

static void func2(void)
{
	func3();
}

static void func1(void)
{
	func2();
}

int main(int argc, char *argv)
{
	printf("%s(): func1() %p.\n", __FUNCTION__, func1);
	printf("%s(): func2() %p.\n", __FUNCTION__, func2);
	printf("%s(): func3() %p.\n", __FUNCTION__, func3);

	func1();

	return 0;
}


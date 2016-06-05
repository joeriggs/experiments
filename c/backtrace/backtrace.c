/* Test the backtrace() function.  Useful for diagnose for program errors. */

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

void function2(void)
{
	int i;
	void *tracePtrs[100];
	int count = backtrace(tracePtrs, 100);
	char **funcNames = backtrace_symbols(tracePtrs, count);
	for(i = 0; i < count; i++) {
		printf("%s\n", funcNames[i]);
	}
	free(funcNames);
}

void function1(void)
{
	function2();
}

int main(int argc, char **argv)
{
	function1();
	return 0;
}


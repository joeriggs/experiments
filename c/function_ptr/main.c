
/******************************************************************************
 * This program demonstrates how to set up a function pointer.
 *****************************************************************************/

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

typedef int (*myMathFuncType)(int arg1, int arg2);

int myAddFunc(int a1, int a2)
{
	return (a1 + a2);
}

int mySubFunc(int a1, int a2)
{
	return (a1 - a2);
}

int
main(int argc, char **argv)
{
	printf("Testing function pointers.\n");
	myMathFuncType myFuncPtr;

	/* Test add. */
	myFuncPtr = myAddFunc;
	printf("First result = %d.\n", myFuncPtr(3, 2));

	/* Test sub. */
	myFuncPtr = mySubFunc;
	printf("First result = %d.\n", myFuncPtr(3, 2));

	return 0;
}


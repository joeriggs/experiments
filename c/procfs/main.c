
/*******************************************************************************
 * This program will look at the various files in /proc/self.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	printf("Look at the stuff in /proc\n");

	/* Look at /proc/self/status. */
	FILE *f = fopen("/proc/self/status", "r");
	printf("fopen() returned %p\n", f);
	if(f > 0) {
		int x;
		for(x = 0; x < 10; x++) {
			char name[1024];
			int num;

			int matchNum = fscanf(f, "%s %d", name, &num);
			printf("fscanf() returned %d : %s : %d\n", matchNum, name, num);
		}

		fclose(f);
	}

	return 0;
}


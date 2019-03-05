/* *****************************************************************************
 * Do a whole bunch of malloc() calls followed by a whole bunch of free() calls.
 * This allows the user to observe the memory usage of the process to see if it
 * goes up (and more importantly, back down).
 * ****************************************************************************/

#include <inttypes.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>

#define MALLOC_SIZE (16 * 1024 * 1024)
#define MALLOC_TOTAL (1 * 1024 * 1024 * 1024)
#define MALLOC_COUNT (MALLOC_TOTAL / MALLOC_SIZE)

uint8_t *ptrs[MALLOC_COUNT];

int main(int argc, char **argv)
{
	uint64_t totalSize = 0ULL;

	printf("%" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
	       MALLOC_SIZE, MALLOC_TOTAL, MALLOC_COUNT);
	sleep(5);

	int i;
	for(i = 0; i < MALLOC_COUNT; i++) {
		ptrs[i] = (uint8_t *) calloc(1, MALLOC_SIZE);
		if(ptrs[i] != NULL) {
			int x;
			uint8_t *p = ptrs[i];
			for(x = 0; x < MALLOC_SIZE; x++) {
				p[x] = x;
			}
		}

		totalSize += MALLOC_SIZE;

		printf("%d: ptr %p: totalSize %" PRIu64 "\n",
		        i, ptrs[i], totalSize);

		usleep(500000);
	}

	for(i = 0; i < MALLOC_COUNT; i++) {
		printf("%d: Free ptr %p\n", i, ptrs[i]);
		free(ptrs[i]);
		usleep(500000);
	}

	sleep(3600);
	return 0;
}


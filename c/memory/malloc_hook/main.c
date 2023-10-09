
#define RTLD_NEXT  ((void *) -1l)

#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>

#define MEM_HOOK_LOGGER printf

static int doTheLog = 1;
static int doTheQueue = 1;

static __thread int doingPrint = 0;

static void *(*__calloc)(size_t number, size_t size) = NULL;
static void *(*__malloc)(size_t size) = NULL;
static int   (*__posix_memalign)(void **ptr, size_t alignment, size_t size) = NULL;
static void *(*__realloc)(void *ptr, size_t size) = NULL;
static void  (*__free)(const void *addr) = NULL;

#define NUM_CALLERS 32
typedef struct my_queue {
	int used;
	size_t num_callers;
	void *callers[NUM_CALLERS];
	void *ptr;
	size_t size;
} my_queue;
#define MY_QUEUE_SIZE 1000000
static my_queue myq[MY_QUEUE_SIZE];
static int myq_adds = 0;
static int myq_dels = 0;
pthread_mutex_t myq_mutex;

static void myq_print(void) {
	int num_entries = 0;
	doingPrint = 1;
	MEM_HOOK_LOGGER("=========================================================\n");
	MEM_HOOK_LOGGER("myq_adds %d : myq_dels %d.", myq_adds, myq_dels);
	pthread_mutex_lock(&myq_mutex);

	int i;
	char callerList[NUM_CALLERS * 32];
	for (i = 0; i < MY_QUEUE_SIZE; i++) {
		if (myq[i].used) {
			num_entries++;
			memset(callerList, 0, sizeof(callerList));
			int x;
			for (x = 0; (x < NUM_CALLERS) && (x < myq[i].num_callers); x++) {
				char oneCaller[32];
				sprintf(oneCaller, "%p ", myq[i].callers[x]);
				strcat(callerList, oneCaller);
			}
			MEM_HOOK_LOGGER("%5d: Ptr %p: Size %ld: Callers (%ld) (%s).\n",
				    i, myq[i].ptr, myq[i].size, myq[i].num_callers, callerList);
		}
	}

	pthread_mutex_unlock(&myq_mutex);
	MEM_HOOK_LOGGER("num_entries %d.", num_entries);
	MEM_HOOK_LOGGER("=========================================================\n");
	doingPrint = 0;
}

static void myq_add(void *ptr, size_t size) {
	if (doTheQueue) {
		int i;
		void *tracePtrs[100];
		int num_callers = backtrace(tracePtrs, 100);

		if ((num_callers == 0) || (num_callers > 100)) {
			printf("JOE RIGGS SPOTTED AN ERROR (%d) (%m).", num_callers);
		}

		pthread_mutex_lock(&myq_mutex);
		for (i = 0; i < MY_QUEUE_SIZE; i++) {
			if (myq[i].used == 0) {
				myq[i].used = 1;
				// The first 2 callers are our malloc_hook functions.
				// Skip them.
				myq[i].num_callers = num_callers - 2;
				int x;
				for (x = 0; (x < NUM_CALLERS) && (x < num_callers); x++) {
					myq[i].callers[x] = tracePtrs[x + 2];
				}
				myq[i].ptr = ptr;
				myq[i].size = size;
				break;
			}
		}
		myq_adds++;
		pthread_mutex_unlock(&myq_mutex);

		if (i == MY_QUEUE_SIZE)
			MEM_HOOK_LOGGER("Ran out of slots.");
	}
}

static void myq_del(void *ptr) {
	if (doTheQueue) {
		int i;

		pthread_mutex_lock(&myq_mutex);
		for (i = 0; i < MY_QUEUE_SIZE; i++) {
			if (myq[i].ptr == ptr) {
				myq[i].used = 0;
				break;
			}
		}
		myq_dels++;
		pthread_mutex_unlock(&myq_mutex);
	}
}

static void myq_reset(void) {
	pthread_mutex_lock(&myq_mutex);
	memset(myq, 0, sizeof(myq));
	myq_adds = 0;
	myq_dels = 0;
	pthread_mutex_unlock(&myq_mutex);
}

static void *malloc_hooks_thread(void *arg)
{
	char *gp_path = (char *)arg;

	while(1) {
		sleep(1);

		struct stat s;
		if (stat("/tmp/doq", &s) == 0) {
			printf("JOE RIGGS: Start the queue.");
			unlink("/tmp/doq");
			doTheQueue = 1;
		}
		if (stat("/tmp/noq", &s) == 0) {
			printf("JOE RIGGS: Stop the queue.");
			unlink("/tmp/noq");
			doTheQueue = 0;
		}
		if (stat("/tmp/print", &s) == 0) {
			printf("JOE RIGGS: Print the queue.");
			unlink("/tmp/print");
			myq_print();
		}
		if (stat("/tmp/reset", &s) == 0) {
			printf("JOE RIGGS: Reset the queue.");
			unlink("/tmp/reset");
			myq_reset();
		}
	}

	return NULL;
}

static void malloc_hooks_init(void)
{
	pthread_t tid;
	int ret = pthread_create(&tid, NULL, malloc_hooks_thread, NULL);
	if (ret) {
		printf("pthread_create() failed (%m).");
	}
}

void *calloc(size_t number, size_t size)
{
	if (__calloc == NULL)
		__calloc = dlsym(RTLD_NEXT, "calloc");

	void *ptr = __calloc(number, size);

	if (doingPrint == 0) {
		doingPrint = 1;
		myq_add(ptr, size);

		const void *caller = __builtin_return_address(0);
		if (doTheLog) {
			MEM_HOOK_LOGGER("%s(): number %lu: size %lu: caller %p: ptr %p.\n", __FUNCTION__, number, size, caller, ptr);
		}
		doingPrint = 0;
	}

	return ptr;
}

void *malloc(size_t size)
{
	if (__malloc == NULL)
		__malloc = dlsym(RTLD_NEXT, "malloc");

	void *ptr = __malloc(size);

	if (doingPrint == 0) {
		doingPrint = 1;
		myq_add(ptr, size);

		const void *caller = __builtin_return_address(0);
		if (doTheLog) {
			MEM_HOOK_LOGGER("%s(): size %lu: caller %p: ptr %p.\n", __FUNCTION__, size, caller, ptr);
		}
		doingPrint = 0;
	}

	return ptr;
}

int posix_memalign(void **ptr, size_t alignment, size_t size)
{
	if (__posix_memalign == NULL)
		__posix_memalign = dlsym(RTLD_NEXT, "posix_memalign");

	int rc = __posix_memalign(ptr, alignment, size);

	if (doingPrint == 0) {
		doingPrint = 1;
		myq_add(*ptr, size);

		const void *caller = __builtin_return_address(0);
		if (doTheLog) {
			MEM_HOOK_LOGGER("%s(): ptr %p (%p): alignment %lu: size %lu: caller %p: ptr %p.\n", __FUNCTION__, ptr, *ptr, alignment, size, caller, ptr);
		}
		doingPrint = 0;
	}

	return rc;
}

void *realloc(void *ptr, size_t size)
{
	if (__realloc == NULL)
		__realloc = dlsym(RTLD_NEXT, "realloc");

	void *new_ptr = __realloc(ptr, size);

	if (doingPrint == 0) {
		doingPrint = 1;
		myq_add(new_ptr, size);

		const void *caller = __builtin_return_address(0);
		if (doTheLog) {
			MEM_HOOK_LOGGER("%s(): ptr %p: size %lu: caller %p: new_ptr %p.\n", __FUNCTION__, ptr, size, caller, new_ptr);
		}
		doingPrint = 0;
	}

	return new_ptr;
}

void free(void *ptr)
{
	if (__free == NULL)
		__free = dlsym(RTLD_NEXT, "free");

	__free(ptr);

	if (doingPrint == 0) {
		doingPrint = 1;
		myq_del(ptr);

		const void *caller = __builtin_return_address(0);
		if (doTheLog) {
			MEM_HOOK_LOGGER("%s(): caller %p: ptr %p.\n", __FUNCTION__, caller, ptr);
		}
		doingPrint = 0;
	}
}

int main(int argc, char *argv[])
{
	void *p1 = NULL, *p2 = NULL, *p3 = NULL, *p4 = NULL;

	printf("Do some alloc tests =====================================\n");
	p1 = malloc(1);
	p2 = calloc(10, 1);
	int rc = posix_memalign(&p3, 64, 1024);
	p4 = realloc(p1, 1024);
	myq_print();

	printf("Do some free tests ======================================\n");
	//free(p1);
	free(p2);
	free(p3);
	free(p4);
	myq_print();

	return 0;
}

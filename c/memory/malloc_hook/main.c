
#define RTLD_NEXT  ((void *) -1l)

#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>

static int doTheLog = 1;

static void *(*__calloc)(size_t number, size_t size) = NULL;
static void *(*__malloc)(size_t size) = NULL;
static int   (*__posix_memalign)(void **ptr, size_t alignment, size_t size) = NULL;
static void *(*__realloc)(void *ptr, size_t size) = NULL;
static void  (*__free)(const void *addr) = NULL;

static int doTheQueue = 1;
typedef struct my_queue {
	int used;
	void *caller1;
	void *caller2;
	void *ptr;
} my_queue;
#define MY_QUEUE_SIZE 1024
static my_queue myq[MY_QUEUE_SIZE];
static int myq_adds = 0;
static int myq_dels = 0;
pthread_mutex_t myq_mutex;

void myq_print(void) {
	if (doTheQueue) {
		printf("=========================================================\n");
		pthread_mutex_lock(&myq_mutex);
		int i;
		for (i = 0; i < MY_QUEUE_SIZE; i++) {
			if (myq[i].used) {
				printf("%5d: Ptr %p: Callers %p %p.\n",
					    i, myq[i].ptr, myq[i].caller1, myq[i].caller2);
			}
		}
		pthread_mutex_unlock(&myq_mutex);
		printf("=========================================================\n");
	}
}

static void myq_add(void *ptr) {
	if (doTheQueue) {
		int i;
		void *caller1 = __builtin_return_address(1);
		void *caller2 = __builtin_return_address(2);

		pthread_mutex_lock(&myq_mutex);
		for (i = 0; i < MY_QUEUE_SIZE; i++) {
			if (myq[i].used == 0) {
				myq[i].used = 1;
				myq[i].caller1 = caller1;
				myq[i].caller2 = caller2;
				myq[i].ptr = ptr;
				break;
			}
		}
		myq_adds++;
		pthread_mutex_unlock(&myq_mutex);

		if (i == MY_QUEUE_SIZE)
			printf("Ran out of slots.");
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

static __thread int doingPrint = 0;

void *calloc(size_t number, size_t size)
{
	void *ptr = __calloc(number, size);

	if (doingPrint == 0) {
		doingPrint = 1;
		myq_add(ptr);

		const void *caller = __builtin_return_address(0);
		if (doTheLog) {
			printf("%s(): number %lu: size %lu: caller %p: ptr %p.\n", __FUNCTION__, number, size, caller, ptr);
		}
		doingPrint = 0;
	}

	return ptr;
}

void *malloc(size_t size)
{
	void *ptr = __malloc(size);

	if (doingPrint == 0) {
		doingPrint = 1;
		myq_add(ptr);

		const void *caller = __builtin_return_address(0);
		if (doTheLog) {
			printf("%s(): size %lu: caller %p: ptr %p.\n", __FUNCTION__, size, caller, ptr);
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
		myq_add(*ptr);

		const void *caller = __builtin_return_address(0);
		if (doTheLog) {
			printf("%s(): ptr %p (%p): alignment %lu: size %lu: caller %p: ptr %p.\n", __FUNCTION__, ptr, *ptr, alignment, size, caller, ptr);
		}
		doingPrint = 0;
	}

	return rc;
}

void *realloc(void *ptr, size_t size)
{
	void *new_ptr = __realloc(ptr, size);

	if (doingPrint == 0) {
		doingPrint = 1;
		myq_add(new_ptr);

		const void *caller = __builtin_return_address(0);
		if (doTheLog) {
			printf("%s(): ptr %p: size %lu: caller %p: new_ptr %p.\n", __FUNCTION__, ptr, size, caller, new_ptr);
		}
		doingPrint = 0;
	}

	return new_ptr;
}

void free(void *ptr)
{
	__free(ptr);

	if (doingPrint == 0) {
		doingPrint = 1;
		myq_del(ptr);

		const void *caller = __builtin_return_address(0);
		if (doTheLog) {
			printf("%s(): caller %p: ptr %p.\n", __FUNCTION__, caller, ptr);
		}
		doingPrint = 0;
	}
}

int main(int argc, char *argv[])
{
	__calloc = dlsym(RTLD_NEXT, "calloc");
	__malloc = dlsym(RTLD_NEXT, "malloc");
	__realloc = dlsym(RTLD_NEXT, "realloc");
	__posix_memalign = dlsym(RTLD_NEXT, "posix_memalign");
	__free   = dlsym(RTLD_NEXT, "free");

	void *p1 = NULL, *p2 = NULL, *p3 = NULL, *p4 = NULL;

	printf("Do some alloc tests =====================================\n");
	p1 = malloc(1);
	p2 = calloc(10, 1);
	int rc = posix_memalign(&p3, 64, 1024);
	p4 = realloc(p1, 1024);
	myq_print();

	printf("Do some free tests ======================================\n");
	free(p1);
	free(p2);
	free(p3);
	free(p4);
	myq_print();

	return 0;
}

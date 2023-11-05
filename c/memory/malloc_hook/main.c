
#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/queue.h>
#include <sys/stat.h>

#include <sys/types.h>

#ifndef RTLD_NEXT
#define RTLD_NEXT  ((void *) -1l)
#endif

#define MEM_HOOK_LOGGER printf
#define LOG_FILE "/tmp/malloc.log"

static int doTheLog = 0;
static int doTheQueue = 0;

static __thread int doingPrint = 0;

static void *(*__calloc)(size_t number, size_t size) = NULL;
static void *(*__malloc)(size_t size) = NULL;
static int   (*__posix_memalign)(void **ptr, size_t alignment, size_t size) = NULL;
static void *(*__realloc)(void *ptr, size_t size) = NULL;
static void  (*__free)(const void *addr) = NULL;

#define NUM_CALLERS 32
typedef struct alloc_event {
	size_t num_callers;
	void *callers[NUM_CALLERS];
	void *ptr;
	size_t size;
	TAILQ_ENTRY(alloc_event) tailq;
} alloc_event;
#define MY_QUEUE_SIZE 1000000
static alloc_event myq[MY_QUEUE_SIZE];
static int myq_adds = 0;
static int myq_dels = 0;
static uint64_t total_bytes_allocated = 0;
static uint64_t total_bytes_freed = 0;
pthread_mutex_t myq_mutex;

/* This is the definition of the alloc_event_queue data type. */
TAILQ_HEAD(alloc_event_queue, alloc_event);

/* These are the declarations of the alloc_event_queue objects. */
static struct alloc_event_queue free_event_queue;

#define NUM_USED_EVENT_QUEUE_BUCKETS 256
#define BUCKET_MASK 0xff
static struct alloc_event_queue used_event_queue[NUM_USED_EVENT_QUEUE_BUCKETS];

static void myq_print(void) {
	int num_entries = 0;
	doingPrint = 1;
	const char *separator = "=========================================================";

	int fd = open(LOG_FILE, O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (fd == -1) {
		MEM_HOOK_LOGGER("Unable to open log file (%s).\n", LOG_FILE);
		return;
	}
	dprintf(fd, "%s\n", separator);

	dprintf(fd, "myq_adds %d (%ld) : myq_dels %d (%ld) : diff %d (%ld).\n",
		myq_adds, total_bytes_allocated, myq_dels, total_bytes_freed,
		myq_adds - myq_dels, total_bytes_allocated - total_bytes_freed);
	pthread_mutex_lock(&myq_mutex);

	size_t num_alloced = 0;
	int i;
	for (i = 0; i < NUM_USED_EVENT_QUEUE_BUCKETS; i++) {
		struct alloc_event *entry;
		TAILQ_FOREACH(entry, &used_event_queue[i], tailq) {
			char callerList[NUM_CALLERS * 32];

			num_entries++;
			num_alloced += entry->size;
			memset(callerList, 0, sizeof(callerList));
			int x;
			for (x = 0; (x < NUM_CALLERS) && (x < entry->num_callers); x++) {
				char oneCaller[32];
				sprintf(oneCaller, "%p ", entry->callers[x]);
				strcat(callerList, oneCaller);
			}
			dprintf(fd, "Ptr %p: Size %ld: Callers (%ld) (%s).\n",
				    entry->ptr, entry->size, entry->num_callers, callerList);
		}
	}

	pthread_mutex_unlock(&myq_mutex);
	dprintf(fd, "num_entries %d.  num_alloced %ld.\n", num_entries, num_alloced);
	dprintf(fd, "%s\n", separator);
	close(fd);

	doingPrint = 0;
}

static void myq_add(void *ptr, size_t size) {
	if (doTheQueue) {
		/* Get the backtrace to this call. */
		void *tracePtrs[100];
		int num_callers = backtrace(tracePtrs, 100);
		if ((num_callers == 0) || (num_callers > 100)) {
			MEM_HOOK_LOGGER("SPOTTED AN ERROR (%d) (%m).", num_callers);
		}

		pthread_mutex_lock(&myq_mutex);

		if (TAILQ_EMPTY(&free_event_queue)) {
			MEM_HOOK_LOGGER("Ran out of slots.\n");
		} else {

			struct alloc_event *first_entry = TAILQ_FIRST(&free_event_queue);
			TAILQ_REMOVE(&free_event_queue, first_entry, tailq);

			/* The first 2 callers are our malloc_hook functions.  Skip them. */
			first_entry->num_callers = num_callers - 2;
			int x;
			for (x = 0; (x < NUM_CALLERS) && (x < num_callers); x++) {
				first_entry->callers[x] = tracePtrs[x + 2];
			}
			first_entry->ptr = ptr;
			first_entry->size = size;

			uint64_t bucket = ((uint64_t) ptr >> 4) & BUCKET_MASK;
			TAILQ_INSERT_TAIL(&used_event_queue[bucket], first_entry, tailq);
			myq_adds++;
			total_bytes_allocated += size;
		}

		pthread_mutex_unlock(&myq_mutex);
	}
}

static void myq_del(void *ptr)
{
	if (doTheQueue) {
		pthread_mutex_lock(&myq_mutex);

		int i;
		for (i = 0; i < NUM_USED_EVENT_QUEUE_BUCKETS; i++) {
			struct alloc_event *entry;
			TAILQ_FOREACH(entry, &used_event_queue[i], tailq) {
				if (entry->ptr == ptr) {
					total_bytes_freed += entry->size;

					TAILQ_REMOVE(&used_event_queue[i], entry, tailq);
					TAILQ_INSERT_TAIL(&free_event_queue, entry, tailq);
				}
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
			MEM_HOOK_LOGGER("Start the queue.");
			unlink("/tmp/doq");
			doTheQueue = 1;
		}
		if (stat("/tmp/noq", &s) == 0) {
			MEM_HOOK_LOGGER("Stop the queue.");
			unlink("/tmp/noq");
			doTheQueue = 0;
		}
		if (stat("/tmp/print", &s) == 0) {
			MEM_HOOK_LOGGER("Print the queue.");
			unlink("/tmp/print");
			myq_print();
		}
		if (stat("/tmp/reset", &s) == 0) {
			MEM_HOOK_LOGGER("Reset the queue.");
			unlink("/tmp/reset");
			myq_reset();
		}
	}

	return NULL;
}

static void malloc_hooks_init(void)
{
	TAILQ_INIT(&free_event_queue);

	int i;
	for (i = 0; i < NUM_USED_EVENT_QUEUE_BUCKETS; i++) {
		TAILQ_INIT(&used_event_queue[i]);
	}

	for (i = 0; i < MY_QUEUE_SIZE; i++) {
		TAILQ_INSERT_TAIL(&free_event_queue, &myq[i], tailq);
	}

	pthread_t tid;
	int ret = pthread_create(&tid, NULL, malloc_hooks_thread, NULL);
	if (ret) {
		MEM_HOOK_LOGGER("pthread_create() failed (%m).");
	}
}

/* On Linux, calling dlsym() to get the address of the "calloc" function
 * results in calloc() getting called again.  This is an array of buffers that
 * can be used to satisfy calloc() calls until dlsym() returns.
 */
static unsigned char callocBuf[100][65536];
static void *callocBufBeg = &callocBuf[0][0];
static void *callocBufEnd = &callocBuf[99][65535];

void *calloc(size_t number, size_t size)
{
	if (__calloc == NULL) {
		static int called_dlsym = 0;

		/* We need to make sure we only call dlsym() once.  dlsym() is
		 * going to eventually call us again, so we need to make sure we
		 * don't get caught in a loop. */
		if (!called_dlsym) {
			called_dlsym = 1;
			__calloc = dlsym(RTLD_NEXT, "calloc");
		}

		else {
			/* While dlsym() is fetching the address of the "calloc"
			 * function for us, we will handle any interim calls to
			 * calloc() by returning the addresses of some static
			 * arrays. */
			static int index = 0;
			unsigned char *b = &callocBuf[index++][0];

			memset(b, 0, 65536);
			return b;
		}
	}

	void *ptr = __calloc(number, size);

	if (doingPrint == 0) {
		doingPrint = 1;
		myq_add(ptr, size);

		const void *caller = __builtin_return_address(0);
		if (doTheLog) {
			MEM_HOOK_LOGGER("%s(): number %lu: size %lu: caller %p: ptr %p.\n",
					__FUNCTION__, number, size, caller, ptr);
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
			MEM_HOOK_LOGGER("%s(): size %lu: caller %p: ptr %p.\n",
					__FUNCTION__, size, caller, ptr);
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
			MEM_HOOK_LOGGER("%s(): ptr %p (%p): alignment %lu: size %lu: caller %p: ptr %p.\n",
					__FUNCTION__, ptr, *ptr, alignment, size, caller, ptr);
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
			MEM_HOOK_LOGGER("%s(): ptr %p: size %lu: caller %p: new_ptr %p.\n",
					__FUNCTION__, ptr, size, caller, new_ptr);
		}
		doingPrint = 0;
	}

	return new_ptr;
}

void free(void *ptr)
{
	/* If the caller is freeing one of our static buffers, then return. */
	if ((ptr >= callocBufBeg) && (ptr <= callocBufEnd))
		return;

	if (__free == NULL)
		__free = dlsym(RTLD_NEXT, "free");

	__free(ptr);

	if (doingPrint == 0) {
		doingPrint = 1;
		myq_del(ptr);

		const void *caller = __builtin_return_address(0);
		if (doTheLog) {
			MEM_HOOK_LOGGER("%s(): caller %p: ptr %p.\n",
					__FUNCTION__, caller, ptr);
		}
		doingPrint = 0;
	}
}

int main(int argc, char *argv[])
{
	malloc_hooks_init();

	doTheLog = 1;
	doTheQueue = 1;

	void *p1 = NULL, *p2 = NULL, *p3 = NULL, *p4 = NULL;

	MEM_HOOK_LOGGER("Do some alloc tests =====================================\n");
	p1 = malloc(1);
	p2 = calloc(10, 1);
	int rc = posix_memalign(&p3, 64, 1024);
	p4 = realloc(p1, 1024);
	myq_print();

	MEM_HOOK_LOGGER("Do some free tests ======================================\n");
	//free(p1);
	free(p2);
	free(p3);
	free(p4);
	myq_print();

	return 0;
}

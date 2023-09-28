
#define RTLD_NEXT  ((void *) -1l)

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>

static void *(*__calloc)(size_t number, size_t size) = NULL;
static void *(*__malloc)(size_t size) = NULL;
static int   (*__posix_memalign)(void **ptr, size_t alignment, size_t size) = NULL;
static void *(*__realloc)(void *ptr, size_t size) = NULL;
static void  (*__free)(const void *addr) = NULL;

static __thread int doingPrint = 0;

void *calloc(size_t number, size_t size)
{
	void *ptr = __calloc(number, size);

	if (doingPrint == 0) {
		doingPrint = 1;
		const void *caller = __builtin_return_address(0);
		printf("%s(): number %lu: size %lu: caller %p: ptr %p.\n",
		       __FUNCTION__, number, size, caller, ptr);
		doingPrint = 0;
	}

	return ptr;
}

void *malloc(size_t size)
{
	void *ptr = __malloc(size);

	if (doingPrint == 0) {
		doingPrint = 1;
		const void *caller = __builtin_return_address(0);
		printf("%s(): size %lu: caller %p: ptr %p.\n",
		       __FUNCTION__, size, caller, ptr);
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
		const void *caller = __builtin_return_address(0);
		printf("%s(): ptr %p (%p): alignment %lu: size %lu: caller %p: ptr %p.\n",
		       __FUNCTION__, ptr, *ptr, alignment, size, caller, ptr);
		doingPrint = 0;
	}

	return rc;
}

void *realloc(void *ptr, size_t size)
{
	void *new_ptr = __realloc(ptr, size);

	if (doingPrint == 0) {
		doingPrint = 1;
		const void *caller = __builtin_return_address(0);
		printf("%s(): ptr %p: size %lu: caller %p: new_ptr %p.\n",
		       __FUNCTION__, ptr, size, caller, new_ptr);
		doingPrint = 0;
	}

	return new_ptr;
}

void free(void *ptr)
{
	__free(ptr);

	if (doingPrint == 0) {
		doingPrint = 1;
		const void *caller = __builtin_return_address(0);
		printf("%s(): caller %p: ptr %p.\n",
		       __FUNCTION__, caller, ptr);
		doingPrint = 0;
	}
}

static void initHooks(void)
{
	__calloc = dlsym(RTLD_NEXT, "calloc");
	__malloc = dlsym(RTLD_NEXT, "malloc");
	__realloc = dlsym(RTLD_NEXT, "realloc");
	__posix_memalign = dlsym(RTLD_NEXT, "posix_memalign");
	__free   = dlsym(RTLD_NEXT, "free");
}

int main(int argc, char *argv[])
{
	initHooks();

	void *p1 = malloc(1);
	void *p2 = calloc(10, 1);

	void *p3 = malloc(1024);
	void *p4 = realloc(p3, 1024);

	void *p5 = NULL;
	int rc = posix_memalign(&p5, 64, 1024);

	free(p1);
	free(p2);
	free(p4);
	free(p5);

	return 0;
}

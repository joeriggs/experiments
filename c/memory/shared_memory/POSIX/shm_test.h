
#ifndef __SHM_TEST_H__
#define __SHM_TEST_H__

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>

#define SHM_PATH "my_shm"
#define SHM_SIZE 65536
#define SHM_FORK_SIZE (100 * 1024 * 1024)

#endif // __SHM_TEST_H__

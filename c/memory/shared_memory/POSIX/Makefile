
SHM_CLIENT_OBJ = shm_client.o

SHM_SERVER_OBJ = shm_server.o

SHM_FORK_MUTEX_OBJ = shm_fork_mutex.o

SHM_FORK_RWLOCK_OBJ = shm_fork_rwlock.o

SHM_FORK_SEMAPHORE_OBJ = shm_fork_semaphore.o


SHM_CLIENT = shm_client

SHM_SERVER = shm_server

SHM_FORK_MUTEX = shm_fork_mutex

SHM_FORK_RWLOCK = shm_fork_rwlock

SHM_FORK_SEMAPHORE = shm_fork_semaphore


DEBUG ?= 0
ifeq ($(DEBUG), 1)
        DEBUG_FLAGS := -O0 -g -DDEBUG
else
        DEBUG_FLAGS := -Os
endif

%.o: %.c
	gcc $(DEBUG_FLAGS) -Wall -Werror -c -o $@ $<

dummy: $(SHM_CLIENT) $(SHM_SERVER) $(SHM_FORK_MUTEX) $(SHM_FORK_RWLOCK) $(SHM_FORK_SEMAPHORE)

$(SHM_CLIENT): $(SHM_CLIENT_OBJ) shm_test.h
	gcc -o $@ $< -lrt

$(SHM_SERVER): $(SHM_SERVER_OBJ) shm_test.h
	gcc -o $@ $< -lrt

$(SHM_FORK_MUTEX): $(SHM_FORK_MUTEX_OBJ) shm_test.h
	gcc -o $@ $< -lrt

$(SHM_FORK_RWLOCK): $(SHM_FORK_RWLOCK_OBJ) shm_test.h
	gcc -o $@ $< -lrt

$(SHM_FORK_SEMAPHORE): $(SHM_FORK_SEMAPHORE_OBJ) shm_test.h
	gcc -o $@ $< -lrt

clean:
	rm -f $(SHM_CLIENT) \
	      $(SHM_SERVER) \
	      $(SHM_FORK_MUTEX) \
	      $(SHM_FORK_RWLOCK) \
	      $(SHM_FORK_SEMAPHORE) \
	      $(SHM_CLIENT_OBJ) \
	      $(SHM_SERVER_OBJ) \
	      $(SHM_FORK_MUTEX_OBJ) \
	      $(SHM_FORK_RWLOCK_OBJ) \
	      $(SHM_FORK_SEMAPHORE_OBJ)


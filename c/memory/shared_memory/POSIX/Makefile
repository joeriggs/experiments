
SHM_CLIENT_OBJ = shm_client.o

SHM_SERVER_OBJ = shm_server.o

SHM_FORK_OBJ = shm_fork.o

SHM_CLIENT = shm_client

SHM_SERVER = shm_server

SHM_FORK = shm_fork

DEBUG ?= 0
ifeq ($(DEBUG), 1)
        DEBUG_FLAGS := -O0 -g -DDEBUG
else
        DEBUG_FLAGS := -Os
endif

%.o: %.c
	gcc $(DEBUG_FLAGS) -Wall -Werror -c -o $@ $<

dummy: $(SHM_CLIENT) $(SHM_SERVER) $(SHM_FORK)

$(SHM_CLIENT): $(SHM_CLIENT_OBJ) shm_test.h
	gcc -o $@ $< -lrt

$(SHM_SERVER): $(SHM_SERVER_OBJ) shm_test.h
	gcc -o $@ $< -lrt

$(SHM_FORK): $(SHM_FORK_OBJ) shm_test.h
	gcc -o $@ $< -lrt

clean:
	rm -f $(SHM_CLIENT) $(SHM_SERVER) $(SHM_FORK) \
	      $(SHM_CLIENT_OBJ) $(SHM_SERVER_OBJ) $(SHM_FORK_OBJ)



OS=$(shell uname -s)
ifeq ($(OS),FreeBSD)
	RELEASE=-D__FREEBSD__
endif

SHM_CLIENT = client

SHM_SERVER = server

DEBUG_FLAGS := -O0 -g -DDEBUG

all: $(SHM_CLIENT) $(SHM_SERVER)

$(SHM_CLIENT): client.c shm_ioctl.c
	cc $(DEBUG_FLAGS) -Wall -Werror $(RELEASE) -o $@ client.c shm_ioctl.c -lrt -lpthread -lcurses

$(SHM_SERVER): server.c shm_ioctl.c
	cc $(DEBUG_FLAGS) -Wall -Werror $(RELEASE) -o $@ server.c shm_ioctl.c -lrt -lpthread -lcurses

clean:
	rm -f $(SHM_CLIENT) $(SHM_SERVER)


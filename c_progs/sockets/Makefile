
SOCKET_CLIENT_OBJ = socket_client.o

SOCKET_SERVER_OBJ = socket_server.o

SOCKET_CLIENT = socket_client

SOCKET_SERVER = socket_server

DEBUG ?= 0
ifeq ($(DEBUG), 1)
        DEBUG_FLAGS := -O0 -g -DDEBUG
else
        DEBUG_FLAGS := -Os
endif

%.o: %.c
	gcc $(DEBUG_FLAGS) -Wall -Werror -c -o $@ $<

dummy: $(SOCKET_CLIENT) $(SOCKET_SERVER)

$(SOCKET_CLIENT): $(SOCKET_CLIENT_OBJ)
	gcc -o $@ $< -lrt

$(SOCKET_SERVER): $(SOCKET_SERVER_OBJ)
	gcc -o $@ $< -lrt

clean:
	rm -f $(SOCKET_CLIENT) $(SOCKET_SERVER) $(SOCKET_CLIENT_OBJ) $(SOCKET_SERVER_OBJ)


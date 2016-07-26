
SOCKET_CLIENT_OBJ = client.o

SOCKET_SERVER_OBJ = server.o

SOCKET_CLIENT = client

SOCKET_SERVER = server

DEBUG ?= 0
ifeq ($(DEBUG), 1)
        DEBUG_FLAGS := -O0 -g -DDEBUG
else
        DEBUG_FLAGS := -Os
endif

%.o: %.c
	gcc $(DEBUG_FLAGS) -Wall -Werror -I .. -c -o $@ $<

dummy: $(SOCKET_CLIENT) $(SOCKET_SERVER)

$(SOCKET_CLIENT): $(SOCKET_CLIENT_OBJ)
	gcc -o $@ $< -lrt

$(SOCKET_SERVER): $(SOCKET_SERVER_OBJ)
	gcc -o $@ $< -lrt

clean:
	rm -f $(SOCKET_CLIENT) $(SOCKET_SERVER) $(SOCKET_CLIENT_OBJ) $(SOCKET_SERVER_OBJ)


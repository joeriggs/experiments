
OBJ := breakpoint.o

TARGET := breakpoint

DEBUG ?= 0
ifeq ($(DEBUG), 1)
        DEBUG_FLAGS := -O0 -g -DDEBUG
else
        DEBUG_FLAGS := -Os
endif

%.o: %.c
	gcc $(DEBUG_FLAGS) -DTEST -O0 -g -Wall -Werror -c -o $@ $<

$(TARGET): $(OBJ)
	gcc -o $(TARGET) $(OBJ) -lrt

clean:
	rm -f $(OBJ) $(TARGET)


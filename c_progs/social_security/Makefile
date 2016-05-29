
OBJ := main.o ssa.o

TARGET := ssa

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	DEBUG_FLAGS := -O0 -g -DDEBUG
else
	DEBUG_FLAGS := -Os
endif

TEST ?= 0
ifeq ($(TEST), 1)
	TEST_FLAGS := -DTEST
	OBJ += ssa_test.o
else
	TEST_FLAGS :=
	OBJ += ssa_data.o
endif

%.o: %.c
	gcc $(DEBUG_FLAGS) $(TEST_FLAGS) -Wall -Werror -c -o $@ $<

$(TARGET): $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET)


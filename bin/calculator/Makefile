
OBJ = main.o calculator.o fp_exponent.o list.o operand.o operator.o raw_stdin.o stack.o

TARGET = calculator

DEBUG ?= 0
ifeq ($(DEBUG), 1)
        DEBUG_FLAGS := -O0 -g -DDEBUG
else
        DEBUG_FLAGS := -Os
endif

%.o: %.c
	gcc $(DEBUG_FLAGS) -Wall -Werror -c -o $@ $<

$(TARGET): $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET)


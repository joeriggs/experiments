
################################################################################
# Makefile for building the Crypto Algorithms program.
################################################################################

# Set this variable to 1 if you want to use the Big Number simulation.
BIG_NUMBER_SIMULATION := 1

TARGET := crypto_algs

ifeq ($(BIG_NUMBER_SIMULATION), 1)
BIG_NUMBER_SRC := big_number_sim.c
else
BIG_NUMBER_SRC := big_number.c
endif

SRCS := $(BIG_NUMBER_SRC)  \
        diffie_hellman.c   \
        main.c             \
        prime_numbers.c    \
        rsa.c

OBJS := $(SRCS:.c=.o)

DEBUG ?= 0
ifeq ($(DEBUG), 1)
        DEBUG_FLAGS := -O0 -g -DDEBUG
else
        DEBUG_FLAGS := -Os
endif

%.o: %.c
	gcc $(DEBUG_FLAGS) -Wall -Werror -c -o $@ $<

$(TARGET): $(OBJS)
	gcc -o $(TARGET) $(OBJS) -l pthread

clean:
	rm -f $(TARGET) $(OBJS)


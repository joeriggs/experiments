
################################################################################
# Makefile for building the Crypto Algorithms program.
################################################################################

TARGET := crypto_algs

BIG_NUMBER_SIMULATION ?= 0
ifeq ($(BIG_NUMBER_SIMULATION), 1)
BIG_NUMBER_SRC := big_number_base_test.c
else
BIG_NUMBER_SRC := big_number_base_full.c
endif

SRCS := big_number.c       \
        $(BIG_NUMBER_SRC)  \
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

TEST ?= 0
TEST_FLAGS := 
ifeq ($(TEST), 1)
        TEST_FLAGS := -D TEST
endif

REGRESSION ?= 0
ifeq ($(REGRESSION), 1)
        TEST_FLAGS := -D TEST -D TEST_REGRESSION
endif

%.o: %.c
	gcc $(DEBUG_FLAGS) $(TEST_FLAGS) -Wall -Werror -c -o $@ $<

$(TARGET): $(OBJS)
	gcc -o $(TARGET) $(OBJS) -l pthread

clean:
	rm -f $(TARGET) $(OBJS)


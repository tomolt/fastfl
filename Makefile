CC=gcc
AR=ar
CFLAGS=-g -Og -Wall -Wextra -pedantic -std=c11 -Iinclude
ARFLAGS=rcs

.PHONY: all clean

all: build/libfastfl.a

clean:
	rm -f build/libfastfl.a

build/libfastfl.a: build/fastfl.o
	$(AR) $(ARFLAGS) $@ $^

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -MMD -MP

-include build/fastfl.d

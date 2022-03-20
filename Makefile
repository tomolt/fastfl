CC=gcc
AR=ar
CFLAGS=-g -Og -Wall -Wextra -pedantic -std=c11 -Iinclude
ARFLAGS=rcs

SRCS=src/fastfl.c

OBJS=$(patsubst src/%.c,build/%.o,$(SRCS))
DEPS=$(patsubst src/%.c,build/%.d,$(SRCS))

.PHONY: all clean

all: build/libfastfl.a

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f build/libfastfl.a

build/libfastfl.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -MMD -MP

-include $(DEPS)

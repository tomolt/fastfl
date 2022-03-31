CC=gcc
LD=gcc
AR=ar

CFLAGS=-g -Wall -Wextra -pedantic -std=c11 -Iinclude
LDFLAGS=-g
ARFLAGS=rcs
LIBS=-lm -lpthread

LIB_SRCS=src/graph.c src/layout.c src/sph.c
LIB_OBJS=$(patsubst src/%.c,build/%.o,$(LIB_SRCS))
LIB_DEPS=$(patsubst src/%.c,build/%.d,$(LIB_SRCS))

APP_SRCS=src/fastfl.c src/import.c src/draw.c
APP_OBJS=$(patsubst src/%.c,build/%.o,$(APP_SRCS))
APP_DEPS=$(patsubst src/%.c,build/%.d,$(APP_SRCS))

.PHONY: all clean

all: build/fastfl

clean:
	rm -f $(LIB_OBJS)
	rm -f $(APP_OBJS)
	rm -f $(LIB_DEPS)
	rm -f $(APP_DEPS)
	rm -f build/libfastfl.a
	rm -f build/fastfl

build/fastfl: $(APP_OBJS) build/libfastfl.a
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

build/libfastfl.a: $(LIB_OBJS)
	$(AR) $(ARFLAGS) $@ $^

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@ -MMD -MP

build:
	mkdir -p $@

-include $(LIB_DEPS) $(APP_DEPS)


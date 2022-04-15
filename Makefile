CC=gcc
LD=gcc
AR=ar

CFLAGS=-g -fno-inline -Wall -Wextra -pedantic -std=c11 -Iinclude
LDFLAGS=-g -fno-inline
ARFLAGS=rcs
LIBS=-lm -lpthread

LIB_SRCS=src/graph.c src/layout.c src/sph.c src/repulsion.c src/reduce.c
LIB_OBJS=$(patsubst %.c,build/%.o,$(LIB_SRCS))
LIB_DEPS=$(patsubst %.c,build/%.d,$(LIB_SRCS))

APP_SRCS=src/fastfl.c src/import.c src/draw.c
APP_OBJS=$(patsubst %.c,build/%.o,$(APP_SRCS))
APP_DEPS=$(patsubst %.c,build/%.d,$(APP_SRCS))

TST_SRCS=test/tester.c
TST_OBJS=$(patsubst %.c,build/%.o,$(TST_SRCS))
TST_DEPS=$(patsubst %.c,build/%.d,$(TST_SRCS))

.PHONY: all clean test

all: build/fastfl build/tester

clean:
	rm -f $(LIB_OBJS) $(LIB_DEPS)
	rm -f $(APP_OBJS) $(APP_DEPS)
	rm -f $(TST_OBJS) $(TST_DEPS)
	rm -f build/libfastfl.a
	rm -f build/fastfl
	rm -f build/tester

test: build/tester
	build/tester

build/fastfl: $(APP_OBJS) build/libfastfl.a
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

build/libfastfl.a: $(LIB_OBJS)
	$(AR) $(ARFLAGS) $@ $^

build/tester: $(TST_OBJS) build/libfastfl.a
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

build/src/%.o: src/%.c | build/src
	$(CC) $(CFLAGS) -c $< -o $@ -MMD -MP

build/test/%.o: test/%.c | build/test
	$(CC) $(CFLAGS) -c $< -o $@ -MMD -MP

build/src:
	mkdir -p $@

build/test:
	mkdir -p $@

-include $(LIB_DEPS) $(APP_DEPS) $(TST_DEPS)


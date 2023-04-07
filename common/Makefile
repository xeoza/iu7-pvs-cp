CPPFLAGS += -I./include -I/Users/zhukovdan/git/repos/small/lib
CFLAGS += -std=c99 -D_GNU_SOURCE

ifdef PROFILE
CFLAGS += -g -O0 -fpass-plugin=/Users/zhukovdan/git/repos/small/build/llvm/profiler/SmallProfilerPass.dylib
endif

ifdef MODIFY
CFLAGS += -DMODIFY
endif

SOURCES = $(wildcard ./src/*.c)
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))

TEST_SOURCES = $(wildcard ./tests/*.c)
TEST_OBJECTS = $(patsubst %.c,%.o,$(TEST_SOURCES))

libcommon.a: $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

tests.out: $(TEST_OBJECTS) libcommon.a
	$(CC) $(CFLAGS) -lcunit -o $@ $(TEST_OBJECTS) libcommon.a $(LDLIBS)

tests: test_units test_style test_memory

test_units: tests.out
	./tests.out

test_style:

test_memory: tests.out

clean:
	rm -rf $(TEST_OBJECTS) $(OBJECTS) libcommon.a

.PHONY: clean tests test_units test_style test_memory


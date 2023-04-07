CPPFLAGS += -I./include -I../common/include
CFLAGS += -std=gnu99 -D_GNU_SOURCE
LDFLAGS += -L../common -lpthread -lcommon -lalloc

ifdef PROFILE
CFLAGS += -g -O0 -fpass-plugin=/Users/zhukovdan/git/repos/small/build/llvm/profiler/SmallProfilerPass.dylib
LDFLAGS += -lprofiler
endif

ifdef MODIFY
CFLAGS += -DMODIFY
endif

SOURCES = $(wildcard ./src/*.c)
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))

all: server

server: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -rf $(OBJECTS) server

.PHONY: clean all


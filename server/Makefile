CPPFLAGS += -I./include -I../common/include
CFLAGS += -std=gnu99 -D_GNU_SOURCE -g -O0

SOURCES = $(wildcard ./src/*.c)
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))

all: server

server: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)   -L../common -lpthread -lcommon

clean:
	rm -rf $(OBJECTS) server

.PHONY: clean all


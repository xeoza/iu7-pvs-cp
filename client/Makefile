CC = gcc
CFLAGS = -Wall -ggdb3 -pedantic -g -g3 -O0 -D_GNU_SOURCE -DDEBUG
LDFLAGS= `autoopts-config ldflags` -lm -lrt 
VERSION := -std=c99

INC = -I include -I ../common/include
SRC_DIR = src
OBJ_DIR = obj
LIB = -lconfig -lresolv
TARGET = client

SRC = client.c  client-fsm.c  client_fsm_handlers.c  client-opts.c  dir_worker.c  main.c  parser.c  smtp_conn.c ../../common/src/dir_helper.c  ../../common/src/error.c  ../../common/src/logger.c  ../../common/src/shared_strings.c
OBJECTS = $(patsubst %.o,$(OBJ_DIR)/%.o, $(SRC:.c=.o))

all: client

client: $(OBJECTS)
	$(CC) $(VERSION) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIB)

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(VERSION) $(LDFLAGS) -c -o $@ $< $(CFLAGS) $(INC) $(LIB)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: client clean

CC = gcc
CFLAGS = -Wall -Werror -ggdb3 -pedantic -g -g3 -O0 -D_GNU_SOURCE -DDEBUG
LDFLAGS= `autoopts-config ldflags` -lm -lrt 
VERSION := -std=c99

INC = -I include -I ../common/include
SRC_DIR = src
OBJ_DIR = obj
LIB = -lconfig -lresolv
TARGET = client

SRC = client.c  client-fsm.c  client_fsm_handlers.c  client-opts.c  dir_worker.c  main.c  parser.c  smtp_conn.c ../../common/src/dir_helper.c  ../../common/src/error.c  ../../common/src/logger.c  ../../common/src/shared_strings.c
OBJECTS = $(patsubst %.o,$(OBJ_DIR)/%.o, $(SRC:.c=.o))

all: client docs

client: $(OBJECTS)
	mkdir -p $(OBJ_DIR)
	$(CC) $(VERSION) $(OBJECTS) -o $(TARGET) $(LIB) $(LDFLAGS)

docs: $(OBJECTS)
	cd ./docs && $(MAKE) && cp ./tex/report_client.pdf ../

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(VERSION) $(CFLAGS) -c -o $@ $< $(INC)

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f report_client.pdf && cd ./docs && $(MAKE) clean

.PHONY: client clean

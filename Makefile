CC =clang 
CFLAGS =-Wall -g3 -I inc/ -fsanitize=address

SERVER_PATH :=src/server
SERVER_SOURCES := $(shell find $(SERVER_PATH)  -name "*.c")
SERVER_OBJ := $(patsubst src/%, obj/%, $(patsubst %.c, %.o, $(SERVER_SOURCES)))

CLIENT_PATH :=src/client
CLIENT_SOURCES := $(shell find $(CLIENT_PATH)  -name "*.c")
CLIENT_OBJ := $(patsubst src/%, obj/%, $(patsubst %.c, %.o, $(CLIENT_SOURCES)))

UTILS_PATH :=src/utils
UTILS_SOURCES := $(shell find $(UTILS_PATH)  -name "*.c")
UTILS_OBJ := $(patsubst src/%, obj/%, $(patsubst %.c, %.o, $(UTILS_SOURCES)))

.PHONY: all build_dirs install rmipcs

all: bin/F4Server bin/F4Client

install:
	make build_dirs
	make all

build_dirs:
	mkdir -p obj/client obj/utils obj/server

bin/F4Server: obj/F4Server.o $(SERVER_OBJ) $(UTILS_OBJ)
	$(CC) $^ $(CFLAGS) -o $@

bin/F4Client: obj/F4Client.o $(CLIENT_OBJ) $(UTILS_OBJ)
	$(CC) $^ $(CFLAGS) -o $@

obj/%.o: src/%.c
	$(CC) -c $< $(CFLAGS) -o $@

obj/%/%.o: src/%/%.c
	$(CC) -c $< $(CFLAGS) -o $@

rmipcs:
	ipcs -q  | awk 'NR==4, NR==0 {print $$2}' | awk 'NF > 0' | xargs -L1     ipcrm -q


clean:
	rm -rf bin/* obj/*
	make build_dirs
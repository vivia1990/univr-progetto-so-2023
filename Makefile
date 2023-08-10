CC =gcc -Wall -g3 -I inc/

SERVER_PATH :=src/server
SERVER_SOURCES := $(shell find $(SERVER_PATH)  -name "*.c")
SERVER_OBJ := $(patsubst src/%, obj/%, $(patsubst %.c, %.o, $(SERVER_SOURCES)))

CLIENT_PATH :=src/client
CLIENT_SOURCES := $(shell find $(CLIENT_PATH)  -name "*.c")
CLIENT_OBJ := $(patsubst src/%, obj/%, $(patsubst %.c, %.o, $(CLIENT_SOURCES)))

UTILS_PATH :=src/utils
UTILS_SOURCES := $(shell find $(UTILS_PATH)  -name "*.c")
UTILS_OBJ := $(patsubst src/%, obj/%, $(patsubst %.c, %.o, $(UTILS_SOURCES)))

.PHONY: all build_dirs install

all: bin/F4Server bin/F4Client

install:
	make build_dirs
	make all

build_dirs:
	mkdir -p obj/client obj/utils obj/server

bin/F4Server: obj/F4Server.o $(SERVER_OBJ) $(UTILS_OBJ)
	$(CC) $^ -o $@

bin/F4Client: obj/F4Client.o $(CLIENT_OBJ) $(UTILS_OBJ)
	$(CC) $^ -o $@

obj/%.o: src/%.c
	$(CC) -c $< -o $@

obj/%/%.o: src/%/%.c
	$(CC) -c $< -o $@

clean:
	rm -rf bin/* obj/*
	make build_dirs
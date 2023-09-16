CC =gcc
CFLAGS =-Wall -g3 -I inc/

SERVER_PATH :=src/server
SERVER_SOURCES := $(shell find $(SERVER_PATH)  -name "*.c")
SERVER_OBJ := $(patsubst src/%, obj/%, $(patsubst %.c, %.o, $(SERVER_SOURCES)))

CLIENT_PATH :=src/client
CLIENT_SOURCES := $(shell find $(CLIENT_PATH)  -name "*.c")
CLIENT_OBJ := $(patsubst src/%, obj/%, $(patsubst %.c, %.o, $(CLIENT_SOURCES)))

UTILS_PATH :=src/utils
UTILS_SOURCES := $(shell find $(UTILS_PATH)  -name "*.c")
UTILS_OBJ := $(patsubst src/%, obj/%, $(patsubst %.c, %.o, $(UTILS_SOURCES)))

TEST_SERVER_PATH :=test/src/server
TEST_SERVER_SOURCES := $(shell find $(TEST_SERVER_PATH)  -name "*.c")
TEST_SERVER_OBJ := $(patsubst src/%, obj/%, $(patsubst %.c, %.o, $(TEST_SERVER_SOURCES)))
TEST_SERVER_BIN := $(shell find $(TEST_SERVER_PATH) -type f -printf "%f\n" | awk '{print "test/bin/server/" $$0}' | sed -r 's/\.c//g')

TEST_CLIENT_PATH :=test/src/client
TEST_CLIENT_SOURCES := $(shell find $(TEST_CLIENT_PATH)  -name "*.c")
TEST_CLIENT_OBJ := $(patsubst src/%, obj/%, $(patsubst %.c, %.o, $(TEST_CLIENT_SOURCES)))
TEST_CLIENT_BIN := $(shell find $(TEST_CLIENT_PATH) -type f -printf "%f\n" | awk '{print "test/bin/client/" $$0}' | sed -r 's/\.c//g')

.PHONY: all build_dirs install rmipcs build_test_dirs test/clean clean

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

clean:
	rm -rf bin/* obj/*
	make build_dirs

### test makefile
test/all: test/server test/client

test/server: $(TEST_SERVER_BIN)

### test makefile
test/client: $(TEST_CLIENT_BIN)

test/bin/server/%: test/obj/server/%.o $(SERVER_OBJ) $(UTILS_OBJ)
	$(CC) $^ $(CFLAGS) -o $@

test/bin/client/%: test/obj/client/%.o $(CLIENT_OBJ) $(UTILS_OBJ)
	$(CC) $^ $(CFLAGS) -o $@

test/obj/client/%.o: test/src/client/%.c
	$(CC) -c $< $(CFLAGS) -o $@

test/obj/server/%.o: test/src/server/%.c
	$(CC) -c $< $(CFLAGS) -o $@	

test/clean:
	rm -rf test/bin/* test/obj/*
	make build_test_dirs

build_test_dirs:
	mkdir -p test/obj/client test/obj/server
	mkdir -p test/bin/server test/bin/client

rmipcs:
	ipcs -q  | awk 'NR==4, NR==0 {print $$2}' | awk 'NF > 0' | xargs -L1 ipcrm -q
CC = gcc
CFLAGS = -Wall

SOURCE_FILES := $(wildcard src/*.c)
TEST_SRC := $(filter-out src/main%.c, $(SOURCE_FILES))
EXECUTABLE = bin/test
INCLUDE_DIR = lib

all: build

build: $(SOURCE_FILES)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SOURCE_FILES) -I $(INCLUDE_DIR)

test: test-send test-recv

test-send: $(TEST_SRC) test/test-send.c
	mkdir -p bin
	$(CC) $(CFLAGS) -o bin/test-send test/test-send.c $(TEST_SRC) -I $(INCLUDE_DIR)

test-recv: $(TEST_SRC) test/test-recv.c
	mkdir -p bin
	$(CC) $(CFLAGS) -o bin/test-recv test/test-recv.c $(TEST_SRC) -I $(INCLUDE_DIR)

clean:
	rm -r bin

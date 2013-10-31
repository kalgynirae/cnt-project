CC = gcc
CFLAGS = -Wall

SOURCE_FILES = src/*.c
EXECUTABLE = bin/test
INCLUDE_DIR = lib

all: build

build: $(SOURCE_FILES)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SOURCE_FILES) -I $(INCLUDE_DIR)

clean:
	rm -r bin

CC = gcc
CFLAGS = -Wall

SOURCE_FILES = src/*.c
EXECUTABLE = bin/test.app.sh.tar.jar.gz.exe.bin.yaml
INCLUDE_DIR = lib

all: build

build: $(SOURCE_FILES)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SOURCE_FILES) -I $(INCLUDE_DIR)

clean:
	rm -r bin

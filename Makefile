CC = gcc
CFLAGS = -Wall

SOURCE_FILES = src/main.c
EXECUTABLE = bin/test.app.sh.tar.jar.gz.exe.bin

all: build

build: $(SOURCE_FILES)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SOURCE_FILES)

clean:
	rm -r bin

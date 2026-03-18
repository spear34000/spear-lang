CC ?= gcc
CFLAGS ?= -O3 -Wall -Wextra -std=c11

all: spearc

spearc: src/spearc.c
	$(CC) $(CFLAGS) -o build/spearc.exe src/spearc.c

example: spearc examples/hello.sp
	./build/spearc.exe examples/hello.sp -o build/hello_sp.c
	$(CC) $(CFLAGS) -o build/hello_sp.exe build/hello_sp.c
	./build/hello_sp.exe

clean:
	del /Q build\*.exe 2> NUL
	del /Q build\*.c 2> NUL

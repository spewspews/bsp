CFLAGS=-Wall -Wpedantic -Wextra -O2 -std=c11 -g
CC=clang

avltest: avltest.o

clean:
	rm -f *.o avltest

.PHONY: clean

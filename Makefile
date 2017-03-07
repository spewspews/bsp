CFLAGS=-Wall -Wpedantic -Wextra -O2 -std=c11 -g
CC=clang

avltest: avltest.o

man:
	man -l avl

clean:
	rm -f *.o avltest

.PHONY: clean man

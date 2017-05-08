CFLAGS=-Wall -Wpedantic -Wextra -O2 -std=c11 -g
CC=clang

all: avltest fibheaptest regexptest dijkstra prim hashtest

hashtest.o: ../bsphash.h

fibheaptest.o: ../bspfibheap.h

regexptest.o: ../bspregexp.h

avltest.o: ../bspavl.h

clean:
	rm -f *.o avltest fibheaptest regexptest dijkstra prim hashtest

.PHONY: clean man

CFLAGS=-Wall -Wpedantic -Wextra -O2 -std=c11 -g
CC=clang

all: avltest fibheaptest regexptest dijkstra prim hashtest bitreetest

hashtest.o: ../bsphash.h

prim.o: ../bspfibheap.h

dijkstra.o: ../bspfibheap.h

fibheaptest.o: ../bspfibheap.h

bitreetest.o: ../bspbitree.h

regexptest.o: ../bspregexp.h

avltest.o: ../bspavl.h

clean:
	rm -f *.o avltest fibheaptest regexptest dijkstra prim hashtest bitreetest

.PHONY: clean man

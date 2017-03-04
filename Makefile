CFLAGS=-Wall -Wpedantic -Wextra -O2 -std=c11 -g
CC=clang

libavl.a: avl.o
	$(AR) $(ARFLAGS) $@ $^

avltest: avltest.o libavl.a

clean:
	rm -f *.o libavl.a avltest

.PHONY: clean

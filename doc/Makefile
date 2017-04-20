avl.man: avl.3 avlexample.c
	nroff -Tascii -U -man avl.3 | col -bx > avl.man

fibheap.man: fibheap.3
	nroff -Tascii -U -man fibheap.3 | col -bx > fibheap.man

clean:
	rm -f avl.man avlexample fibheap.man

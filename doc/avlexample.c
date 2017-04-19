#define BSP_AVL_IMPLEMENTATION
#include "../bspavl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Important that the first struct member is
// the Avl node.
typedef struct Node {
	Avl avl;
	char *key;
	double val;
} Node;

int
nodecmp(Avl *a, Avl *b)
{
	Node *m, *n;

	m = (Node*)a;
	n = (Node*)b;
	return strcmp(m->key, n->key);
}

int
main(void)
{
	Avltree t;
	Node *n, m;

	avlinit(&t, nodecmp);
	n = malloc(sizeof(*n));
	n->key = "meaningoflife";
	n->val = 42;
	avlinsert(&t, &n->avl);

	n = malloc(sizeof(*n));
	n->key = "pi";
	n->val = 3.14;
	avlinsert(&t, &n->avl);

	m.key = "meaningoflife";
	n = (Node*)avllookup(&t, &m.avl);
	printf("%s: %g\n", n->key, n->val);
	n->val = 54;
	// Outputs "meaningoflife: 42".

	n = (Node*)avlnext(&n->avl);
	printf("%s: %g\n", n->key, n->val);
	// Outputs "pi: 3.14".

	// There are no more nodes.
	if(avlnext(&n->avl) == NULL)
		printf("No more nodes\n");

	m.key = "meaningoflife";
	n = (Node*)avldelete(&t, &m.avl);
	printf("%s: %g\n", n->key, n->val);
	free(n);
	// Outputs "meaningoflife: 54"

	// A very inefficient update.
	n = malloc(sizeof(*n));
	n->key = "pi";
	n->val = 3.14159;
	n = (Node*)avlinsert(&t, &n->avl);
	printf("%g\n", n->val);
	free(n);
	// We get back the old value 3.14 and
	// new value 3.14159 is inserted into tree.

	exit(0);
}

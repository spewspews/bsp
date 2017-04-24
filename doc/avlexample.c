#define BSP_AVL_IMPLEMENTATION
#include "../bspavl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Important that the first struct member is
// the Avl struct since this makes address of the Avl
// struct equal to  the address of the Node struct.
typedef struct Node {
	Avl avl;
	char *key;
	double val;
} Node;

int
nodecmp(Avl *a, Avl *b)
{
	Node *m, *n;

	// Addresses are the same so conversion is easy.
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
	n = (Node*)avllookup(&t, &m.avl, 0);

	printf("%s: %g\n", n->key, n->val);

	n->val = 54;

	n = (Node*)avlnext(&n->avl);
	printf("%s: %g\n", n->key, n->val);

	if(avlnext(&n->avl) == NULL)
		printf("No more nodes\n");

	m.key = "meaningoflife";
	n = (Node*)avldelete(&t, &m.avl);

	printf("%s: %g\n", n->key, n->val);
	free(n);

	n = malloc(sizeof(*n));

	n->key = "pi";
	n->val = 3.14159;
	n = (Node*)avlinsert(&t, &n->avl);

	printf("old node: %s: %g\n", n->key, n->val);
	free(n);

	m.key = "pi";
	n = (Node*)avllookup(&t, &m.avl, 0);

	printf("new node: %s: %g\n", n->key, n->val);
	
	exit(0);
}
// Output:
// meaningoflife: 42
// pi: 3.14
// No more nodes
// meaningoflife: 54
// old node: pi: 3.14
// new node: pi: 3.14159

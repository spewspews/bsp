#define BSP_AVL_IMPLEMENTATION
#include "../bspavl.h"

#include <stdlib.h>
#include <string.h>

typedef struct Node {
	Avl avl;
	char *key;
	int val;
} Node;

int
nodecmp(void *a, void *b)
{
	Node *na, *nb;

	na = a;
	nb = b;
	return strcmp(na->key, nb->key);
}

int
update(Avltree *t, char *key, int val)
{
	Node *h, n;

	n.key = key;
	h = avllookup(t, &n.avl);
	if(h != NULL) {
		h->val = val;
		return 1;
	}
	return 0;
}

void
put(Avltree *t, char *key, int val)
{
	Node *h;

	h = malloc(sizeof(*h));
	h->key = key;
	h->val = val;
	h = avlinsert(t, &h->avl);
	if(h != NULL)
		free(h);
}

int
get(Avltree *t, char *key)
{
	Node *h, n;

	n.key = key;
	h = avllookup(t, &n.avl);
	return h == NULL ? -1 : h->val;
}

int
remove(Avltree *t, char *key)
{
	Node *h, n;

	n.key = key;
	h = avldelete(t, &n.avl);
	if(h == NULL)
		return 0;
	free(h);
	return 1;
}

int
main(void)
{
	Avltree t;
	avlinit(&t, nodecmp);
	exit(0);
}

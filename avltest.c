#define _XOPEN_SOURCE
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "avl.h"

typedef struct Int Int;
struct Int {
	Avl a;
	int i;
};

Int pool[100];

#define nelem(x) (sizeof(x)/sizeof(*x))

int
Intcmp(Avl *a, Avl *b)
{
	Int *ai, *bi;

	ai = (Int*)a;
	bi = (Int*)b;
	if(ai->i < bi->i)
		return -1;
	if(ai->i > bi->i)
		return 1;
	return 0;
}

enum {
	randmax = 100
};

int
depth(Avl *n)
{
	int dl, dr, d;

	if(n == NULL)
		return 0;
	dl = depth(n->c[0]);
	dr = depth(n->c[1]);
	d = dl > dr ? dl : dr;
	return d + 1;
}

void
check(Avl *n)
{
	int dl, dr, b;

	dl = depth(n->c[0]);
	dr = depth(n->c[1]);
	b = dr - dl;
	printf("Real balance is %d\n", b);
	assert(b == n->balance);
}

void
checkbalance(Avltree *t)
{
	Avl *n;

	n = t->root;
	if(n == NULL)
		return;

	while(n->c[0] != NULL)
		n = n->c[0];

	while(n != NULL) {
		check(n);
		n = avlnext(n);
	}
}

int
main(void)
{
	Avltree *t;
	Avl *n;
	Int *ip;

	srand48(time(NULL));

	for(ip = pool; ip < pool+nelem(pool); ip++) {
		ip->i = drand48()*randmax;
		printf("Inserting %d\n", ip->i);
	}

	t = avlcreate(Intcmp);
	for(ip = pool; ip < pool+nelem(pool); ip++)
		avlinsert(t, &ip->a);

	n = t->root;
	while(n->c[0] != NULL)
		n = n->c[0];

	printf("Sorted:\n");
	ip = (Int*)n;
	while(ip != NULL) {
		printf("Val is %d\n", ip->i);
		ip = (Int*)avlnext((Avl*)ip);
	}

	printf("Balance check:\n");
	checkbalance(t);

	exit(0);
}

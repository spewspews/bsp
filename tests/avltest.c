#define _XOPEN_SOURCE
#define BSP_AVL_IMPLEMENTATION
#include "../bspavl.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
	printf("Actual balance is %d\n", b);
	assert(b == n->b);
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
	Avltree t;
	Int *ip, d;
	int i;

	srand48(time(NULL));

	for(ip = pool; ip < pool+nelem(pool); ip++) {
		ip->i = drand48()*randmax;
		printf("Inserting %d\n", ip->i);
	}

	avlinit(&t, Intcmp);
	for(ip = pool; ip < pool+nelem(pool); ip++)
		avlinsert(&t, &ip->a);

	printf("Sorted:\n");
	for(ip = (Int*)avlmin(&t); ip != NULL; ip = (Int*)avlnext(&ip->a))
		printf("Val is %d\n", ip->i);

	printf("Balance check:\n");
	checkbalance(&t);

	for(i = 0; i < 50; i++) {
		d.i = drand48()*randmax;
		printf("Deleting %d\n", d.i);
		if(avldelete(&t, &d.a) != NULL)
			printf("\tDeleted %d\n", d.i);
	}

	printf("Sorted:\n");
	for(ip = (Int*)avlmin(&t); ip != NULL; ip = (Int*)avlnext(&ip->a))
		printf("Val is %d\n", ip->i);
	

	printf("Balance check:\n");
	checkbalance(&t);

	exit(0);
}

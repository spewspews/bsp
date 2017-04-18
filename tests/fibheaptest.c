#define _XOPEN_SOURCE

#define BSP_FIBHEAP_IMPLEMENTATION
#include "../bspfibheap.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Int Int;
struct Int {
	Fibnode f;
	int i;
};

int
intcmp(void *x, void *y)
{
	Int *s, *t;

	s = x;
	t = y;

	if(s->i < t->i)
		return -1;
	if(s->i > t->i)
		return 1;
	return 0;
}

enum {
	POOLSIZ = 100,
	RANDSIZ = 500,
};

int
main(void)
{
	Fibheap fh;
	Int pool[POOLSIZ], *ip;

	srand48(time(NULL));

	fibinit(&fh, intcmp);
	for(ip = pool; ip < pool+POOLSIZ; ip++) {
		ip->i = drand48()*RANDSIZ;
		printf("Adding %d %p\n", ip->i, (void*)ip);
		fibinsert(&fh, &ip->f);
	}

	while(fh.min != NULL) {
		ip = (Int*)fh.min;
		printf("Heap sorted %d %p\n", ip->i, (void*)ip);
		if(fibdeletemin(&fh) < 0)
			exit(1);
	}

	printf("\nDecrease key test\n");

	fibinit(&fh, intcmp);
	for(ip = pool; ip < pool+10; ip++) {
		ip->i = ip-pool + 10;
		fibinsert(&fh, &ip->f);
	}

	ip = pool+4;
	ip->i = 4;
	fibdecreasekey(&fh, &ip->f);

	ip = (Int*)fh.min;
	printf("%d %p\n", ip->i, (void*)ip);
	fibdeletemin(&fh);

	ip = pool+7;
	ip->i = 7;
	fibdecreasekey(&fh, &ip->f);
	ip = pool+6;
	ip->i = 6;
	fibdecreasekey(&fh, &ip->f);

	while(fh.min != NULL) {
		ip = (Int*)fh.min;
		printf("%d %p\n", ip->i, (void*)ip);
		fibdeletemin(&fh);
	}

	exit(0);
}

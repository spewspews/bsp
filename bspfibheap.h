/*
Copyright (c) 2017 Benjamin Scher Purcell <benjapurcell@gmail.com>
and is licensed for use under the terms found at
https://github.com/spewspews/bsp/blob/master/LICENSE

This is a fibonacci heap implementation with dependencies on an ANSI C
compatible calloc and free routines.

Simply '#include "bspfibheap.h"' anywhere you wish to use it
and in only one file do this:
	#define BSP_FIBHEAP_IMPLEMENTATION
before the '#include "bspfibheap.h"' line.

If you would like the implementation to remain static to one compilation
unit also do this:
	#define BSP_FIBHEAP_STATIC
before the '#include "bspfibheap.h"' line.

If you would like to provide your own calloc routine do this:
	#define BSP_FIBHEAP_CALLOC mycalloc
before the '#include "bspfibheap.h"' line.

If you would like to provide your own calloc routine do this:
	#define BSP_FIBHEAP_CALLOC myfree
before the '#include "bspfibheap.h"' line.
*/

#ifdef BSP_FIBHEAP_STATIC
#define __BSP_FIBHEAP_SCOPE static
#else
#define __BSP_FIBHEAP_SCOPE
#endif

#ifndef BSP_FIBHEAP_CALLOC
#include <stdlib.h>
#define BSP_FIBHEAP_CALLOC calloc
#endif

#ifndef BSP_FIBHEAP_FREE
#include <stdlib.h>
#define BSP_FIBHEAP_FREE free
#endif

#ifndef __BSP_FIBHEAP_H_INCLUDE
#define __BSP_FIBHEAP_H_INCLUDE

typedef struct Fibheap Fibheap;
typedef struct Fibnode Fibnode;
typedef int (*Fibcmp)(Fibnode*, Fibnode*);

struct Fibheap {
	Fibcmp cmp;
	Fibnode *min;
	Fibnode **arr;
	int arrlen;
};

struct Fibnode {
	Fibnode *p, *c, *next, *prev;
	int rank;
	char mark;
};

__BSP_FIBHEAP_SCOPE Fibheap* fibinit(Fibheap *heap, Fibcmp cmp);
__BSP_FIBHEAP_SCOPE Fibheap *fibcreate(Fibcmp);
__BSP_FIBHEAP_SCOPE Fibheap *fibfree(Fibheap*);
__BSP_FIBHEAP_SCOPE void     fibinsert(Fibheap*, Fibnode*);
__BSP_FIBHEAP_SCOPE int      fibdeletemin(Fibheap*);
__BSP_FIBHEAP_SCOPE void     fibdecreasekey(Fibheap*, Fibnode*);
__BSP_FIBHEAP_SCOPE int      fibdelete(Fibheap*, Fibnode*);

#endif // __BSP_AVL_H_INCLUDE

#ifdef BSP_FIBHEAP_IMPLEMENTATION

#include <string.h>

__BSP_FIBHEAP_SCOPE
Fibheap*
fibinit(Fibheap *heap, Fibcmp cmp)
{
	heap->arr = BSP_FIBHEAP_CALLOC(10, sizeof(*heap->arr));
	if(heap->arr == NULL) {
		return NULL;
	}
	heap->arrlen = 10;

	heap->cmp = cmp;
	heap->min = NULL;

	return heap;
}

__BSP_FIBHEAP_SCOPE
Fibheap*
fibfree(Fibheap *h)
{
	BSP_FIBHEAP_FREE(h->arr);
	return h;
}

static Fibnode*
concat(Fibnode *h1, Fibnode *h2)
{
	Fibnode *prev;

	if(h1 == NULL)
		return h2;
	if(h2 == NULL)
		return h1;

	h1->prev->next = h2;
	h2->prev->next = h1;

	prev = h1->prev;
	h1->prev = h2->prev;
	h2->prev = prev;
	return h1;
}

static Fibnode*
meld(Fibnode *h1, Fibnode *h2, Fibcmp cmp)
{
	if(h1 == NULL)
		return h2;
	if(h2 == NULL)
		return h1;

	concat(h1, h2);
	return cmp(h1, h2) <= 0 ? h1 : h2;
}

static Fibnode*
initnode(Fibnode *n)
{
	n->p = NULL;
	n->c = NULL;
	n->next = n;
	n->prev = n;
	n->rank = 0;
	n->mark = 0;
	return n;
}

__BSP_FIBHEAP_SCOPE
void
fibinsert(Fibheap *h, Fibnode *n)
{
	h->min = meld(h->min, initnode(n), h->cmp);
}

static Fibnode*
link1(Fibnode *x, Fibnode *y)
{
	x->c = concat(x->c, y);
	y->p = x;
	y->mark = 0;
	x->rank += y->rank + 1;
	return x;
}

static Fibnode*
link(Fibnode *x, Fibnode *y, Fibcmp cmp)
{
	if(cmp(x, y) <= 0)
		return link1(x, y);
	else
		return link1(y, x);
}

static int
resizearr(Fibheap *h, int rank)
{
	Fibnode **a;
	int alen;

	a = h->arr;
	alen = h->arrlen;
	h->arrlen = 2 * rank * sizeof(*h->arr);
	h->arr = BSP_FIBHEAP_CALLOC(h->arrlen, sizeof(*h->arr));
	if(h->arr == NULL)
		return -1;
	memcpy(h->arr, a, alen*sizeof(*h->arr));
	return 0;
}

static int
arraylink(Fibheap *h, Fibnode *n)
{
	Fibnode *x;

	for(;;) {
		if(h->arrlen <= n->rank) {
			if(resizearr(h, n->rank) == -1)
				return -1;
		}

		x = h->arr[n->rank];
		if(x == NULL) {
			h->arr[n->rank] = n;
			return n->rank;
		}
		h->arr[n->rank] = NULL;
		n = link(x, n, h->cmp);
	}
}

static int
linkheaps(Fibheap *h, Fibnode *head)
{
	Fibnode *n, *next;
	int rank, maxrank;

	memset(h->arr, 0, sizeof(*h->arr) * h->arrlen);

	maxrank = 0;
	n = head;
	do {
		next = n->next;
		n->next = n;
		n->prev = n;
		rank = arraylink(h, n);
		if(rank == -1)
			return -1;
		if(maxrank < rank)
			maxrank = rank;
		n = next;
	} while(n != head);

	return maxrank;
}

static void
meldheaps(Fibheap *h, int maxrank)
{
	Fibnode **ni;

	h->min = NULL;
	for(ni = h->arr; ni <= h->arr + maxrank; ni++) {
		if(*ni != NULL)
			h->min = meld(h->min, *ni, h->cmp);
	}
}

static int
linkstep(Fibheap *h, Fibnode *head)
{
	int maxrank;

	maxrank = linkheaps(h, head);
	if(maxrank == -1)
		return -1;
	meldheaps(h, maxrank);
	return 0;
}

static Fibnode*
removenode(Fibnode *n)
{
	Fibnode *next;

	n->p = NULL;
	if(n->next == n)
		return NULL;

	next = n->next;
	n->next->prev = n->prev;
	n->prev->next = n->next;

	n->next = n;
	n->prev = n;

	return next;
}

static Fibnode*
detachchildren(Fibnode *p)
{
	Fibnode *c;

	c = p->c;
	if(c != NULL) do {
		c->p = NULL;
		c = c->next;
	} while(c != p->c);
	p->c = NULL;
	return c;
}

__BSP_FIBHEAP_SCOPE
int
fibdeletemin(Fibheap *h)
{
	Fibnode *head, *min;

	min = h->min;
	if(min == NULL)
		return 0;

	head = concat(removenode(min), detachchildren(min));
	if(head == NULL) {
		h->min = NULL;
		return 0;
	}

	return linkstep(h, head);
}

static void
cut(Fibheap *h, Fibnode *n)
{
	Fibnode *p;

	p = n->p;
	p->rank -= n->rank + 1;
	p->c = removenode(n);
	h->min = meld(h->min, n, h->cmp);
}

static void
cascadingcut(Fibheap *h, Fibnode *n)
{
	Fibnode *p;

Loop:
	p = n->p;
	cut(h, n);
	if(p->p == NULL)
		return;

	if(p->mark) {
		n = p;
		goto Loop;
	}

	p->mark = 1;
}

__BSP_FIBHEAP_SCOPE
void
fibdecreasekey(Fibheap *h, Fibnode *n)
{
	if(n->p == NULL) {
		h->min = h->cmp(h->min, n) <= 0 ? h->min : n;
		return;
	}

	if(h->cmp(n->p, n) < 0)
		return;

	cascadingcut(h, n);
}

__BSP_FIBHEAP_SCOPE
int
fibdelete(Fibheap *h, Fibnode *n)
{
	if(h->min == n)
		return fibdeletemin(h);

	if(n->p != NULL)
		cascadingcut(h, n);

	removenode(n);
	concat(h->min, detachchildren(n));
	return 0;
}

#endif // BSP_FIBHEAP_IMPLEMENTATION

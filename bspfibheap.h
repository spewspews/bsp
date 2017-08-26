/*
Copyright (c) 2017 Benjamin Scher Purcell <benjapurcell@gmail.com>
and is licensed for use under the terms found at
https://github.com/spewspews/bsp/blob/master/LICENSE

This is a fibonacci heap implementation with dependencies on an ANSI C
compatible calloc and free routines.

Do this:
	#define BSP_FIBHEAP_IMPLEMENTATION
before you include this file in *one* C file to create the implementation.

// i.e. it should look like this:
#include ...
#include ...
#include ...
#define BSP_FIBHEAP_IMPLEMENTATION
#include "bspfibheap.h"

You can #define BSP_FIBHEAP_STATIC before the #include to keep everything
private to one compilation unit. And #define BSP_FIBHEAP_CALLOC, and
BSP_FIBHEAP_FREE to avoid using using calloc, and free.


FIBHEAP(3)                 Library Functions Manual                 FIBHEAP(3)



NAME
       fibinit  fibcreate  fibfree  fibinsert fibdeletemin fibdecreasekey fib-
       delete - Fibonacci heap routines

SYNOPSIS
       typedef struct Fibheap Fibheap;
       typedef struct Fibnode Fibnode;
       typedef int (*Fibcmp)(Fibnode*, Fibnode*);

       struct Fibheap {
              Fibnode *min;
              Fibcmp cmp;
              Fibnode **arr;
              int arrlen;
       };

       struct Fibnode {
              Fibnode *p, *c, *next, *prev;
              int rank;
              char mark;
       };

       Fibheap* fibinit(Fibheap *heap, Fibcmp cmp);
       Fibheap *fibcreate(Fibcmp cmp);
       Fibheap *fibfree(Fibheap *heap);
       void     fibinsert(Fibheap *heap, Fibnode *node);
       int      fibdeletemin(Fibheap *node);
       void     fibdecreasekey(Fibheap *heap, Fibnode *node);
       int      fibdelete(Fibheap *heap, Fibnode *node);


DESCRIPTION
       These routines allow creation and  maintenance  of  in-memory  priority
       queues implemented by a Fibonacci heap.

       The  intended  usage  is  for a parent structure to contain the Fibnode
       structure as its first member along with other data to be stored in the
       tree.  A  pointer  to  the  Fibnode member is passed to the library API
       functions. The API functions then pass these pointers to the comparison
       function  and store them in the heap. See the example below for details
       on how this works in practice.

       A heap is initialized by calling fibheapinit with an empty tree  and  a
       comparison function as arguments.  The comparison function receives two
       pointers to Fibnodes stored in the heap and should  return  an  integer
       less  than,  equal  to,  or greater than 0 as the first is respectively
       ordered less than, equal to, or greater than the  second.   Fibheapinit
       allocates  memory  used for maintenance of the heap and fibfree must be
       called to free the memory.  A new empty heap can be created by  calling
       fibcreate  with  a  comparison  function  as an argument. This function
       calls malloc (see malloc(3)) to create the tree and must be freed after
       fibfree has been called.

       The minimum element in the heap is stored in the min member of the Fib-
       heap struct. If min is NULL  then  the  heap  is  empty.   Fibdeletemin
       removes  the  minimum element from the heap and queues the next item in
       the min field. This may require a memory allocation and will return  -1
       in  case  of failure.  Keys in a node can be changed as long as the new
       value is not greater than the old value. In  that  case  Fibdecreasekey
       must be called to re-establish heap order on the heap.  Any node can be
       removed from the heap by calling fibdelete.  In the case where the node
       is  the  minimum node, allocation may occur and the function returns -1
       in case of failure.

EXAMPLES
       Typical usage is to embed the Fibnode structure as the first member  of
       a  structure  that  holds  data  to be stored in the tree.  Then pass a
       pointer to this member to the library functions.

              #define BSP_FIBHEAP_IMPLEMENTATION
              #include "../bspfibheap.h"

              #include <stdio.h>
              #include <stdlib.h>

              typedef struct Int Int;
              struct Int {
                     Fibnode fib;
                     int i;
              };

              int
              intcmp(Fibnode *x, Fibnode *y)
              {
                     Int *s, *t;

                     s = (Int*)x;
                     t = (Int*)y;

                     if(s->i < t->i)
                             return -1;
                     if(s->i > t->i)
                             return 1;
                     return 0;
              }

              int
              main(void)
              {
                     Fibheap heap;
                     Int *node;

                     fibinit(&heap, intcmp);

                     // Insert and delete min.
                     node = malloc(sizeof(*node));
                     node->i = 10;
                     fibinsert(&heap, &node->fib);

                     node = malloc(sizeof(*node));
                     node->i = 5;
                     fibinsert(&heap, &node->fib);

                     node = malloc(sizeof(*node));
                     node->i = 15;
                     fibinsert(&heap, &node->fib);

                     while(heap.min != NULL) {
                             node = (Int*)heap.min;
                             printf("%d\n", node->i);
                             fibdeletemin(&heap);
                             free(node);
                     }

                     // Decrease key.
                     node = malloc(sizeof(*node));
                     node->i = 10;
                     fibinsert(&heap, &node->fib);

                     node = malloc(sizeof(*node));
                     node->i = 15;
                     fibinsert(&heap, &node->fib);

                     node = (Int*)heap.min;
                     printf("%d\n", node->i);

                     node->i = 5;
                     fibdecreasekey(&heap, &node->fib);

                     node = (Int*)heap.min;
                     printf("%d\n", node->i);
              }
              // Output:
              // 5
              // 10
              // 15
              // 10
              // 5

SEE ALSO
       Michael L. Fredman and Robert Endre Tarjan. 1987. Fibonacci heaps and their uses in improved network optimization algorithms. J. ACM 34, 3 (July 1987), 596-615. DOI=http://dx.doi.org/10.1145/28869.28874

DIAGNOSTICS
       fibinit, fibdeletemin, and fibdelete returns NULL on error.



                                                                    FIBHEAP(3)
*/

#ifdef BSP_FIBHEAP_STATIC
#define __BSP_FIBHEAP_SCOPE static
#else
#define __BSP_FIBHEAP_SCOPE
#endif

#ifndef __BSP_FIBHEAP_H_INCLUDE
#define __BSP_FIBHEAP_H_INCLUDE

#ifdef __cpluscplus
extern "C" {
#endif

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
__BSP_FIBHEAP_SCOPE Fibheap *fibmeld(Fibheap*, Fibheap*);
__BSP_FIBHEAP_SCOPE void     fibinsert(Fibheap*, Fibnode*);
__BSP_FIBHEAP_SCOPE int      fibdeletemin(Fibheap*);
__BSP_FIBHEAP_SCOPE void     fibdecreasekey(Fibheap*, Fibnode*);
__BSP_FIBHEAP_SCOPE int      fibdelete(Fibheap*, Fibnode*);

#ifdef __cpluscplus
}
#endif

#endif // __BSP_FIBHEAP_H_INCLUDE

#ifdef BSP_FIBHEAP_IMPLEMENTATION

#ifndef BSP_FIBHEAP_CALLOC
#include <stdlib.h>
#define BSP_FIBHEAP_CALLOC calloc
#endif

#ifndef BSP_FIBHEAP_FREE
#include <stdlib.h>
#define BSP_FIBHEAP_FREE free
#endif

#include <string.h>

__BSP_FIBHEAP_SCOPE
Fibheap*
fibinit(Fibheap *heap, Fibcmp cmp)
{
	heap->cmp = cmp;
	heap->min = NULL;
	heap->arr = NULL;
	heap->arrlen = 0;

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

	if(h1 == NULL) return h2;
	if(h2 == NULL) return h1;

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
	if(h1 == NULL) return h2;
	if(h2 == NULL) return h1;

	concat(h1, h2);
	return cmp(h1, h2) <= 0 ? h1 : h2;
}

__BSP_FIBHEAP_SCOPE
Fibheap*
fibmeld(Fibheap *h1, Fibheap *h2)
{
	h1->min = meld(h1->min, h2->min, h1->cmp);
	return h1;
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
	if(cmp(x, y) <= 0) return link1(x, y);
	else               return link1(y, x);
}

static int
resizearr(Fibheap *h, int rank)
{
	Fibnode **a;
	int alen;

	alen = 2*rank + 10;
	a = BSP_FIBHEAP_CALLOC(alen, sizeof(*a));
	if(a == NULL) return -1;
	memcpy(a, h->arr, h->arrlen*sizeof(*a));
	free(h->arr);
	h->arr = a;
	h->arrlen = alen;
	return 0;
}

static int
arraylink(Fibheap *h, Fibnode *n)
{
	Fibnode *m;

	for(;;) {
		if(h->arrlen <= n->rank && resizearr(h, n->rank) == -1) return -1;
		m = h->arr[n->rank];
		if(m == NULL) {
			h->arr[n->rank] = n;
			return n->rank;
		}
		h->arr[n->rank] = NULL;
		n = link(m, n, h->cmp);
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
		if(rank == -1) return -1;
		if(maxrank < rank) maxrank = rank;
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
		if(*ni != NULL) h->min = meld(h->min, *ni, h->cmp);
	}
}

static int
linkstep(Fibheap *h, Fibnode *head)
{
	int maxrank;

	maxrank = linkheaps(h, head);
	if(maxrank == -1) return -1;
	meldheaps(h, maxrank);
	return 0;
}

static Fibnode*
removenode(Fibnode *n)
{
	Fibnode *next;

	n->p = NULL;
	if(n->next == n) return NULL;

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
	if(min == NULL) return 0;

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
	if(p->p == NULL) return;

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

	if(h->cmp(n->p, n) < 0) return;

	cascadingcut(h, n);
}

__BSP_FIBHEAP_SCOPE
int
fibdelete(Fibheap *h, Fibnode *n)
{
	if(h->min == n) return fibdeletemin(h);

	if(n->p != NULL) cascadingcut(h, n);

	removenode(n);
	concat(h->min, detachchildren(n));
	return 0;
}

#endif // BSP_FIBHEAP_IMPLEMENTATION

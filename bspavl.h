/*
Copyright (c) 2017 Benjamin Scher Purcell <benjapurcell@gmail.com>
and is licensed for use under the terms found at
https://github.com/spewspews/bsp/blob/master/LICENSE

This is a no memory allocation balanced binary tree with no dependencies.

Do this:
	#define BSP_AVL_IMPLEMENTATION
before you include this file in *one* C file to create the implementation.

// i.e. it should look like this:
#include ...
#include ...
#include ...
#define BSP_AVL_IMPLEMENTATION
#include "bspavl.h"

You can #define BSP_AVL_STATIC before the #include to keep everything
private to one compilation unit.

AVL(3)                     Library Functions Manual                     AVL(3)



NAME
       avlinit, avlcreate, avlinsert, avldelete, avllookup, avlnext, avlprev -
       Balanced binary search tree routines

SYNOPSIS
       #include "spewavl.h"

       typedef struct Avl Avl;
       typedef struct Avltree Avltree;

       struct Avl {
              Avl *c[2];
              Avl *p;
              int8_t b;
       };

       struct Avltree {
              int (*cmp)(void*, void*);
              Avl *root;
       };

       Avltree *avlinit(Avltree*, int(*cmp)(void*, void*));
       Avltree *avlcreate(int(*cmp)(Avl*, Avl*));
       Avl     *avlinsert(Avltree *tree, Avl *new);
       Avl     *avldelete(Avltree *tree, Avl *key);
       Avl     *avllookup(Avltree *tree, Avl *key, int dir);
       Avl     *avlnext(Avl *n);
       Avl     *avlprev(Avl *n);


DESCRIPTION
       These routines allow creation and  maintenance  of  in-memory  balanced
       binary search trees.

       The  intended usage is for a parent structure to contain the Avl struc-
       ture as its first member as well as other data  to  be  stored  in  the
       tree.  A  pointer  to the Avl member is passed to the library API func-
       tions. The API functions then pass the pointers to the comparison func-
       tion  and  return  them  as  return  values.  See the example below for
       details on how this works in practice.

       A tree is initialized by calling avlinit with an empty tree and a  com-
       parison  function  as  arguments.  The comparison function receives two
       pointers to nodes stored in the tree and should return an integer  less
       than,  equal to, or greater than 0 as the first is respectively ordered
       less than, equal to, or greater than the second.  A new empty tree  can
       be  created by calling avlcreate with a comparison function as an argu-
       ment. This function calls malloc (see malloc(3)) and the returned  tree
       should be free after use.

       Avlinsert  adds  a new node to the tree. If avlinsert finds an existing
       node with the same key then that node is  removed  from  the  tree  and
       returned.  Otherwise  avlinsert returns NULL.  Avllookup searches for a
       given key and returns the closest node less than the given  key,  equal
       to,  or  the closest node greater than the key depending on whether dir
       is less than, equal to, or greater than zero, respectively. If  dir  is
       zero  and there is no matching key, it returns NULL.  Avldelete removes
       the node matching the key from the tree and returns it. It returns NULL
       if no matching key is found.

       Avlnext  returns  the next Avl node in an in-order walk of the AVL tree
       and avlprev returns the previous node.

EXAMPLES
       Typical usage is to embed the Avl structure as the first  member  of  a
       structure  that  holds  data  to  be  stored  in the tree.  Then pass a
       pointer to this member to the library functions.

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

SEE ALSO
       Donald Knuth, ``The Art of Computer Programming'', Volume 3. Section 6.2.3

DIAGNOSTICS
       Avlcreate returns NULL on error.

HISTORY
       This implementation was originally written for 9front (Dec, 2016).



                                                                        AVL(3)
*/

#ifdef BSP_AVL_STATIC
#define __BSP_AVL_SCOPE static
#else
#define __BSP_AVL_SCOPE
#endif

#ifndef __BSP_AVL_H_INCLUDE
#define __BSP_AVL_H_INCLUDE

#include <stdint.h>
#include <stdlib.h>

/* See Knuth Volume 3, 6.2.3 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Avl Avl;
typedef struct Avltree Avltree;
typedef int (*Avlcmp)(Avl*, Avl*);

struct Avl {
	Avl *c[2];
	Avl *p;
	int8_t b;
};

struct Avltree {
	Avlcmp cmp;
	Avl *root;
};

__BSP_AVL_SCOPE Avltree *avlcreate(Avlcmp);
__BSP_AVL_SCOPE Avltree *avlinit(Avltree*, Avlcmp);
__BSP_AVL_SCOPE Avl *avllookup(Avltree*, Avl*, int);
__BSP_AVL_SCOPE Avl *avldelete(Avltree*, Avl*);
__BSP_AVL_SCOPE Avl *avlinsert(Avltree*, Avl*);
__BSP_AVL_SCOPE Avl *avlnext(Avl*);
__BSP_AVL_SCOPE Avl *avlprev(Avl*);
__BSP_AVL_SCOPE Avl *avlmin(Avltree*);
__BSP_AVL_SCOPE Avl *avlmax(Avltree*);

#ifdef __cplusplus
}
#endif

#endif // __BSP_AVL_H_INCLUDE

#ifdef BSP_AVL_IMPLEMENTATION

__BSP_AVL_SCOPE
Avltree*
avlcreate(Avlcmp cmp)
{
	Avltree *t;

	t = malloc(sizeof(*t));
	if(t == NULL)
		return NULL;

	t->cmp = cmp;
	t->root = NULL;
	return t;
}

__BSP_AVL_SCOPE
Avltree*
avlinit(Avltree *t, Avlcmp cmp)
{
	if(t == NULL)
		return NULL;

	t->cmp = cmp;
	t->root = NULL;
	return t;
}


__BSP_AVL_SCOPE
Avl*
avllookup(Avltree *t, Avl *k, int d)
{
	Avl *h, *n;
	int c;

	n = NULL;
	h = t->root;
	while(h != NULL){
		c = (t->cmp)(k, h);
		if(c < 0){
			if(d > 0)
				n = h;
			h = h->c[0];
			continue;
		}
		if(c > 0){
			if(d < 0)
				n = h;
			h = h->c[1];
			continue;
		}
		return h;
	}
	return NULL;
}

static int insert(Avlcmp, Avl*, Avl**, Avl*, Avl**);

__BSP_AVL_SCOPE
Avl*
avlinsert(Avltree *t, Avl *k)
{
	Avl *old;

	if(t == NULL)
		return NULL;

	old = NULL;
	insert(t->cmp, NULL, &t->root, k, &old);
	return old;
}

static int insertfix(int, Avl**);

static int
insert(Avlcmp cmp, Avl *p, Avl **qp, Avl *k, Avl **oldp)
{
	Avl *q;
	int fix, c;

	q = *qp;
	if(q == NULL) {
		k->c[0] = NULL;
		k->c[1] = NULL;
		k->b = 0;
		k->p = p;
		*qp = k;
		return 1;
	}

	c = cmp(k, q);
	if(c == 0) {
		*oldp = q;
		*k = *q;
		if(q->c[0] != NULL)
			q->c[0]->p = k;
		if(q->c[1] != NULL)
			q->c[1]->p = k;
		*qp = k;
		return 0;
	}
	c = c > 0 ? 1 : -1;
	fix = insert(cmp, q, q->c + (c+1)/2, k, oldp);
	if(fix)
		return insertfix(c, qp);
	return 0;
}

static Avl *singlerot(int, Avl*);
static Avl *doublerot(int, Avl*);

static int
insertfix(int c, Avl **t)
{
	Avl *s;

	s = *t;
	if(s->b == 0) {
		s->b = c;
		return 1;
	}
	if(s->b == -c) {
		s->b = 0;
		return 0;
	}
	if(s->c[(c+1)/2]->b == c)
		s = singlerot(c, s);
	else
		s = doublerot(c, s);
	*t = s;
	return 0;
}

static int delete(Avlcmp, Avl**, Avl*, Avl**);
static int deletemin(Avl**, Avl**);
static int deletefix(int, Avl**);

__BSP_AVL_SCOPE
Avl*
avldelete(Avltree *t, Avl *k)
{
	Avl *old;

	if(t == NULL)
		return NULL;
	if(t->root == NULL)
		return NULL;

	old = NULL;
	delete(t->cmp, &t->root, k, &old);
	return old;
}

static int
delete(Avlcmp cmp, Avl **qp, Avl *k, Avl **oldp)
{
	Avl *q, *e;
	int c, fix;

	q = *qp;
	if(q == NULL)
		return 0;

	c = cmp(k, q);
	c = c > 0 ? 1 : c < 0 ? -1: 0;
	if(c == 0) {
		*oldp = q;
		if(q->c[1] == NULL) {
			*qp = q->c[0];
			if(*qp != NULL)
				(*qp)->p = q->p;
			return 1;
		}
		fix = deletemin(q->c+1, &e);
		*e = *q;
		if(q->c[0] != NULL)
			q->c[0]->p = e;
		if(q->c[1] != NULL)
			q->c[1]->p = e;
		*qp = e;
		if(fix)
			return deletefix(-1, qp);
		return 0;
	}
	fix = delete(cmp, q->c + (c+1)/2, k, oldp);
	if(fix)
		return deletefix(-c, qp);
	return 0;
}

static int
deletemin(Avl **qp, Avl **oldp)
{
	Avl *q;
	int fix;

	q = *qp;
	if(q->c[0] == NULL) {
		*oldp = q;
		*qp = q->c[1];
		if(*qp != NULL)
			(*qp)->p = q->p;
		return 1;
	}
	fix = deletemin(q->c, oldp);
	if(fix)
		return deletefix(1, qp);
	return 0;
}

static Avl *rotate(int, Avl*);

static int
deletefix(int c, Avl **t)
{
	Avl *s;
	int a;

	s = *t;
	if(s->b == 0) {
		s->b = c;
		return 0;
	}
	if(s->b == -c) {
		s->b = 0;
		return 1;
	}
	a = (c+1)/2;
	if(s->c[a]->b == 0) {
		s = rotate(c, s);
		s->b = -c;
		*t = s;
		return 0;
	}
	if(s->c[a]->b == c)
		s = singlerot(c, s);
	else
		s = doublerot(c, s);
	*t = s;
	return 1;
}

static Avl*
singlerot(int c, Avl *s)
{
	s->b = 0;
	s = rotate(c, s);
	s->b = 0;
	return s;
}

static Avl*
doublerot(int c, Avl *s)
{
	Avl *r, *p;
	int a;

	a = (c+1)/2;
	r = s->c[a];
	s->c[a] = rotate(-c, s->c[a]);
	p = rotate(c, s);

	if(p->b == c) {
		s->b = -c;
		r->b = 0;
	} else if(p->b == -c) {
		s->b = 0;
		r->b = c;
	} else
		s->b = r->b = 0;
	p->b = 0;
	return p;
}

static Avl*
rotate(int c, Avl *s)
{
	Avl *r, *n;
	int a;

	a = (c+1)/2;
	r = s->c[a];
	s->c[a] = n = r->c[a^1];
	if(n != NULL)
		n->p = s;
	r->c[a^1] = s;
	r->p = s->p;
	s->p = r;
	return r;
}

static Avl *walk1(int, Avl*);

__BSP_AVL_SCOPE
Avl*
avlprev(Avl *q)
{
	return walk1(0, q);
}

__BSP_AVL_SCOPE
Avl*
avlnext(Avl *q)
{
	return walk1(1, q);
}

static Avl*
walk1(int a, Avl *q)
{
	Avl *p;

	if(q == NULL)
		return NULL;

	if(q->c[a] != NULL){
		for(q = q->c[a]; q->c[a^1] != NULL; q = q->c[a^1])
			;
		return q;
	}
	for(p = q->p; p != NULL && p->c[a] == q; p = p->p)
		q = p;
	return p;
}

static Avl *bottom(Avltree*,int);

__BSP_AVL_SCOPE
Avl*
avlmin(Avltree *t)
{
	return bottom(t, 0);
}

__BSP_AVL_SCOPE
Avl*
avlmax(Avltree *t)
{
	return bottom(t, 1);
}

static Avl*
bottom(Avltree *t, int d)
{
	Avl *n;

	if(t == NULL)
		return NULL;
	if(t->root == NULL)
		return NULL;

	for(n = t->root; n->c[d] != NULL; n = n->c[d])
		;
	return n;
}


#endif // BSP_AVL_IMPLEMENTATION

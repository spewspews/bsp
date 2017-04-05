/*
Copyright (c) 2017 Benjamin Scher Purcell <benjapurcell@gmail.com>
and is licensed for use under the terms found at
https://github.com/spewspews/spewsh/blob/master/LICENSE

This is a no memory allocation balanced binary tree with no dependencies.
Simply '#include "spewavl.h"' the file anywhere you wish to use it and in one file
do this:
	#define SPEW_AVL_IMPLEMENTATION
before the '#include "spewavl.h"' line.

If you would like the implementation to remain static to one compilation
unit also do this:
	#define SPEW_AVL_STATIC
before the '#include "spewavl.h"' line.

     AVL(3)                                                     AVL(3)

     NAME
          avlinit, avlcreate, avlinsert, avldelete, avllookup,
          avlnext, avlprev - Balanced binary search tree routines

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
          void    *avlinsert(Avltree *tree, Avl *new);
          void    *avldelete(Avltree *tree, Avl *key);
          void    *avllookup(Avltree *tree, Avl *key);
          void    *avlnext(Avl *n);
          void    *avlprev(Avl *n);


     DESCRIPTION
          These routines allow creation and maintenance of in-memory
          balanced binary search trees.

          The intended usage is for a parent structure to contain the
          Avl structure as its first member as well as other data to
          be stored in the tree. A pointer to the Avl member is then
          passed to the library API functions and the API functions
          will then return pointers to the parent structure.  See the
          example below for details.

          A tree is initialized by calling avlinit with an empty tree
          and a comparison function as arguments.  The comparison
          function receives two pointers to nodes stored in the tree
          and should return an integer less than, equal to, or greater
          than 0 as the first is respectively ordered less than, equal
          to, or greater than the second.  A new empty tree can be
          created by calling avlcreate with a comparison function as
          an argument. This function calls malloc (see malloc(3)) and
          the returned tree should be free after use.

          Avlinsert adds a new node to the tree. If avlinsert finds an
          existing node with the same key then that node is removed
          from the tree and returned. Otherwise avlinsert returns
          NULL.  Avllookup returns the node that matches the key or
          NULL if no node matches.  Avldelete removes the node match-
          ing the key from the tree and returns it. It returns NULL if
          no matching key is found.

          Avlnext returns the next Avl node in an in-order walk of the
          AVL tree and avlprev returns the previous node.

     EXAMPLES
          Intended usage is to embed the Avl structure as the first
          member of a parent structure which also contains the data to
          be stored in the tree.  A pointer to the embedded AVL struc-
          ture should then be passed to the library api functions.
          For example, the following is a full implementation of a
          string to integer map.

               #define SPEW_AVL_IMPLEMENTATION
               #include "spewavl.h"

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

     SEE ALSO
          Donald Knuth, ``The Art of Computer Programming'', Volume 3. Section 6.2.3

     DIAGNOSTICS
          Avlcreate returns nil on error.

     HISTORY
          This implementation was originally written for 9front (Dec,
          2016).
*/

#ifdef SPW_AVL_STATIC
#define __SPEW_AVL_SCOPE static
#else
#define __SPEW_AVL_SCOPE
#endif

#ifndef __SPEW_AVL_H_INCLUDE
#define __SPEW_AVL_H_INCLUDE

#include <stdint.h>
#include <stdlib.h>

/* See Knuth Volume 3, 6.2.3 */

#ifdef __cplusplus
extern "C" {
#endif

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

__SPEW_AVL_SCOPE Avltree *avlcreate(int(*cmp)(void*, void*));
__SPEW_AVL_SCOPE Avltree *avlinit(Avltree*, int(*cmp)(void*, void*));
__SPEW_AVL_SCOPE void *avllookup(Avltree*, Avl*);
__SPEW_AVL_SCOPE void *avldelete(Avltree*, Avl*);
__SPEW_AVL_SCOPE void *avlinsert(Avltree*, Avl*);
__SPEW_AVL_SCOPE void *avlnext(Avl*);
__SPEW_AVL_SCOPE void *avlprev(Avl*);

#endif // __SPEW_AVL_H_INCLUDE

#ifdef SPEW_AVL_IMPLEMENTATION

__SPEW_AVL_SCOPE
Avltree*
avlcreate(int (*cmp)(void*, void*))
{
	Avltree *t;

	t = malloc(sizeof(*t));
	if(t == NULL)
		return NULL;

	t->cmp = cmp;
	t->root = NULL;
	return t;
}

__SPEW_AVL_SCOPE
Avltree*
avlinit(Avltree *t, int (*cmp)(void*, void*))
{
	if(t == NULL)
		return NULL;

	t->cmp = cmp;
	t->root = NULL;
	return t;
}


__SPEW_AVL_SCOPE
void*
avllookup(Avltree *t, Avl *k)
{
	Avl *h;
	int c;

	h = t->root;
	while(h != NULL){
		c = (t->cmp)(k, h);
		if(c < 0){
			h = h->c[0];
			continue;
		}
		if(c > 0){
			h = h->c[1];
			continue;
		}
		return h;
	}
	return NULL;
}

static int insert(int (*cmp)(void*, void*), Avl*, Avl**, Avl*, Avl**);

__SPEW_AVL_SCOPE
void*
avlinsert(Avltree *t, Avl *k)
{
	Avl *old;

	old = NULL;
	insert(t->cmp, NULL, &t->root, k, &old);
	return old;
}

static int insertfix(int, Avl**);

static int
insert(int (*cmp)(void*, void*), Avl *p, Avl **qp, Avl *k, Avl **oldp)
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

static int delete(int (*cmp)(void*, void*), Avl**, Avl*, Avl**);
static int deletemin(Avl**, Avl**);
static int deletefix(int, Avl**);

__SPEW_AVL_SCOPE
void*
avldelete(Avltree *t, Avl *k)
{
	Avl *old;

	if(t->root == NULL)
		return NULL;
	old = NULL;
	delete(t->cmp, &t->root, k, &old);
	return old;
}

static int
delete(int (*cmp)(void*, void*), Avl **qp, Avl *k, Avl **oldp)
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

__SPEW_AVL_SCOPE
void*
avlprev(Avl *q)
{
	return walk1(0, q);
}

__SPEW_AVL_SCOPE
void*
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

#endif // SPEW_AVL_IMPLEMENTATION

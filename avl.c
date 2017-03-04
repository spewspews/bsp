#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "avl.h"
/* See Knuth Volume 3, 6.2.3 */

Avltree*
avlcreate(int (*cmp)(Avl*, Avl*))
{
	Avltree *t;

	t = malloc(sizeof(*t));
	if(t == NULL)
		return NULL;

	t->cmp = cmp;
	t->root = NULL;
	return t;
}

Avl*
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

static int insert(int (*)(Avl*, Avl*), Avl*, Avl**, Avl*, Avl**);

Avl*
avlinsert(Avltree *t, Avl *k)
{
	Avl *old;

	old = NULL;
	insert(t->cmp, NULL, &t->root, k, &old);
	return old;
}

static int insertfix(int, Avl**);

static int
insert(int (*cmp)(Avl*, Avl*), Avl *p, Avl **qp, Avl *k, Avl **oldp)
{
	Avl *q;
	int fix, c;

	q = *qp;
	if(q == NULL) {
		k->c[0] = NULL;
		k->c[1] = NULL;
		k->balance = 0;
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
	if(s->balance == 0) {
		s->balance = c;
		return 1;
	}
	if(s->balance == -c) {
		s->balance = 0;
		return 0;
	}
	if(s->c[(c+1)/2]->balance == c)
		s = singlerot(c, s);
	else
		s = doublerot(c, s);
	*t = s;
	return 0;
}

static int delete(int (*cmp)(Avl*, Avl*), Avl**, Avl*, Avl**);
static int deletemin(Avl**, Avl**);
static int deletefix(int, Avl**);

Avl*
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
delete(int (*cmp)(Avl*, Avl*), Avl **qp, Avl *k, Avl **oldp)
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
	if(s->balance == 0) {
		s->balance = c;
		return 0;
	}
	if(s->balance == -c) {
		s->balance = 0;
		return 1;
	}
	a = (c+1)/2;
	if(s->c[a]->balance == 0) {
		s = rotate(c, s);
		s->balance = -c;
		*t = s;
		return 0;
	}
	if(s->c[a]->balance == c)
		s = singlerot(c, s);
	else
		s = doublerot(c, s);
	*t = s;
	return 1;
}

static Avl*
singlerot(int c, Avl *s)
{
	s->balance = 0;
	s = rotate(c, s);
	s->balance = 0;
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
	assert(r->p == p);
	assert(s->p == p);

	if(p->balance == c) {
		s->balance = -c;
		r->balance = 0;
	} else if(p->balance == -c) {
		s->balance = 0;
		r->balance = c;
	} else
		s->balance = r->balance = 0;
	p->balance = 0;
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

Avl*
avlprev(Avl *q)
{
	return walk1(0, q);
}

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

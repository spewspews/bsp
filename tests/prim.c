// This solves the problem posed at https://www.hackerrank.com/challenges/primsmstsub
// The implementation of the Prim algorithm is the function prim. Everything else
// is setup.

#define BSP_FIBHEAP_IMPLEMENTATION
#include "../bspfibheap.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
sysfatal(char *fmt, ...)
{
	char buf[1024];
	int w;
	va_list va;

	w = snprintf(buf, sizeof(buf), "dijkstra: ");
	va_start(va, fmt);
	w += vsnprintf(buf+w, sizeof(buf)-w, fmt, va);
	va_end(va);
	snprintf(buf+w, sizeof(buf)-w, ": %s", strerror(errno));

	fprintf(stderr, "%s\n", buf);
	exit(1);
}

typedef struct Edge Edge;
typedef struct Nodedata Nodedata;

struct Edge {
	Fibnode fibnode;
	int node, dist;
	Edge *next;
};

struct Nodedata {
	Edge *edges, **etail;
	int intree, node;
};

struct {
	Nodedata *a;
	int len;
} nodes;

Edge *edgepool;

Edge*
edgealloc(void)
{
	Edge *e;

	if(edgepool == NULL)
		return malloc(sizeof(Edge));

	e = edgepool;
	edgepool = edgepool->next;
	return e;
}

Nodedata*
nodedata(int n)
{
	return nodes.a + n-1;
}

int
edgecmp(Fibnode *a, Fibnode *b)
{
	Edge *e, *f;

	e = (Edge*)a;
	f = (Edge*)b;

	if(e->dist < f->dist)
		return -1;
	if(e->dist > f->dist)
		return 1;
	return 0;
}

void
insertedges(Fibheap *pq, Nodedata *s)
{
	Edge *e;
	Nodedata *d;

	for(e = s->edges; e != NULL; e = e->next) {
		d = nodedata(e->node);
		if(d->intree)
			continue;
		fibinsert(pq, &e->fibnode);
	}
}

int
prim(int start)
{
	Fibheap pq;
	Nodedata *n;
	Edge *e;
	int primsum;

	fibinit(&pq, edgecmp);
	n = nodedata(start);
	n->intree = 1;
	primsum = 0;
	insertedges(&pq, n);
	while(pq.min != NULL) {
		e = (Edge*)pq.min;
		if(fibdeletemin(&pq) < 0)
			sysfatal("deletion failed");
		n = nodedata(e->node);
		if(n->intree)
			continue;
		n->intree = 1;
		primsum += e->dist;
		insertedges(&pq, n);
	}
	return primsum;
}

void
addedge(int s, int d, int dist)
{
	Edge *e;
	Nodedata *nd;

	e = edgealloc();
	e->node = d;
	e->dist = dist;
	e->next = NULL;
	nd = nodedata(s);
	*nd->etail = e;
	nd->etail = &e->next;
}

void
initnodedata(Nodedata *nd, int n)
{
	*nd->etail = edgepool;
	edgepool = nd->edges;
	nd->edges = NULL;
	nd->etail = &nd->edges;
	nd->intree = 0;
	nd->node = n;
}

void
reallocnodes(int nnodes)
{
	Nodedata *ndi;

	nodes.len = 2*nnodes;
	free(nodes.a);
	nodes.a = calloc(nodes.len, sizeof(*nodes.a));
	for(ndi = nodes.a; ndi < nodes.a+nodes.len; ndi++)
		ndi->etail = &ndi->edges;
}

int
main(void)
{
	int nnodes, edges, s, d, dist, start, i;

	scanf("%d %d", &nnodes, &edges);
	if(nodes.len < nnodes)
		reallocnodes(nnodes);
	for(i = 0; i < nnodes; i++)
		initnodedata(nodes.a + i, i+1);

	while(edges-- > 0) {
		scanf("%d %d %d", &s, &d, &dist);
		addedge(s, d, dist);
		addedge(d, s, dist);
	}

	scanf("%d", &start);
	printf("%d\n", prim(start));
}

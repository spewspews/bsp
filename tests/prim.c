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
typedef struct Node Node;

struct Edge {
	Fibnode fibnode;
	Edge *next;
	Node *node;
	int dist;
};

struct Node {
	Edge *edges, **etail;
	int intree;
};

struct {
	Node *a;
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

Node*
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
addedge(int s, int d, int dist)
{
	Edge *e;
	Node *n;

	e = edgealloc();
	e->node = nodedata(d);
	e->dist = dist;
	e->next = NULL;
	n = nodedata(s);
	*n->etail = e;
	n->etail = &e->next;
}

void
initnodedata(Node *n)
{
	*n->etail = edgepool;
	edgepool = n->edges;
	n->edges = NULL;
	n->etail = &n->edges;
	n->intree = 0;
}

void
reallocnodes(int nnodes)
{
	Node *ni;

	nodes.len = 2*nnodes;
	free(nodes.a);
	nodes.a = calloc(nodes.len, sizeof(*nodes.a));
	for(ni = nodes.a; ni < nodes.a+nodes.len; ni++)
		ni->etail = &ni->edges;
}

void
insertedges(Fibheap *pq, Node *s)
{
	Edge *e;

	for(e = s->edges; e != NULL; e = e->next) {
		if(e->node->intree)
			continue;
		fibinsert(pq, &e->fibnode);
	}
}

int
prim(int start)
{
	Fibheap pq;
	Node *n;
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
		n = e->node;
		if(n->intree)
			continue;
		n->intree = 1;
		primsum += e->dist;
		insertedges(&pq, n);
	}
	fibfree(&pq);
	return primsum;
}

int
main(void)
{
	Node *ni;
	int nnodes, edges, s, d, dist, start;

	scanf("%d %d", &nnodes, &edges);
	if(nodes.len < nnodes)
		reallocnodes(nnodes);
	for(ni = nodes.a; ni < nodes.a+nnodes; ni++)
		initnodedata(ni);

	while(edges-- > 0) {
		scanf("%d %d %d", &s, &d, &dist);
		addedge(s, d, dist);
		addedge(d, s, dist);
	}

	scanf("%d", &start);
	printf("%d\n", prim(start));
}

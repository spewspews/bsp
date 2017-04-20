// This solves the problem posed at https://www.hackerrank.com/challenges/dijkstrashortreach
// The implementation of the Dijkstra algorithm is the function dijkstra. Everything else
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
	int node, dist;
	Edge *next;
};

struct Nodedata {
	Fibnode fibnode;
	Edge *edges, **etail;
	int dist, node;
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
nodecmp(Fibnode *a, Fibnode *b)
{
	Nodedata *m, *n;

	m = (Nodedata*)a;
	n = (Nodedata*)b;

	if(m->dist < n->dist)
		return -1;
	if(m->dist > n->dist)
		return 1;
	return 0;
}

void
dijkstra(int start)
{
	Fibheap pq;
	Nodedata *s, *d;
	Edge *e;
	int dist;

	fibinit(&pq, nodecmp);
	s = nodedata(start);
	s->dist = 0;
	fibinsert(&pq, &s->fibnode);
	while(pq.min != NULL) {
		s = (Nodedata*)pq.min;
		if(fibdeletemin(&pq) < 0)
			sysfatal("deletion failed");
		for(e = s->edges; e != NULL; e = e->next) {
			d = nodedata(e->node);
			dist = s->dist + e->dist;
			if(d->dist < 0) {
				d->dist = dist;
				fibinsert(&pq, &d->fibnode);
			} else if(d->dist > dist) {
				d->dist = dist;
				fibdecreasekey(&pq, &d->fibnode);
			}
		}
	}
	fibfree(&pq);
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
	nd->dist = -1;
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

void
testcase(void)
{
	Nodedata *ndi;
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
	dijkstra(start);
	i = 0;
	for(ndi = nodes.a; ndi < nodes.a + nnodes; ndi++) {
		if(ndi->node == start)
			continue;
		if(i++ > 0)
			printf(" ");
		printf("%d", ndi->dist);
	}
	printf("\n");
}

int
main(void)
{
	int cases;

	scanf("%d", &cases);
	while(cases-- > 0)
		testcase();

	exit(0);
}

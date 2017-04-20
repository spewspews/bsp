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
typedef struct Node Node;

struct Edge {
	Node *node;
	Edge *next;
	int dist;
};

struct Node {
	Fibnode fibnode;
	Edge *edges, **etail;
	int dist;
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
nodecmp(Fibnode *a, Fibnode *b)
{
	Node *m, *n;

	m = (Node*)a;
	n = (Node*)b;

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
	Node *s, *d;
	Edge *e;
	int dist;

	fibinit(&pq, nodecmp);
	s = nodedata(start);
	s->dist = 0;
	fibinsert(&pq, &s->fibnode);
	while(pq.min != NULL) {
		s = (Node*)pq.min;
		if(fibdeletemin(&pq) < 0)
			sysfatal("deletion failed");
		for(e = s->edges; e != NULL; e = e->next) {
			dist = s->dist + e->dist;
			d = e->node;
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
	n->dist = -1;
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
testcase(void)
{
	Node *ni;
	int nnodes, edges, s, d, dist, start, i;

	scanf("%d %d", &nnodes, &edges);
	if(nodes.len < nnodes)
		reallocnodes(nnodes);
	for(i = 0; i < nnodes; i++)
		initnodedata(nodes.a + i);

	while(edges-- > 0) {
		scanf("%d %d %d", &s, &d, &dist);
		addedge(s, d, dist);
		addedge(d, s, dist);
	}

	scanf("%d", &start);
	dijkstra(start);
	i = 0;
	for(ni = nodes.a; ni < nodes.a + nnodes; ni++) {
		if(ni->dist == 0)
			continue;
		if(i++ > 0)
			printf(" ");
		printf("%d", ni->dist);
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

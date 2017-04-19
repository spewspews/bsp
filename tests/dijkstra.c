#define BSP_FIBHEAP_IMPLEMENTATION
#include "../bspfibheap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
} nodesdata;

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

void
edgefree(Edge *e)
{
	e->next = edgepool;
	edgepool = e;
}

Nodedata*
nodedata(int n)
{
	return nodesdata.a + n-1;
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
dijkstra(int start)
{
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
reallocnodesdata(int nodes)
{
	nodesdata.len = 2*nodes;
	free(nodesdata.a);
	nodesdata.a = calloc(nodesdata.len, sizeof(*nodesdata.a));
}

void
testcase(void)
{
	int nodes, edges, s, d, dist, start, i;

	scanf("%d %d", &nodes, &edges);
	if(nodesdata.len < nodes)
		reallocnodesdata(nodes);
	for(i = 0; i < nodes; i++)
		initnodedata(&nodesdata.a[i], i+1);

	while(edges-- > 0) {
		scanf("%d %d %d", &s, &d, &dist);
		addedge(s, d, dist);
		addedge(d, s, dist);
	}

	scanf("%d", &start);
	dijkstra(start);
}

int
main(void)
{
	int cases;

	scanf("%d", &cases);
	printf("cases %d\n", cases);
	while(cases-- > 0)
		testcase();

	exit(0);
}

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

void
addedge(int s, int d, int dist)
{
	Edge *e;

	e = edgealloc();
	e->node = d;
	e->dist = dist;
	e->next = NULL;
	*nodesdata.a[s-1].etail = e;
	nodesdata.a[s-1].etail = &e->next;
}

void
dijkstra(int start)
{
}

void
initnode(Nodedata *nd, int n)
{
	*ndp->etail = edgepool;
	edgepool = ndp->edges;
	ndp->edges = NULL;
	ndp->etail = &ndp->edges;
	ndp->dist = -1;
	ndp->node = n++;
}

void
testcase(void)
{
	Nodedata *ndp;
	int nodes, edges, s, d, dist, start, n;

	scanf("%d %d", &nodes, &edges);
	if(nodesdata.len < nodes+1) {
		nodesdata.len = 2*(nodes+1);
		free(nodesdata.a);
		nodesdata.a = calloc(nodesdata.len, sizeof(*nodesdata.a));
	}
	n = 1;
	for(ndp = nodesdata.a; ndp < nodesdata.a + nodes; ndp++)
		initnode(ndp, n++);

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

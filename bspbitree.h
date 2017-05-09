#include <stdlib.h>

typedef struct Bitree Bitree;
struct Bitree {
	int *arr;
	size_t alen;
};

Bitree*
bitinit(Bitree *tree, size_t n)
{
	tree->arr = calloc(n+1, sizeof(*tree->arr));
	tree->alen = n+1;
	return tree;
}

void
bitupdate(Bitree *tree, size_t i, int v)
{
	i++;
	while(i < tree->alen) {
		tree->arr[i] += v;
		i += i & -i;
	}
}

int
bitsum(Bitree *tree, size_t i, size_t j)
{
	int count;

	count = 0;
	while(j > i) {
		count += tree->arr[j];
		j -= j & -j;
	}
	while(i > j) {
		count -= tree->arr[i];
		i -= i & -i;
	}
	return count;
}

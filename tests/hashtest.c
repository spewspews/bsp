#define _XOPEN_SOURCE
#include "../bsphash.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

typedef struct Int Int;
struct Int {
	Hashval h;
	int val;
};

enum {
	NNODES = 100,
	RANDMAX = 50,
};

int
main(void)
{
	Hash hash;
	Hashval lookup;
	Int pool[NNODES];
	Int *ip, *i;

	srand48(time(NULL));

	ip = pool;

	hashinit(&hash);
	i = ip++;
	i->h.key = "foobar";
	i->h.keysize = strlen("foobar");
	i->val = drand48()*RANDMAX;
	printf("inserting %d at foobar\n", i->val);
	hashinsert(&hash, &i->h);
	lookup.key = "baz";
	lookup.keysize = strlen("baz");
	i = (Int*)hashlookup(&hash, &lookup);
	if(i != NULL)
		printf("Found element not there\n");
	lookup.key = "foobar";
	lookup.keysize = strlen("foobar");
	i = (Int*)hashlookup(&hash, &lookup);
	if(i == NULL)
		printf("Didn't find foobar\n");
	else
		printf("foobar's val is %d\n", i->val);
}

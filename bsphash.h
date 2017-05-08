#include <stdlib.h>
#include <string.h>

typedef struct Hash Hash;
typedef struct Hashval Hashval;

struct Hashval {
	void *key;
	size_t keysize;
	Hashval *next;
};

enum {
	NBKT = 1031,
};

struct Hash {
	Hashval *bkt[NBKT];
};

Hash*
hashinit(Hash *h)
{
	memset(h->bkt, 0, sizeof(h->bkt));
	return h;
}

static unsigned long
djb2(void *key, size_t keysize)
{
	char *s, *sp;
	unsigned long hash = 5831;

	s = (char*)key;
	for(sp = s; sp < s + keysize; sp++)
		hash = ((hash << 5) + hash) ^ *sp;
	return hash;
}

Hashval*
hashinsert(Hash *map, Hashval *k)
{
	Hashval **hp, *h;
	unsigned long hash;

	hash = djb2(k->key, k->keysize) % NBKT;
	for(hp = &map->bkt[hash]; *hp != NULL; hp = &h->next) {
		h = *hp;
		if(h->keysize == k->keysize && memcmp(h->key, k->key, h->keysize) == 0) {
			k->next = h->next;
			*hp = k;
			return h;
		}
	}
	k->next = map->bkt[hash];
	map->bkt[hash] = k;
	return NULL;
}

Hashval*
hashlookup(Hash *map, Hashval *k) {
	Hashval *h;
	unsigned long hash;

	hash = djb2(k->key, k->keysize) % NBKT;
	for(h = map->bkt[hash]; h != NULL; h = h->next) {
		if(h->keysize == k->keysize && memcmp(h->key, k->key, h->keysize) == 0)
			break;
	}
	return h;
}

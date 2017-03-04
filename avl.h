typedef signed char int8_t;

typedef struct Avl Avl;
typedef struct Avltree Avltree;

struct Avl {
	Avl *c[2];
	Avl *p;
	int8_t balance;
};

struct Avltree {
	int (*cmp)(Avl*, Avl*);
	Avl *root;
};

Avltree *avlcreate(int(*cmp)(Avl*, Avl*));
Avl *avllookup(Avltree*, Avl*);
Avl *avldelete(Avltree*, Avl*);
Avl *avlinsert(Avltree*, Avl*);
Avl *avlnext(Avl*);
Avl *avlprev(Avl*);

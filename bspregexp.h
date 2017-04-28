#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <wchar.h>

enum {
	OANY,
	OBOL,
	OCLASS,
	OEOL,
	OJMP,
	ONOTNL,
	ORUNE,
	OSAVE,
	OSPLIT,
	OUNSAVE,
};

typedef struct Resub Resub;
typedef struct Reinst Reinst;
typedef struct Reprog Reprog;
typedef struct Rethread Rethread;

struct Resub
{
	union
	{
		char *sp;
		wchar_t *rsp;
	};
	union
	{
		char *ep;
		wchar_t *rep;
	};
};
struct Reprog
{
	Reinst *startinst;
	Rethread *threads;
	char *regstr;
	int len;
	int nthr;
};

Reprog* regcomp(char*);
Reprog* regcomplit(char*);
Reprog* regcompnl(char*);
void    regerror(char*);
int     regexec(Reprog*, char*, Resub*, int);
void	regsub(char*, char*, int, Resub*, int);
int     rregexec(Reprog*, wchar_t*, Resub*, int);
void    rregsub(wchar_t*, wchar_t*, int, Resub*, int);

enum {
	LANY = 0,
	LBOL,
	LCLASS,
	LEND,
	LEOL,
	LLPAR,
	LOR,
	LREP,
	LRPAR,
	LRUNE,

	TANY = 0,
	TBOL,
	TCAT,
	TCLASS,
	TEOL,
	TNOTNL,
	TOR,
	TPLUS,
	TQUES,
	TRUNE,
	TSTAR,
	TSUB,

	NSUBEXPM = 32
};

typedef struct Parselex Parselex;
typedef struct Renode Renode;

struct Parselex {
	/* Parse */
	Renode *next;
	Renode *nodes;
	int sub;
	int instrs;
	jmp_buf exitenv;
	/* Lex */
	void (*getnextr)(Parselex*);
	char *regstr;
	char *orig;
	wchar_t rune;
	int peek;
	int peeklex;
	int done;
	int literal;
	wchar_t cpairs[400+2];
	int nc;
};

struct Renode {
	int op;
	Renode *left;
	wchar_t r;
	union
	{
		wchar_t r1;
		int sub;
		Renode *right;
	};
	int nclass;
};

struct Rethread {
	Reinst *i;
	Resub sem[NSUBEXPM];
	Rethread *next;
	int gen;
};

struct Reinst {
	char op;
	int gen;
	Reinst *a;
	union
	{
		wchar_t r;
		int sub;
	};
	union
	{
		wchar_t r1;
		Reinst *b;
	};
};

static int lex(Parselex*);
static void getnextr(Parselex*);
static void getnextrlit(Parselex*);
static void getclass(Parselex*);
static Renode *e0(Parselex*);
static Renode *e1(Parselex*);
static Renode *e2(Parselex*);
static Renode *e3(Parselex*);
static Renode *buildclass(Parselex*);
static Renode *buildclassn(Parselex*);
static int pcmp(const void*, const void*);
static Reprog *regcomp1(char*, int, int);
static Reinst *compile(Renode*, Reprog*, int);
static Reinst *compile1(Renode*, Reinst*, int*, int);
static inline void prtree(Renode*, int, int);

int instrcnt[] = {
	[TANY]   = 2,
	[TBOL]   = 1,
	[TCAT]   = 0,
	[TCLASS] = 1,
	[TEOL]   = 1,
	[TNOTNL] = 1,
	[TOR]    = 2,
	[TPLUS]  = 1,
	[TQUES]  = 1,
	[TRUNE]  = 1,
	[TSTAR]  = 2,
	[TSUB]   = 2,
};

static Renode*
node(Parselex *plex, int op, Renode *l, Renode *r)
{
	Renode *n;

	plex->instrs += instrcnt[op];
	n = plex->next++;
	n->op = op;
	n->left = l;
	n->right = r;
	return n;
}

static Renode*
e3(Parselex *plex)
{
	char error[1024];
	Renode *n;

	switch(lex(plex)) {
	case LANY:
		return node(plex, TANY, NULL, NULL);
	case LBOL:
		return node(plex, TBOL, NULL, NULL);
	case LEOL:
		return node(plex, TEOL, NULL, NULL);
	case LRUNE:
		n = node(plex, TRUNE, NULL, NULL);
		n->r = plex->rune;
		return n;
	case LCLASS:
		if(plex->nc)
			return buildclassn(plex);
		return buildclass(plex);
	case LLPAR:
		n = e0(plex);
		n = node(plex, TSUB, n, NULL);
		if(lex(plex) != LRPAR) {
			snprintf(error, sizeof(error), "regexp %s: no matching parenthesis", plex->orig);
			regerror(error);
			longjmp(plex->exitenv, 1);
		}
		return n;
	default:
		if(plex->rune)
			snprintf(error, sizeof(error), "regexp %s: syntax error: %C", plex->orig, plex->rune);
		else
			snprintf(error, sizeof(error), "regexp %s: parsing error", plex->orig);
		regerror(error);
		longjmp(plex->exitenv, 1);
	}
	return NULL;
}

static Renode*
e2(Parselex *plex)
{
	Renode *n;

	n = e3(plex);
	while(lex(plex) == LREP) {
		switch(plex->rune) {
		case L'*':
			n = node(plex, TSTAR, n, NULL);
			break;
		case L'+':
			n = node(plex, TPLUS, n, NULL);
			break;
		case L'?':
			n = node(plex, TQUES, n, NULL);
			break;
		}
	}
	plex->peek = 1;
	return n;
}

static Renode*
invert(Renode *n)
{
	Renode *n1;

	if(n->op != TCAT)
		return n;
	while(n->left->op == TCAT) {
		n1 = n->left;
		n->left = n1->right;
		n1->right = n;
		n = n1;
	}
	return n;
}

static Renode*
e1(Parselex *plex)
{
	Renode *n;
	int sym;

	n = e2(plex);
	for(;;) {
		sym = lex(plex);
		if(sym == LEND || sym == LOR || sym == LRPAR)
			break;
		plex->peek = 1;
		n = node(plex, TCAT, n, e2(plex));
	}
	plex->peek = 1;
	return invert(n);
}

static Renode*
e0(Parselex *plex)
{
	Renode *n;

	n = e1(plex);
	for(;;) {
		if(lex(plex) != LOR)
			break;
		n = node(plex, TOR, n, e1(plex));
	}
	plex->peek = 1;
	return n;
}

static Parselex*
initplex(Parselex *plex, char *regstr, int lit)
{
	plex->getnextr = lit ? getnextrlit : getnextr;
	plex->regstr = plex->orig = regstr;
	plex->sub = 0;
	plex->instrs = 0;
	plex->peek = 0;
	plex->done = 0;
	return plex;
}

static Reprog*
regcomp1(char *regstr, int nl, int lit)
{
	Reprog *reprog;
	Parselex plex;
	Renode *parsetr;
	int regstrlen, maxthr;

	regstrlen = mbstowcs(NULL, regstr, 0);
	if(regstrlen == -1) {
		regerror("invalid character encoding in regexp");
		return NULL;
	}
	initplex(&plex, regstr, lit);
	plex.nodes = calloc(sizeof(*plex.nodes), regstrlen*2);
	if(plex.nodes == NULL)
		return NULL;
	plex.next = plex.nodes;

	if(setjmp(plex.exitenv) != 0) {
		free(plex.nodes);
		return NULL;
	}

	maxthr = regstrlen + 1;
	parsetr = node(&plex, TSUB, e0(&plex), NULL);

//	prtree(parsetr, 0, 1);
	reprog = malloc(sizeof(Reprog) +
	                sizeof(Reinst) * plex.instrs +
	                sizeof(Rethread) * maxthr);
	reprog->len = plex.instrs;
	reprog->nthr = maxthr;
	reprog->startinst = compile(parsetr, reprog, nl);
	reprog->threads = (Rethread*)(reprog->startinst + reprog->len);
	reprog->regstr = regstr;

	free(plex.nodes);
	return reprog;
}

Reprog*
regcomp(char *str)
{
	return regcomp1(str, 0, 0);
}

Reprog*
regcomplit(char *str)
{
	return regcomp1(str, 0, 1);
}

Reprog*
regcompnl(char *str)
{
	return regcomp1(str, 1, 0);
}

static Reinst*
compile1(Renode *renode, Reinst *reinst, int *sub, int nl)
{
	Reinst *i;
	int s;

Tailcall:
	if(renode == NULL)
		return reinst;
	switch(renode->op) {
	case TCLASS:
		reinst->op = OCLASS;
		reinst->r = renode->r;
		reinst->r1 = renode->r1;
		reinst->a = reinst + 1 + renode->nclass;
		renode = renode->left;
		reinst++;
		goto Tailcall;
	case TCAT:
		reinst = compile1(renode->left, reinst, sub, nl);
		renode = renode->right;
		goto Tailcall;
	case TOR:
		reinst->op = OSPLIT;
		reinst->a = reinst + 1;
		i = compile1(renode->left, reinst->a, sub, nl);
		reinst->b = i + 1;
		i->op = OJMP;
		i->a = compile1(renode->right, reinst->b, sub, nl);
		return i->a;
	case TSTAR:
		reinst->op = OSPLIT;
		reinst->a = reinst + 1;
		i = compile1(renode->left, reinst->a, sub, nl);
		reinst->b = i + 1;
		i->op = OJMP;
		i->a = reinst;
		return reinst->b;
	case TPLUS:
		i = reinst;
		reinst = compile1(renode->left, reinst, sub, nl);
		reinst->op = OSPLIT;
		reinst->a = i;
		reinst->b = reinst + 1;
		return reinst->b;
	case TQUES:
		reinst->op = OSPLIT;
		reinst->a = reinst + 1;
		reinst->b = compile1(renode->left, reinst->a, sub, nl);
		return reinst->b;
	case TSUB:
		reinst->op = OSAVE;
		reinst->sub = s = (*sub)++;
		reinst = compile1(renode->left, reinst+1, sub, nl);
		reinst->op = OUNSAVE;
		reinst->sub = s;
		return reinst + 1;
	case TANY:
		if(nl == 0)
			reinst++->op = ONOTNL;
		reinst->op = OANY;
		return reinst + 1;
	case TRUNE:
		reinst->op = ORUNE;
		reinst->r = renode->r;
		return reinst + 1;
	case TNOTNL:
		reinst->op = ONOTNL;
		return reinst + 1;
	case TEOL:
		reinst->op = OEOL;
		return reinst + 1;
	case TBOL:
		reinst->op = OBOL;
		return reinst + 1;
	}
	return NULL;
}

static Reinst*
compile(Renode *parsetr, Reprog *reprog, int nl)
{
	Reinst *reinst, *end;
	int sub;

	sub = 0;
	reinst = (Reinst*)(reprog+1);
	end = compile1(parsetr, reinst, &sub, nl);
	assert(end <= reinst + reprog->len);
	return reinst;
}

static void
getnextr(Parselex *l)
{
	l->literal = 0;
	if(l->done) {
		l->rune = 0;
		return;
	}
	l->regstr += mbtowc(&l->rune, l->regstr, MB_CUR_MAX);
	if(l->rune == L'\\') {
		l->regstr += mbtowc(&l->rune, l->regstr, MB_CUR_MAX);
		l->literal = 1;
	}
	if(*l->regstr == 0)
		l->done = 1;
	return;
}

static void
getnextrlit(Parselex *l)
{
	l->literal = 1;
	if(l->done) {
		l->literal = 0;
		l->rune = 0;
		return;
	}
	l->regstr += mbtowc(&l->rune, l->regstr, MB_CUR_MAX);
	if(*l->regstr == 0)
		l->done = 1;
	return;
}

static int
lex(Parselex *l)
{
	if(l->peek) {
		l->peek = 0;
		return l->peeklex;
	}
	l->getnextr(l);
	if(l->literal)
		return l->peeklex = LRUNE;
	switch(l->rune){
	case L'\0':
		return l->peeklex = LEND;
	case L'*':
	case L'?':
	case L'+':
		return l->peeklex = LREP;
	case L'|':
		return l->peeklex = LOR;
	case L'.':
		return l->peeklex = LANY;
	case L'(':
		return l->peeklex = LLPAR;
	case L')':
		return l->peeklex = LRPAR;
	case L'^':
		return l->peeklex = LBOL;
	case L'$':
		return l->peeklex = LEOL;
	case L'[':
		getclass(l);
		return l->peeklex = LCLASS;
	}
	return l->peeklex = LRUNE;
}

static int
pcmp(const void *va, const void *vb)
{
	long long n;
	const wchar_t *a, *b;

	a = va;
	b = vb;

	n = (long long)b[0] - (long long)a[0];
	if(n)
		return n;
	return (long long)b[1] - (long long)a[1];
}

static void
getclass(Parselex *l)
{
	wchar_t *p, *q, t;

	l->nc = 0;
	getnextrlit(l);
	if(l->rune == L'^') {
		l->nc = 1;
		getnextrlit(l);
	}
	p = l->cpairs;
	for(;;) {
		*p = l->rune;
		if(l->rune == L']')
			break;
		if(l->rune == L'-') {
			regerror("Malformed class");
			longjmp(l->exitenv, 1);
		}
		if(l->rune == '\\') {
			getnextrlit(l);
			*p = l->rune;
		}
		if(l->rune == 0) {
			regerror("No closing ] for class");
			longjmp(l->exitenv, 1);
		}
		getnextrlit(l);
		if(l->rune == L'-') {
			getnextrlit(l);
			if(l->rune == L']') {
				regerror("Malformed class");
				longjmp(l->exitenv, 1);
			}
			if(l->rune == L'-') {
				regerror("Malformed class");
				longjmp(l->exitenv, 1);
			}
			if(l->rune == L'\\')
				getnextrlit(l);
			p[1] = l->rune;
			if(p[0] > p[1]) {
				t = p[0];
				p[0] = p[1];
				p[1] = t;
			}
			getnextrlit(l);
		} else
			p[1] = p[0];
		if(p >= l->cpairs + sizeof(l->cpairs)/sizeof(*l->cpairs) - 2) {
			regerror("Class too big\n");
			longjmp(l->exitenv, 1);
		}
		p += 2;
	}
	*p = L'\0';
	qsort(l->cpairs, (p - l->cpairs)/2, 2*sizeof(*l->cpairs), pcmp);
	q = l->cpairs;
	for(p = l->cpairs+2; *p != 0; p += 2) {
		if(p[1] < q[0] - 1) {
			q[2] = p[0];
			q[3] = p[1];
			q += 2;
			continue;
		}
		q[0] = p[0];
		if(p[1] > q[1])
			q[1] = p[1];
	}
	q[2] = 0;
}

/* classes are in descending order */
static Renode*
buildclassn(Parselex *l)
{
	Renode *n;
	wchar_t *p;
	int i;

	i = 0;
	p = l->cpairs;
	n = node(l, TCLASS, NULL, NULL);
	n->r = p[1] + 1;
	n->r1 = WCHAR_MAX;
	n->nclass = i++;

	for(; *p != 0; p += 2) {
		n = node(l, TCLASS, n, NULL);
		n->r = p[3] + 1;
		n->r1 = p[0] - 1;
		n->nclass = i++;
	}
	n->r = 0;
	return node(l, TCAT, node(l, TNOTNL, NULL, NULL), n);
}

static Renode*
buildclass(Parselex *l)
{
	Renode *n;
	wchar_t *p;
	int i;

	i = 0;
	n = node(l, TCLASS, NULL, NULL);
	n->r = WEOF;
	n->nclass = i++;

	for(p = l->cpairs; *p != 0; p += 2) {
		n = node(l, TCLASS, n, NULL);
		n->r = p[0];
		n->r1 = p[1];
		n->nclass = i++;
	}
	return n;
}

static inline void
prtree(Renode *tree, int d, int f)
{
	int i;

	if(tree == NULL)
		return;
	if(f)
	for(i = 0; i < d; i++)
		printf("\t");
	switch(tree->op) {
	case TCAT:
		prtree(tree->left, d, 0);
		prtree(tree->right, d, 1);
		break;
	case TOR:
		printf("TOR\n");
		prtree(tree->left, d+1, 1);
		for(i = 0; i < d; i++)
			printf("\t");
		printf("|\n");
		prtree(tree->right, d+1, 1);
		break;
	case TSTAR:
		printf("*\n");
		prtree(tree->left, d+1, 1);
		break;
	case TPLUS:
		printf("+\n");
		prtree(tree->left, d+1, 1);
		break;
	case TQUES:
		printf("?\n");
		prtree(tree->left, d+1, 1);
		break;
	case TANY:
		printf(".\n");
		prtree(tree->left, d+1, 1);
		break;
	case TBOL:
		printf("^\n");
		break;
	case TEOL:
		printf("$\n");
		break;
	case TSUB:
		printf("TSUB\n");
		prtree(tree->left, d+1, 1);
		break;
	case TRUNE:
		printf("TRUNE: %C\n", tree->r);
		break;
	case TNOTNL:
		printf("TNOTNL: !\\n\n");
		break;
	case TCLASS:
		printf("CLASS: %C-%C\n", tree->r, tree->r1);
		prtree(tree->left, d, 1);
		break;
	}
}

typedef struct RethreadQ RethreadQ;
struct RethreadQ {
	Rethread *head;
	Rethread **tail;
};

int
regexec(Reprog *p, char *str, Resub *sem, int msize)
{
	RethreadQ lists[2], *clist, *nlist, *tmp;
	Rethread *t, *next, *pool, *avail;
	Reinst *ci;
	wchar_t r;
	char *sp, *ep, endc;
	int i, matchgen, gen;

	memset(p->threads, 0, sizeof(Rethread)*p->nthr);
	if(msize > NSUBEXPM)
		msize = NSUBEXPM;
	if(p->startinst->gen != 0) {
		for(ci = p->startinst; ci < p->startinst + p->len; ci++)
			ci->gen = 0;
	}

	clist = lists;
	clist->head = NULL;
	clist->tail = &clist->head;
	nlist = lists + 1;
	nlist->head = NULL;
	nlist->tail = &nlist->head;

	pool = p->threads;
	avail = NULL;
	gen = matchgen = 0;

	sp = str;
	ep = NULL;
	endc = '\0';
	if(sem != NULL && msize > 0) {
		if(sem->sp != NULL)
			sp = sem->sp;
		if(sem->ep != NULL && *sem->ep != '\0') {
			ep = sem->ep;
			endc = *sem->ep;
			*sem->ep = '\0';
		}
	}

	for(r = L'☺'; r != L'\0'; sp += i) {
		i = mbtowc(&r, sp, MB_CUR_MAX);
		gen++;
		if(matchgen == 0) {
			if(avail == NULL) {
				assert(pool < p->threads + p->nthr);
				t = pool++;
			} else {
				t = avail;
				avail = avail->next;
			}
			t->i = p->startinst;
			if(msize > 0)
				memset(t->sem, 0, sizeof(Resub)*msize);
			t->next = NULL;
			t->gen = gen;
			*clist->tail = t;
			clist->tail = &t->next;
		}
		t = clist->head;
		if(t == NULL)
			break;
		ci = t->i;
Again:
		if(ci->gen == gen)
			goto Done;
		ci->gen = gen;
		switch(ci->op) {
		case ORUNE:
			if(r != ci->r)
				goto Done;
		case OANY: /* fallthrough */
			next = t->next;
			t->i = ci + 1;
			t->next = NULL;
			*nlist->tail = t;
			nlist->tail = &t->next;
			goto Next;
		case OCLASS:
		Class:
			if(r < ci->r)
				goto Done;
			if(r > ci->r1) {
				ci++;
				goto Class;
			}
			next = t->next;
			t->i = ci->a;
			t->next = NULL;
			*nlist->tail = t;
			nlist->tail = &t->next;
			goto Next;
		case ONOTNL:
			if(r != L'\n') {
				ci++;
				goto Again;
			}
			goto Done;
		case OBOL:
			if(sp == str || sp[-1] == '\n') {
				ci++;
				goto Again;
			}
			goto Done;
		case OEOL:
			if(r == L'\n' || (r == L'\0' && ep == NULL)) {
				ci++;
				goto Again;
			}
			goto Done;
		case OJMP:
			ci = ci->a;
			goto Again;
		case OSPLIT:
			if(avail == NULL) {
				assert(pool < p->threads + p->nthr);
				next = pool++;
			} else {
				next = avail;
				avail = avail->next;
			}
			next->i = ci->b;
			if(msize > 0)
				memcpy(next->sem, t->sem, sizeof(Resub)*msize);
			next->next = t->next;
			next->gen = t->gen;
			t->next = next;
			ci = ci->a;
			goto Again;
		case OSAVE:
			if(ci->sub < msize)
				t->sem[ci->sub].sp = sp;
			ci++;
			goto Again;
		case OUNSAVE:
			if(ci->sub == 0) {
				matchgen = t->gen;
				if(sem != NULL && msize > 0) {
					memcpy(sem, t->sem, sizeof(*sem)*msize);
					sem->ep = sp;
				}
				goto Done;
			}
			if(ci->sub < msize)
				t->sem[ci->sub].ep = sp;
			ci++;
			goto Again;
		Done:
			next = t->next;
			t->next = avail;
			avail = t;
		Next:
			if(next == NULL)
				break;
			if(matchgen && next->gen > matchgen) {
				*clist->tail = avail;
				avail = next;
				break;
			}
			t = next;
			ci = t->i;
			goto Again;
		}
		tmp = clist;
		clist = nlist;
		nlist = tmp;
		nlist->head = NULL;
		nlist->tail = &nlist->head;
	}
	if(ep != NULL)
		*ep = endc;
	return matchgen > 0 ? 1 : 0;
}

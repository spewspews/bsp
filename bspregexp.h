#ifdef BSP_REGEXP_STATIC
#define __BSP_REGEXP_SCOPE static
#else
#define __BSP_REGEXP_SCOPE
#endif

#ifndef __BSP_REGEXP_H_INCLUDE
#define __BSP_REGEXP_H_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	OANY = 0,
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
	char *sp;
	char *ep;
};

struct Reprog
{
	Reinst *startinst;
	Rethread *threads;
	char *regstr;
	int len;
	int nthr;
};

__BSP_REGEXP_SCOPE Reprog* regcomp(char *regex);
__BSP_REGEXP_SCOPE Reprog* regcomplit(char *regex);
__BSP_REGEXP_SCOPE Reprog* regcompnl(char *regex);
__BSP_REGEXP_SCOPE void    regerror(char *err);
__BSP_REGEXP_SCOPE int     regexec(Reprog *regexp, char *matchstr, Resub *match, int msize);
__BSP_REGEXP_SCOPE void    regsub(char *source, char *dest, int destlen, Resub *match, int msize);

#endif // __BSP_REGEXP_H_INCLUDE

#ifdef BSP_REGEXP_IMPLEMENTATION

#ifndef BSP_REGEXP_CALLOC
#include <stdlib.h>
#define BSP_REGEXP_CALLOC calloc
#endif

#ifndef BSP_REGEXP_FREE
#include <stdlib.h>
#define BSP_REGEXP_FREE free
#endif

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

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
	char *rawexp;
	char *orig;
	char rune;
	char peek;
	int peeklex;
	int done;
	int literal;
	char cpairs[400+2];
	int nc;
};

struct Renode {
	int op;
	Renode *left;
	char r;
	union
	{
		char r1;
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
		char r;
		int sub;
	};
	union
	{
		char r1;
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
		case '*':
			n = node(plex, TSTAR, n, NULL);
			break;
		case '+':
			n = node(plex, TPLUS, n, NULL);
			break;
		case '?':
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
	plex->rawexp = plex->orig = regstr;
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

	regstrlen = strlen(regstr);
	initplex(&plex, regstr, lit);
	plex.nodes = BSP_REGEXP_CALLOC(sizeof(*plex.nodes), regstrlen*2);
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
	reprog = BSP_REGEXP_CALLOC(1, sizeof(Reprog) +
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
		l->rune = '\0';
		return;
	}
	l->rune = *l->rawexp++;
	if(l->rune == L'\\') {
		l->rune = *l->rawexp++;
		l->literal = 1;
	}
	if(*l->rawexp == 0)
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
	l->rune = *l->rawexp++;
	if(*l->rawexp == 0)
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
	case '\0':
		return l->peeklex = LEND;
	case '*':
	case '?':
	case '+':
		return l->peeklex = LREP;
	case '|':
		return l->peeklex = LOR;
	case '.':
		return l->peeklex = LANY;
	case '(':
		return l->peeklex = LLPAR;
	case ')':
		return l->peeklex = LRPAR;
	case '^':
		return l->peeklex = LBOL;
	case '$':
		return l->peeklex = LEOL;
	case '[':
		getclass(l);
		return l->peeklex = LCLASS;
	}
	return l->peeklex = LRUNE;
}

static int
pcmp(const void *va, const void *vb)
{
	int c;
	const int *a, *b;

	a = va;
	b = vb;

	c = b[0] - a[0];
	if(c)
		return c;
	return b[1] - a[1];
}

static void
getclass(Parselex *l)
{
	char *p, *q, t;

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
		if(p >= l->cpairs + sizeof(l->cpairs) - 2) {
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
	char *p;
	int i;

	i = 0;
	p = l->cpairs;
	n = node(l, TCLASS, NULL, NULL);
	n->r = p[1] + 1;
	n->r1 = -1;
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
	char *p;
	int i;

	i = 0;
	n = node(l, TCLASS, NULL, NULL);
	n->r = -1;
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
		printf("TRUNE: %c\n", tree->r);
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

#endif // BSP_REGEXP_IMPLEMENTATION

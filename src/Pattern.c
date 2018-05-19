/** 2017 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 Regular expression pattern, \cite{Thompson1968Regular}, \cite{Cox2007},
 \url{ https://swtch.com/~rsc/regexp/regexp1.html },
 \url{ http://users.pja.edu.pl/~jms/qnx/help/watcom/wd/regexp.html },
 \url{ http://www.cs.sfu.ca/~cameron/Teaching/384/99-3/regexp-plg.html },
 \url{ http://matt.might.net/articles/parsing-regex-with-recursive-descent/ }.

 @title		Pattern
 @author	Neil
 @std		C89 with C99 stdint.h
 @version	2018-04
 @since
 2018-04 Conglomerated into {Pattern}. */

/*#include <stddef.h>*/	/* ptrdiff_t offset_of */
/*#include <stdlib.h>*/ /* malloc realloc free */
#include <stdlib.h> /* EXIT_ rand */
#include <stdio.h>  /* fprintf */
#include <string.h> /* memcmp strcmp strlen memmove memcpy memchr */
#include <errno.h>  /* errno */
#include <assert.h> /* assert */
#include <ctype.h>  /* isspace */
#include <stdarg.h> /* va_* */
#include <stdint.h> /* C99 uint32_t */
#include "Pattern.h"

struct Transition;
/** Mostly used as a debug tool, eg, printing graphs. */
typedef void (*TransitionToString)(const struct Transition *, char(*const)[12]);
/** A total order on {Transition}. */
typedef int (*TransitionComparator)(const struct Transition *,
	const struct Transition *);
/** Maps to a {size_t} for comparison in {TransitionComparator}, eg, the length
 for ranking which branch gets tried first. */
typedef size_t (*TransitionOrder)(const struct Transition *);

/**
 * Intermediary structure created when walking the graph of regular expression
 * and the remainder of the string that it's matched against. Used in
 * \see{match_here}.
 */
struct Match {
	const struct Transition *edge;
	const struct MachineVertex *vertex;
	const char *next;
};
typedef const char *(*Match)(const struct Match *const);

/**
 * Transition virtual table, referenced from {Transition}.
 */
struct TransitionVt {
	const char *class;
	const TransitionToString to_string;
	const Match match;
	const TransitionOrder order;
};

/**
 * Abstract {Transition}. This forms extra data on the edge of the {Machine}
 * digraph.
 */
struct Transition {
	const struct TransitionVt *vt;
};
/** Constructor; only called from it's children. */
static void Transition(struct Transition *const t,
	const struct TransitionVt *const vt) {
	assert(t && vt);
	t->vt = vt;
}
/** @implements <Transition>ToString */
static void transition_to_string(const struct Transition *t,char(*const a)[12]){
	t->vt->to_string(t, a);
}
/** @implements Match */
static const char *transition_match(struct Match *const match) {
	assert(match && match->edge && match->vertex && match->next);
	return match->edge->vt->match(match);
}
/** Sorts on decresing size.
 @implements <Transition>Comparator */
static int transition_cmp(const struct Transition *a,
	const struct Transition *b) {
	const size_t a_order = a->vt->order(a), b_order = b->vt->order(b);
	assert(a && b);
	return (b_order > a_order) - (a_order > b_order);
}
/*
 * Closure of vertices indices, used in the digraph.
 */
#define POOL_NAME Index
#define POOL_TYPE size_t
#define POOL_STACK
#include "Pool.h"

static void vertex_to_string(const struct IndexPool *v, char (*const a)[12]) {
	/*char num[64];*/
	assert(v && a);
	sprintf(*a, "a");
}

/* {Transition \in MachineVertex, MachineEdge, \in MachineDigraph}. */
#define DIGRAPH_NAME Machine
#define DIGRAPH_VDATA struct IndexPool
#define DIGRAPH_VDATA_TO_STRING &vertex_to_string
#define DIGRAPH_EDATA struct Transition
#define DIGRAPH_EDATA_TO_STRING &transition_to_string
#define DIGRAPH_EDATA_COMPARATOR &transition_cmp
#include "../src/Digraph.h"

/** @implements <<<Machine>Vertex>Link>Migrate */
static void vertex_migrate_each(struct MachineVertexLink *v,
	const struct Migrate *const migrate) {
	assert(v && migrate);
	MachineVertexLinkMigrate(&v->data, migrate);
	MachineEdgeListSelfCorrect(&v->data.out);
}
/*
 * {MachineVertex} container. Vertices are states that correspond to {Match}.
 * However, we don't need to store any data in them, just the edges.
 */
#define POOL_NAME Vertex
#define POOL_TYPE struct MachineVertexLink
#define POOL_MIGRATE_EACH &vertex_migrate_each
#define POOL_MIGRATE_ALL struct MachineDigraph
#include "Pool.h"

/*
 * {Empty} extends {Transition}.
 */
/** @implements <<<Machine>Edge>Link>Migrate */
static void empty_migrate_each(struct MachineEdgeLink *e,
	const struct Migrate *const migrate) {
	assert(e && migrate);
	MachineEdgeLinkMigrate(&e->data, migrate);
}
/** @implements <Transition>ToString */
static void empty_to_string(const struct Transition *e, char (*const a)[12]) {
	strcpy(*a, "ε");
	(void)e;
}
/** @implements Match */
static const char *empty_match(const struct Match *const match) {
	return match->next;
}
/** Empty is relegated to the very back.
 @implements <Transition>Order */
static size_t empty_order(const struct Transition *e) {
	(void)e;
	return 0;
}
static struct TransitionVt empty_vt
	= { "Empty", &empty_to_string, &empty_match, &empty_order };
#define POOL_NAME Empty
#define POOL_TYPE struct MachineEdgeLink
#define POOL_MIGRATE_EACH &empty_migrate_each
#include "Pool.h"
/** Constructor.
 @param e: This is instantiated; any data will be erased. */
static struct MachineEdgeLink *Empty(struct EmptyPool *const ep) {
	struct MachineEdgeLink *e;
	assert(ep);
	if(!(e = EmptyPoolNew(ep))) return 0;
	Transition(&e->data.info, &empty_vt);
	printf("Empty.\n");
	return e;
}

/**
 * {Literals} extends {Transition}.
 */
struct Literals {
	struct MachineEdgeLink edge;
	char *text;
	size_t text_size;
};
/** {container_of} (probably does nothing.) */
static const struct Literals *literals_holds_transition(const struct
	Transition *const t) {
	return (const struct Literals *)(const void *)
		((const char *)t
		- offsetof(struct MachineEdge, info)
		- offsetof(struct MachineEdgeLink, data)
		- offsetof(struct Literals, edge));
}
/** @implements <Literals>Migrate */
static void literals_migrate_each(struct Literals *l,
	const struct Migrate *const migrate) {
	assert(l && migrate);
	MachineEdgeLinkMigrate(&l->edge.data, migrate);
}
/** @implements <Transition>ToString */
static void literals_to_string(const struct Transition *t, char (*const a)[12]){
	sprintf(*a, "%.11s", literals_holds_transition(t)->text);
}
/** @implements Match */
static const char *literals_match(const struct Match *const match) {
	const struct Literals *l = literals_holds_transition(match->edge);
	if(memcmp(l->text, match->next, l->text_size)) return 0;
	return match->next + l->text_size;
}
/** @implements <Transition>Order */
static size_t literals_order(const struct Transition *t) {
	return literals_holds_transition(t)->text_size;
}
static struct TransitionVt literals_vt
	= { "Literals", &literals_to_string, &literals_match, &literals_order };
#define POOL_NAME Literals
#define POOL_TYPE struct Literals
#define POOL_MIGRATE_EACH &literals_migrate_each
#include "Pool.h"
/** Destructor because this takes up resources, but doesn't do anything about
 the graph.
 @implements <Literals>Action */
static void Literals_(struct Literals *const l) {
	if(!l) return;
	printf("~Literals: freeing %s.\n", l->text);
	free(l->text);
	l->text = 0;
	l->text_size = 0;
}
/** Constructor.
 @param text, text_size: The literal. A copy of this value is taken. Must have
 a non-zero length.
 @return Success.
 @throws {malloc} errors. */
static struct Literals *Literals(struct LiteralsPool *const lp,
	const char *const text, size_t text_size) {
	struct Literals *l;
	const char *a, *a_end;
	char *t;
	int is_escaped;
	assert(lp && text && text_size);
	if(!(l = LiteralsPoolNew(lp))) return 0;
	Transition(&l->edge.data.info, &literals_vt);
	l->text = 0;
	l->text_size = 0;
	/* Copy the literals; null terminator even thought it's not really used. */
	if(!(l->text = malloc(sizeof *text * (text_size + 1))))
		{ LiteralsPoolPop(lp); return 0; }
	/*memcpy(l->text, text, text_size);
	l->text[text_size] = '\0';
	l->text_size = text_size;*/
	for(a = text, a_end = a + text_size, t = l->text, is_escaped = 0;
		a < a_end; a++) {
		assert(*a != '\0');
		if(*a == '\\' && !is_escaped) { is_escaped = 1; continue; }
		*t++ = *a;
		is_escaped = 0;
	}
	*t = '\0';
	l->text_size = (size_t)(t - l->text);
	printf("Literals %p: <%s>:%lu\n", (void *)l, l->text,
		(unsigned long)l->text_size);
	return l;
}

#if 0 /* Not used yet. */
/**
 * {Sieve} extends {Transition}.
 */
struct Sieve {
	uint32_t bit[8]; /* Bit field, one byte, {256/32 = 8}. {!out} -> {bit}. */
}
static void bit_clear(uint32_t bit[8]) {
	bit[0] = bit[1] = bit[3] = bit[4] = bit[5] = bit[6] = bit[7] = 0;
}
static void bit_invert(uint32_t bit[8]) {
	bit[0] = ~bit[0], bit[1] = ~bit[1], bit[2] = ~bit[2], bit[3] = ~bit[3],
	bit[4] = ~bit[4], bit[5] = ~bit[5], bit[6] = ~bit[6], bit[7] = ~bit[7];
}
static void bit_set(uint32_t bit[8], char b) {
	uint32_t *const hi = bit + (b >> 5);
	*hi |= (1 << (b & 31));
}
static int bit_test(uint32_t bit[8], char b) {
	uint32_t *const hi = bit + (b >> 5);
	return *hi & (uint32_t)(1 << (b & 31));
}
/** Initialises an empty {state}.
 @param state: Has to be valid. */
static void Sieve(void) {
	assert(state);
	bit_clear(state->bit);
}
/** Adds {match} to state. */
static void sieve_add(struct Machine *const state, const char match) {
	assert(state);
	bit_set(state->bit, match);
}
/** Inverts the matches that will get though {state}.
 @fixme UTF-8 does not do as expecected. */
static void sieve_invert(struct Machine *const state) {
	assert(state);
	bit_invert(state->bit);
}
/** Tests {match} against {state}. */
static int sieve_match(struct Machine *const state, const char match) {
	assert(state);
	return bit_test(state->bit, match);
}
#define POOL_NAME Sieve
#define POOL_TYPE struct Sieve
#define POOL_MIGRATE_ALL struct MakeRe
#include "Pool.h"
#endif /* Not used. */

/**
 * {Pattern} is composed of the things defined above.
 */
struct Pattern {
	const char *title;
	struct MachineDigraph graph;
	struct VertexPool vs;
	struct EmptyPool empties;
	struct LiteralsPool literals;
};

/** Destructor.
 @param re: If null, does nothing, otherwise it is set to match zero characters
 and frees the memory. */
void Pattern_(struct Pattern **const pre) {
	struct Pattern *re;
	if(!pre || !(re = *pre)) return;
	printf("~Pattern<%s>.\n", re->title);
	LiteralsPoolForEach(&re->literals, &Literals_);
	LiteralsPool_(&re->literals);
	EmptyPool_(&re->empties);
	VertexPool_(&re->vs);
	MachineDigraph_(&re->graph);
	*pre = 0;
}

/* Defined at eof. */
static int compile_re(struct Pattern *re, const char *const compile);

/** Compiles a regular expression.
 @param compile: If null, returns null. Otherwise, this is a null-terminated
 modified UTF-8 string that gets compiled into a regular expression.
 @return The regular expression. Requires freeing with \see{Pattern_}.
 @throws {malloc/realloc} errors: {IEEE Std 1003.1-2001}.
 @throws EILSEQ: The {re} is not understood, (required since 1994 Amendment 1
 to C89 standard.) */
struct Pattern *Pattern(const char *const compile) {
	struct Pattern *re;
	if(!compile || !(re = malloc(sizeof *re))) return 0;
	re->title = compile;
	MachineDigraph(&re->graph);
	VertexPool(&re->vs, &MachineDigraphVertexMigrateAll, &re->graph);
	EmptyPool(&re->empties);
	LiteralsPool(&re->literals);
	if(!compile_re(re, compile)) Pattern_(&re);
	return re;
}

/** Checks if the {root} of the DFA matches {arg}. Called in \see{PatternMatch}.
 @return The end of the match if it was a match or null.
 @fixme implements <Machine,char*>Predicate,
 if(!(e = MachineEdgeListMatchShortCircuit(&s->out, &no_match, match))) would be
 nice. List doesn't have interfaces . . . yet? */
static int match_here(const struct MachineVertex *const root,
	const char *const arg) {
	struct Match m;
	struct MachineEdge *e;
	const char *end;
	enum { NOT_MATCHED, MATCHED } status = MATCHED;
	m.edge = 0, m.vertex = root, m.next = arg;
	while(m.vertex) {
		/* Finished when the vertex has degree zero. */
		if(!(e = MachineEdgeListFirst(&m.vertex->out))) break;
		status = NOT_MATCHED;
		do { /* Loop through all the edges out of this vertex. */
			m.edge = &e->info;
			if(!(end = transition_match(&m))) continue;
			m.vertex = e->to, m.next = end, status = MATCHED;
			break;
		} while((e = MachineEdgeListNext(e)));
		if(status == NOT_MATCHED) break;
	}
	return status == MATCHED ? 1 : 0;
}
/** Match {re} by performing a DFS in-order search on each character.
 @param re, arg: If null, returns null.
 @return The first point it matches or null if it doesn't. */
const char *PatternMatch(const struct Pattern *const re, const char *const arg){
	const struct MachineVertex *root;
	const char *a;
	if(!re || !arg) return 0;
	root = MachineDigraphGetRoot(&re->graph);
	for(a = arg; *a; a++) if(match_here(root, a)) return a;
	return 0;
}

/** Appends {re} to {fp} in GraphViz format.
 @param re: If null, does nothing.
 @param fp: File pointer.
 @return Success.
 @throws {fprintf} errors: {IEEE Std 1003.1-2001}.
 @order O(|{vertices}| + |{edges}|)
 @allow */
int PatternOut(const struct Pattern *const re, FILE *const fp) {
	if(!re) return 0;
	return MachineDigraphOut(&re->graph, re->title, fp);
}



/*
 * The rest is for compiling a regular expression.
 */



/*
 * First, scan the input and decide what each thing means.
 */

/*void scan(struct Make *const make) {
	assert(make);
	
}*/

/**
 * Temporary nesting for compiling. Refers to index in the vertices pool.
 */
struct Nest { size_t v0i, v2i; const char *arg; };
#define POOL_NAME Nest
#define POOL_TYPE struct Nest
#define POOL_STACK
#include "Pool.h"
/** Private constructor.
 @param nest: A {NestPool}. Required.
 @param a: An already existing vertex index to use as the opening.
 @return Creates a new Nest or null.
 @throws {realloc} errors. */
static struct Nest *Nest(struct NestPool *const nest, const size_t v0i) {
	struct Nest *n;
	assert(nest);
	if(!(n = NestPoolNew(nest))) return 0;
	n->v0i = v0i;
	n->v2i = (size_t)-1; /* By agreed upon convention, this is null. */
	return n;
}

struct Make;
/** Definition of {MakeContext}. */
typedef void (*MakeContext)(struct Make *const);

/**
 * Temporary structure called on compiling regular expressions into DFAs. All
 * wrapped up one one object for convenience.
 */
struct Make {
	struct Pattern *re;
	struct NestPool nests;
	MakeContext context;
	const char *c_from, *c;
	enum {
		DONE = 1,
		ERRNO = 2,
		SYNTAX = 4,
		BRANCH = 8,
		EDGE = 16,
		OPEN = 32,
		CLOSE = 64
	} status;
};

/* Prototypes defined later, used now. */
static void normal_context(struct Make *const make);
static void escape_context(struct Make *const make);

/** Private initialiser. */
static int Make(struct Make *const make,
	struct Pattern *const re, const char *const compile) {
	struct MachineVertexLink *start;
	assert(make && re && compile && !MachineDigraphGetRoot(&re->graph));
	make->re = re;
	NestPool(&make->nests);
	make->context = &normal_context;
	make->c_from = 0;
	make->c = compile;
	make->status = 0;
	/* Set up starting state: implied parenthesis around all. */
	if(!(start = VertexPoolNew(&make->re->vs))) return 0;
	MachineDigraphPutVertex(&make->re->graph, &start->data);
	if(!Nest(&make->nests, VertexPoolIndex(&make->re->vs, start))) return 0;
	return 1;
}

/** Private destructor. */
static void Make_(struct Make *const make) {
	assert(make);
	NestPool_(&make->nests);
	make->context = 0;
}

/** .|\ (capturing group, nah) (lazy ? eh) (lookarounds, meh)
 @implements MakeContext */
static void normal_context(struct Make *const make) {
	assert(make && !make->status && NestPoolPeek(&make->nests) && make->c);
	printf("normal_context: %c (0x%x.)\n", *make->c, (unsigned)*make->c);
	switch(*make->c) {
	case '\\': make->context = &escape_context; break;
	case '|': make->status |= BRANCH; break;
	case '(': make->status |= OPEN; break;
	case ')': make->status |= CLOSE; break;
	case '*':
	case '+':
	case '?':
	case '^':
	case '$':
	case '{':
	case '}': break; /* @fixme Not implemented. */
	case '\0': make->status |= DONE, make->context = 0; break;
	default: if(!make->c_from) make->c_from = make->c; break; /*Start literal.*/
	}
}

/** \d (digit,intern) \w (word,_,number,>255?) \s any separator?
 \D \W \S \N(not a line break)
 @implements MakeContext */
static void escape_context(struct Make *const make) {
	assert(make && !make->status && NestPoolPeek(&make->nests) && make->c);
	printf("escape_context: %c (0x%x.)\n", *make->c, (unsigned)*make->c);
	switch(*make->c) {
		case '\0': make->status |= SYNTAX, make->context = 0; return;
		default: if(!make->c_from) make->c_from = make->c; break;
	}
	make->context = &normal_context;
}

/** {3}{2,7}{4,}
 @implements MakeContext */
/*static enum MakeStatus brace_context(struct Make *const make) {
	assert(make);
	return SUCCESS;
}*/

/** [] [^]
 @implements MakeContext *
static enum MakeStatus brackets_context(struct Make *const make) {
	assert(make);
	switch(*make->to) {
	case ']': make->context = &normal_context;
	case '\0': return SYNTAX;
	}
	return SUCCESS;
}*/

/** Gets the index of  */

/** Private: initialises {re} with {compile} and compiles. Called from
 \see{Pattern}.
 @return Success, otherwise {errno} will (probably) be set; it always
 initialises {re}. */
static int compile_re(struct Pattern *re, const char *const compile) {
	struct Make make;
	struct Nest *n;
	size_t v1i = 0;
	assert(re && compile);
	printf("compile_re: <%s>.\n", compile);
	if(!Make(&make, re, compile)) return Make_(&make), 0;
	do {
		assert((make.status & (ERRNO | SYNTAX | DONE)) == 0), make.status = 0;
		/* Main compiling loop. */
		make.context(&make);
		if(!make.status) continue;
		/* Something happened. Retrieve nesting level. */
		n = NestPoolPeek(&make.nests), assert(n);
		/* Any literals always add an edge. */
		if(make.c_from) make.status |= EDGE;
		/* Add onto the graph. */
		if(make.status & BRANCH && n->v2i == (size_t)-1) { /* Terminating v2. */
			struct MachineVertexLink *const v2 = VertexPoolNew(&re->vs);
			if(!v2) { make.status |= ERRNO, make.context = 0; break; }
			MachineDigraphPutVertex(&re->graph, &v2->data);
			n->v2i = VertexPoolIndex(&re->vs, v2);
			/* Force it to make a, possibly empty, edge, sometimes. */
			if(!(compile < make.c && make.c[-1] == ')')) make.status |= EDGE;
			printf("branch %c\n", *make.c);
		}
		if(make.status & EDGE) { /* Intermediary v1. */
			struct MachineEdge *edge;
			struct MachineVertex *v0, *v1;
			if(n->v2i != (size_t)-1) { /* v1 == v2 terminating. */
				struct MachineVertexLink *const v1l
					= VertexPoolGet(&re->vs, v1i = n->v2i);
				assert(v1l);
				v1 = &v1l->data;
			} else { /* Make an intermediary vertex, v1. */
				struct MachineVertexLink *const v1l = VertexPoolNew(&re->vs);
				if(!v1l) { make.status |= ERRNO, make.context = 0; break; }
				v1 = &v1l->data;
				v1i = VertexPoolIndex(&re->vs, v1l);
				MachineDigraphPutVertex(&re->graph, v1);
			}
			{ /* First vertex; after second because possibly invalidated. */
				struct MachineVertexLink *const v0l
					= VertexPoolGet(&re->vs, n->v0i);
				assert(v0l);
				v0 = &v0l->data;
			}
			/* The edge. */
			if(make.c_from && make.c_from < make.c) {
				struct Literals *const lit = Literals(&re->literals,
					make.c_from, make.c - make.c_from);
				if(!lit) { make.status |= ERRNO, make.context = 0; break; }
				edge = &lit->edge.data;
			} else {
				struct MachineEdgeLink *emp = Empty(&re->empties);
				if(!emp) { make.status |= ERRNO, make.context = 0; break; }
				edge = &emp->data;
			}
			/* Reset the literal. */
			make.c_from = 0;
			MachineDigraphPutEdge(edge, v0, v1);
			printf("edge %c\n", *make.c);
		}
		if(make.status & OPEN) { /* Open parenthesis. */
			assert(!(make.status & CLOSE));
			if(!Nest(&make.nests, v1i))
				{ make.status |= ERRNO, make.context = 0; break; }
			printf("open\n");
		}
		if(make.status & CLOSE) { /* Close parenthesis. */
			assert(!(make.status & OPEN));
			NestPoolPop(&make.nests);
			/* Too many ')'. */
			if(!NestPoolPeek(&make.nests))
				{ make.status |= SYNTAX, make.context = 0; break; }
			printf("close\n");
		}
		MachineDigraphSort(&make.re->graph);
	} while(make.context && (make.c++, 1));
	/* Verify the parentheses match. */
	if(!NestPoolPop(&make.nests) || NestPoolPeek(&make.nests))
		make.status |= SYNTAX;
	/* Catch(SYNTAX) -- set {errno}. */
	if(make.status & SYNTAX) assert(!(make.status & ERRNO)), errno = EILSEQ;
	/* Finally. */
	Make_(&make);
	printf("compile_re: <%s> make.status 0x%x\n", compile, make.status);
	return !(make.status & (ERRNO | SYNTAX));
}

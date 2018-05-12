/** 2017 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 Regex, \cite{Thompson1968Regular}, Cox2007
 \url{ https://swtch.com/~rsc/regexp/regexp1.html }. We don't take exactly the
 same approach. (I think?)
 \url{ http://users.pja.edu.pl/~jms/qnx/help/watcom/wd/regexp.html }
 \url{ http://www.cs.sfu.ca/~cameron/Teaching/384/99-3/regexp-plg.html }
 \url{ http://matt.might.net/articles/parsing-regex-with-recursive-descent/ }.

 <re> ::= <term> '|' <re> | <term>
 <term> ::= { <factor> }
 <factor> ::= <atom> <repeat> | <atom> | '^' | '$'
 <repeat> ::= '*' | '+' | '?' | '{' <number> [ ',' [ <number> ] ] '}'
 <atom> ::= <char> | '\' <char> | '(' <re> ')'

 <re> ::= <nestnch> | <piece>
 <nestnch> ::= <re> "|" <piece>
 <piece> ::= <concatenation> | <expression>
 <concatenation> ::= <piece> <expression>
 <expression> ::= <star> | <plus> | <atom>
 <star> ::=	<atom> "*"
 <plus> ::=	<atom> "+"
 <atom> ::= <group> | <any> | <bos> | <eos> | <char> | <set>
 <group> ::= "(" <re> ")"
 <any> ::= "."
 <bos> ::= "^"
 <eos> ::= "$"
 <char> ::= any non metacharacter | "\" metacharacter
 <set> ::= <positive-set> | <negative-set>
 <positive-set> ::= "[" <set-items> "]"
 <negative-set> ::= "[^" <set-items> "]"
 <set-items> ::= <set-item> | <set-item> <set-items>
 <set-items> ::= <range> | <char>
 <range> ::= <char> "-" <char>

 @title		Regex
 @author	Neil
 @std		C89 with C99 stdint.h
 @version	2018-04
 @since
 2018-04 Conglomerated into {Regex}. */

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
#include "Regex.h"

/* Pre-define these constants. */
struct Transition;
typedef void (*TransitionToString)(const struct Transition *, char(*const)[12]);

/**
 * Intermediary structure created when walking the graph of regex and the
 * remainder of the string that it's matched against. Used in \see{match_here}.
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
/** @implements TransitionToString */
static void transition_to_string(const struct Transition *t,char(*const a)[12]){
	t->vt->to_string(t, a);
}
/** @implements Match */
static const char *transition_match(struct Match *const match) {
	assert(match && match->edge && match->vertex && match->next);
	return match->edge->vt->match(match);
}
/* {Transition \in MachineVertex, MachineEdge, \in MachineDigraph}. */
#define DIGRAPH_NAME Machine
#define DIGRAPH_EDATA struct Transition
#define DIGRAPH_EDATA_TO_STRING &transition_to_string
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
/** @implements TransitionToString */
static void empty_to_string(const struct Transition *e, char (*const a)[12]) {
	strcpy(*a, "ε");
	(void)e;
}
/** @implements Match */
static const char *empty_match(const struct Match *const match) {
	return match->next;
}
static struct TransitionVt empty_vt
	= { "Empty", &empty_to_string, &empty_match };
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
static struct TransitionVt literals_vt
	= { "Literals", &literals_to_string, &literals_match };
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
 * {Regex} is composed of the things defined above.
 */
struct Regex {
	const char *title;
	struct MachineDigraph graph;
	struct VertexPool vs;
	struct EmptyPool empties;
	struct LiteralsPool literals;
};

/** Destructor.
 @param re: If null, does nothing, otherwise it is set to match zero characters
 and frees the memory. */
void Regex_(struct Regex **const pre) {
	struct Regex *re;
	if(!pre || !(re = *pre)) return;
	printf("~Regex<%s>.\n", re->title);
	LiteralsPoolForEach(&re->literals, &Literals_);
	LiteralsPool_(&re->literals);
	EmptyPool_(&re->empties);
	VertexPool_(&re->vs);
	MachineDigraph_(&re->graph);
	*pre = 0;
}

/*
 * Temporary nesting for compiling.
 * Refers to index in the vertices pool.
 */
struct Nest { size_t v0i, v2i; };
#define POOL_NAME Nest
#define POOL_TYPE struct Nest
#define POOL_STACK
#include "Pool.h"
/** @param nest: A {NestPool}. Required.
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

#if 0
/** Used to return status in {compile_re}. */
enum MakeReStatus { SUCCESS, RESOURCES, SYNTAX };
struct MakeRe;
/** Used in {MakeRe} for different contexts. */
typedef enum MakeReStatus (*MakeReContext)(struct MakeRe *const);
#endif

/** Called from \see{Regex}.
 @return Success, otherwise {errno} will (probably) be set. */
static int re_compile(struct Regex *re, const char *const compile) {
	struct NestPool nest;
	enum { SUCCESS, RESOURCES, SYNTAX } e = SUCCESS;

	NestPool(&nest);
	printf("m_compile: <%s>.\n", compile);
	do { /* try */
		size_t v1i = 0;
		int is_closing = 0; /* @fixme Ugly. */
		struct Nest *n;
		const char *c = compile, *c_start = 0;
		enum { DONE = 1, FINAL = 2, EDGE = 4, OPEN = 8, CLOSE = 16 } flags;
		{ /* Starting vertex and implied nestle. */
			struct MachineVertexLink *start = VertexPoolNew(&re->vs);
			if(!start || !Nest(&nest, VertexPoolIndex(&re->vs, start)))
				{ e = RESOURCES; break; }
			MachineDigraphPutVertex(&re->graph, &start->data);
		}
		do { /* For each byte. */
			printf("__%c (%d)__\n", *c, (int)*c);
			/* Set the flags. */
			flags = 0;
			switch(*c) {
				case '|': flags |= FINAL; break;
				case '(': flags |= OPEN; break;
				case ')': flags |= CLOSE; break;
				case '\0': flags |= DONE; break;
				default: if(!c_start) c_start = c; break;
			}
			if(!flags) continue;
			/* Retrieve nesting level; any literals always add an edge. */
			n = NestPoolPeek(&nest), assert(n);
			if(c_start) flags |= EDGE;
			/* Add onto the graph. */
			if(flags & FINAL && n->v2i == (size_t)-1) { /* Terminating v2. */
				struct MachineVertexLink *const v2 = VertexPoolNew(&re->vs);
				if(!v2) { e = RESOURCES; break; }
				MachineDigraphPutVertex(&re->graph, &v2->data);
				n->v2i = VertexPoolIndex(&re->vs, v2);
				if(!is_closing) flags |= EDGE;
				printf("final %c\n", *c);
			}
			is_closing = 0;
			if(flags & EDGE) { /* Intermediary v1. */
				struct MachineEdge *edge;
				struct MachineVertex *v0, *v1;
				if(n->v2i != (size_t)-1) { /* v1 == v2 terminating. */
					struct MachineVertexLink *const v1l
						= VertexPoolGet(&re->vs, v1i = n->v2i);
					assert(v1l);
					v1 = &v1l->data;
				} else { /* Make an intermediary vertex, v1. */
					struct MachineVertexLink *const v1l = VertexPoolNew(&re->vs);
					if(!v1l) { e = RESOURCES; break; }
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
				if(c_start && c_start < c) {
					struct Literals *lit;
					if(!(lit = Literals(&re->literals, c_start, c - c_start)))
						{ e = RESOURCES; break; }
					edge = &lit->edge.data;
				} else {
					struct MachineEdgeLink *emp;
					if(!(emp = Empty(&re->empties)))
						{ e = RESOURCES; break; }
					edge = &emp->data;
				}
				c_start = 0;
				MachineDigraphPutEdge(edge, v0, v1);
				printf("edge %c\n", *c);
			}
			if(flags & OPEN) { /* Open parenthesis. */
				assert(!(flags & CLOSE) /*&& flags & EDGE @fixme Fails. Are you sure we need this? */);
				if(!Nest(&nest, v1i)) { e = RESOURCES; break; }
				printf("open\n");
			}
			if(flags & CLOSE) { /* Close parenthesis. */
				assert(!(flags & OPEN));
				NestPoolPop(&nest);
				if(!NestPoolPeek(&nest)) { e = SYNTAX; break; }
				printf("close\n");
				is_closing = 1;
			}
		} while(c++, !(flags & DONE)); /* For @ byte. */
		if(e) break;
		/* Verify the parentheses match. */
		if(!NestPoolPop(&nest) || NestPoolPeek(&nest)) { e = SYNTAX; break; }
	} while(0); if(e == SYNTAX) { /* catch(SYNTAX) */
		errno = EILSEQ;
	} { /* finally */
		NestPoolClear(&nest);
	}
	printf("m_compile: e %d\n", e);
	return !e;
}
/** Compiles a regular expression.
 @param compile: If null, returns null. Otherwise, this is a null-terminated
 modified UTF-8 string that gets compiled into a regular expression.
 @return The regular expression. Requires freeing with \see{Regex_}.
 @throws {malloc/realloc} errors: {IEEE Std 1003.1-2001}.
 @throws EILSEQ: The {re} is not understood, (required since 1994 Amendment 1
 to C89 standard.) */
struct Regex *Regex(const char *const compile) {
	struct Regex *re;
	if(!compile || !(re = malloc(sizeof *re))) return 0;
	re->title = compile;
	MachineDigraph(&re->graph);
	VertexPool(&re->vs, &MachineDigraphVertexMigrateAll, &re->graph);
	EmptyPool(&re->empties);
	LiteralsPool(&re->literals);
	if(!re_compile(re, compile)) Regex_(&re);
	return re;
}

/** Checks if the {root} of the DFA matches {arg}. Called in \see{RegexMatch}.
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
const char *RegexMatch(const struct Regex *const re, const char *const arg) {
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
int RegexOut(const struct Regex *const re, FILE *const fp) {
	if(!re) return 0;
	return MachineDigraphOut(&re->graph, fp);
}



















#if 0


/* Private functions later on. */
static int init_compile_re(struct Regex *const re, const char *const compile);
static int match_here(const struct MachineVertex *const root,
	const char *const arg);


/**
 * Temporary structure called on compiling regular expressions into DFAs. All
 * wrapped up one one object for convenience.
 */
struct MakeRe {
	struct Regex *re;
	struct NestPool nests;
	MakeReContext context;
	const char *from, *to;
	struct MachineVertex *v;
};

/* Prototypes defined later. */
static enum MakeReStatus advance_literals(struct MakeRe *const make);
static enum MakeReStatus normal_context(struct MakeRe *const make);
static enum MakeReStatus escape_context(struct MakeRe *const make);

/** Private: initialises {re} with {compile} and compiles.
 @return Success, otherwise (probably) {errno} is set; it always initialises
 {re}. */
static int init_compile_re(struct Regex *const re, const char *const compile) {
	struct MakeRe make;
	enum MakeReStatus e = SUCCESS;
	assert(re && compile);

	/* Initialise {re}. */
	re->title = compile;
	MachineDigraph(&re->states);
	MachineVertexPool(&re->vertices, &vertex_migrate, &make);
	EmptyPool(&re->empties, &edge_migrate, &make);
	LiteralsPool(&re->literals, &edge_migrate, &make);

	/* Initialise {make}, the {re} scaffolding. */
	make.re = re;
	NestPool(&make.nests);
	make.context = &normal_context;
	make.from = make.to = compile;
	make.v = 0;

	printf("Regex<%s> compiling.\n", re->title);
	do { /* Try. */
		{ /* Set up starting state: implied parenthesis around all. */
			struct Nest *nest;
			if(!(make.v = MachineVertexPoolNew(&make.re->vertices))
				|| !(nest = NestPoolNew(&make.nests)))
				{ e = RESOURCES; break; }
			MachineDigraphVertex(&make.re->states, make.v);
			Nest(nest, make.to, make.v);
		}
		do { /* Main compiling loop. */
			if((e = make.context(&make)) != SUCCESS) break;
		} while(make.context && (make.to++, 1));
		if(e) break;
		/* One last call to clean up the rest. */
		if((e = advance_literals(&make)) != SUCCESS) break;
		/* Make sure the parentheses are matched. */
		if(!NestPoolPop(&make.nests) || NestPoolPeek(&make.nests))
			{ e = SYNTAX; break; }
	} while(0); if(e == SYNTAX) { /* Catch(SYNTAX) -- set {errno}. */
		errno = EILSEQ;
	} { /* Finally. */
		NestPool_(&make.nests);
	}
	return !e;
}

/** .|\ (capturing group, nah) (lazy ? eh) (lookarounds, meh)
 @implements MakeReContext */
static enum MakeReStatus normal_context(struct MakeRe *const make) {
	struct Nest *nest;
	enum MakeReStatus e = SUCCESS;
	assert(make && NestPoolPeek(&make->nests));
	/*printf("char: %c (0x%x.)\n", *make->to, (unsigned)*make->to);*/
	switch(*make->to) {
		case '\\':
			make->context = &escape_context; break;
		case '|':
			if((e = advance_literals(make)) != SUCCESS) break;
			make->from = make->to + 1;
			make->v = NestPoolPeek(&make->nests)->start;
			break;
		case '*':
		case '+':
		case '?':
		case '^':
		case '$':
		case '{':
		case '}': break; /* @fixme Not implemented. */
		case '(':
			if((e = advance_literals(make)) != SUCCESS) break; /* Clean up. */
			if(!(nest = NestPoolNew(&make->nests))) return RESOURCES;
			Nest(nest, make->from, make->v);
			printf("normal_context: '(': <%s>\n", make->from);
			break;
		case ')':
			if((e = advance_literals(make)) != SUCCESS) break;
			if(!(nest = NestPoolPop(&make->nests))
			   || !NestPoolPeek(&make->nests)) return SYNTAX;
			printf("normal_context: ')': <%s>\n", make->from);
			break;
		case '\0': make->context = 0; break;
		default: break;
	}
	return e;
}

/** \d (digit,intern) \w (word,_,number,>255?) \s any separator?
 \D \W \S \N(not a line break)
 @implements MakeReContext */
static enum MakeReStatus escape_context(struct MakeRe *const make) {
	assert(make);
	switch(*make->to) {
		case '\0': return SYNTAX;
			/*default:*/
	}
	make->context = &normal_context;
	return SUCCESS;
}

/** {3}{2,7}{4,}
 @implements MakeReContext */
/*static enum MakeReStatus brace_context(struct MakeRe *const make) {
	assert(make);
	return SUCCESS;
}*/

/** [] [^]
 @implements MakeReContext *
static enum MakeReStatus brackets_context(struct MakeRe *const make) {
	assert(make);
	switch(*make->to) {
	case ']': make->context = &normal_context;
	case '\0': return SYNTAX;
	}
	return SUCCESS;
}*/

#endif


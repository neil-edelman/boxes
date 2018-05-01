/** 2017 Neil Edelman, distributed under the terms of the MIT License;
 see readme.txt, or \url{ https://opensource.org/licenses/MIT }.

 Regex, \cite{Thompson1968Regular}, Cox2007
 \url{ https://swtch.com/~rsc/regexp/regexp1.html }. We don't take exactly the
 same approach.
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

/**
 * Intermediary structure created when walking the graph of regex and the
 * remainder of the string that it's matched against.
 */
struct Match {
	const struct Transition *t;
	const struct StateVertex *vertex;
	const char *next;
};

/**
 * Transition virtual table, referenced from {Transition}.
 */
struct Transition;
typedef const char *(*Match)(const struct Match *const);
typedef void (*TransitionToString)(const struct Transition *, char(*const)[12]);
struct TransitionVt {
	const char *debug;
	const Match match;
	const TransitionToString to_string;
};

/**
 * Abstract {Transition}. This forms extra data on the edge of the digraph.
 */
struct Transition {
	const struct TransitionVt *vt;
};
/** Constructor; only called from it's children. */
static void Transition(struct Transition *const t,
	const struct TransitionVt *const vt) {
	assert(t && vt);
	t->vt = vt;
	printf("super transition %s\n", vt->debug);
}
/** @implements TransitionToString */
static void transition_to_string(const struct Transition *t,char(*const a)[12]){
	t->vt->to_string(t, a);
}
/** @implements Match */
static const char *transition_match(struct Match *const match) {
	assert(match && match->t && match->vertex && match->next);
	return match->t->vt->match(match);
}
#define DIGRAPH_NAME State
#define DIGRAPH_EDATA struct Transition
#define DIGRAPH_EDATA_TO_STRING &transition_to_string
#include "../src/Digraph.h" /* StateDigraph, StateVertex, StateEdge. */

/*
 * {StateVertex} container.
 */
#define POOL_NAME StateVertex
#define POOL_TYPE struct StateVertex
#define POOL_MIGRATE_ALL struct StateDigraph
#include "Pool.h"

/**
 * {Literals} extends {Transition} as {StateEdgeLink}.
 */
struct Literals {
	struct StateEdgeLink edge;
	char *text;
	size_t text_size;
};
/** {container_of} (probably does nothing.) */
static const struct Literals *literals_holds_transition(
	const struct Transition *const t) {
	return (const struct Literals *)(const void *)
	((const char *)t
		- offsetof(struct StateEdge, info)
		- offsetof(struct StateEdgeLink, data)
		- offsetof(struct Literals, edge));
}
/** @implements Match */
static const char *literals_match(const struct Match *const match) {
	const struct Literals *l = literals_holds_transition(match->t);
	if(memcmp(l->text, match->next, l->text_size)) return 0;
	return match->next + l->text_size;
}
/** @implements TransitionToString */
static void literals_to_string(const struct Transition *t, char (*const a)[12]){
	sprintf(*a, "%.11s", literals_holds_transition(t)->text);
}
static struct TransitionVt literals_vt
	= { "Literals", literals_match, literals_to_string };
/** Destructor.
 @implements <Literals>Action */
static void Literals_(struct Literals *const l) {
	if(!l) return;
	printf("~Literals: freeing %s.\n", l->text);
	free(l->text);
	l->text = 0;
	l->text_size = 0;
}
/** Constructor.
 @param l: This is instantiated; any data will be erased.
 @param arg, arg_size: The literal. A copy of this value is taken.
 @param arg_size: Must be greater then zero.
 @return Success.
 @throws {malloc} errors. */
static int Literals(struct Literals *const l, const char *const arg,
	size_t arg_size) {
	const char *a, *a_end;
	char *t;
	int is_escaped;
	assert(l && arg && arg_size);
	Transition(&l->edge.data.info, &literals_vt);
	l->text = 0;
	l->text_size = 0;
	/* Copy the literals; null terminator even thought it's not really used. */
	if(!(l->text = malloc(sizeof *arg * (arg_size + 1)))) return 0;
	/* This was before escaping chars:
	 memcpy(l->text, arg, arg_size);
	 l->text[arg_size] = '\0';
	 l->text_size = arg_size;*/
	for(a=arg, a_end=a+arg_size, t = l->text, is_escaped = 0; a < a_end; a++) {
		assert(*a != '\0');
		if(*a == '\\' && !is_escaped) { is_escaped = 1; continue; }
		*t++ = *a;
		is_escaped = 0;
	}
	*t = '\0';
	l->text_size = (size_t)(t - l->text);
	printf("Literals %p: <%s>:%lu\n", (void *)l, l->text,
		(unsigned long)l->text_size);
	return 1;
}
#define POOL_NAME Literals
#define POOL_TYPE struct Literals
#define POOL_MIGRATE_ALL struct StateDigraph
#include "Pool.h"

/**
 * {Sieve} extends {Transition}.
 */
#if 0 /* Not used yet. */
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
static void sieve_add(struct State *const state, const char match) {
	assert(state);
	bit_set(state->bit, match);
}
/** Inverts the matches that will get though {state}.
 @fixme UTF-8 does not do as expecected. */
static void sieve_invert(struct State *const state) {
	assert(state);
	bit_invert(state->bit);
}
/** Tests {match} against {state}. */
static int sieve_match(struct State *const state, const char match) {
	assert(state);
	return bit_test(state->bit, match);
}
#define POOL_NAME Sieve
#define POOL_TYPE struct Sieve
#define POOL_MIGRATE_ALL struct StateDigraph
#include "Pool.h"
#endif /* Not used. */



/**
 * Regular expression contains all the above.
 */
struct Regex {
	const char *title;
	struct StateDigraph states;
	struct StateVertexPool vertices;
	struct LiteralsPool literals;
};



struct Nest {
	const char *c;
	struct StateVertex *start;
};
static void Nest(struct Nest *const nest, const char *const c,
	struct StateVertex *const start) {
	assert(nest && c && start);
	nest->c = c;
	nest->start = start;
}
/* Used in {MakeRe} for parentheses matching. */
#define POOL_NAME Nest
#define POOL_TYPE struct Nest
#define POOL_STACK
#include "Pool.h"

/** Used to return status in {MakeRe} from {MakeReContext}. */
enum MakeReStatus { SUCCESS, RESOURCES, SYNTAX };

struct MakeRe;
/** Used in {MakeRe} for context. */
typedef enum MakeReStatus (*MakeReContext)(struct MakeRe *const);

/**
 * Temporary structure called on compiling regular expressions into DFAs. All
 * wrapped up one one object for convenience.
 */
struct MakeRe {
	struct Regex *re;
	struct NestPool nests;
	MakeReContext context;
	const char *from, *to;
	struct StateVertex *v;
};

/** Check for literals at the end of a sequence. If it finds any literals
 between {make.from} and {make.to}, it builds an literals edge from {make.v} to
 a new vertex, and makes the new vertex {make.v}.
 @return {MakeReStatus}.
 @throws {realloc} errors. */
static enum MakeReStatus make_literals(struct MakeRe *const make) {
	assert(make);
	if(make->from < make->to) { /* At least one byte. */
		struct Literals *const lit = LiteralsPoolNew(&make->re->literals);
		struct StateVertex *const v = StateVertexPoolNew(&make->re->vertices);
		StateDigraphVertex(&make->re->states, v);
		if(!v || !lit || !Literals(lit, make->from, make->to - make->from))
			return RESOURCES;
		StateDigraphEdge(&lit->edge.data, make->v, v);
		make->v = v;
	}
	return SUCCESS;
}

/** {3}{2,7}{4,}
 @implements MakeReContext */
/*static enum MakeReStatus nestces_context(struct MakeRe *const make) {
	assert(make);
	return SUCCESS;
}*/

/** [] [^]
 @implements MakeReContext *
static enum MakeReStatus nestckets_context(struct MakeRe *const make) {
	assert(make);
	switch(*make->to) {
	case ']': make->context = &normal_context;
	case '\0': return SYNTAX;
	}
	return SUCCESS;
}*/

/** \d (digit,intern) \w (word,_,number,>255?) \s any separator?
 \D \W \S \N(not a line break)
 @implements MakeReContext */
static enum MakeReStatus escape_context(struct MakeRe *const make) {
	assert(make);
	switch(*make->to) {
		case '\0': return SYNTAX;
		/*default:*/
	}
	return SUCCESS;
}

/** .|\ (capturing group, nah) (lazy ? eh) (lookarounds, meh)
 @implements MakeReContext */
static enum MakeReStatus normal_context(struct MakeRe *const make) {
	struct Nest *nest;
	struct StateVertex *vtx;
	enum MakeReStatus e = SUCCESS;
	assert(make);
	/*printf("char: %c (0x%x.)\n", *make->to, (unsigned)*make->to);*/
	switch(*make->to) {
		case '\\':
			make->context = &escape_context; break;
		case '|':
			vtx = make->v;
			if((e = make_literals(make)) != SUCCESS) break;
			make->from = make->to + 1;
			make->v = vtx;
			break;
		case '*':
		case '+':
		case '?':
		case '{':
		case '}': break; /* @fixme Not implemented. */
		case '(':
			if((e = make_literals(make)) != SUCCESS) break; /* Clean up. */
			if(!(vtx = StateVertexPoolNew(&make->re->vertices))
				|| !(nest = NestPoolNew(&make->nests))) return RESOURCES;
			StateDigraphVertex(&make->re->states, vtx);
			Nest(nest, make->to + 1, vtx);
			make->v = vtx;
			make->from = make->to + 1;
			break;
		case ')':
			if(!(nest = NestPoolPop(&make->nests))) return SYNTAX;
			if((e = make_literals(make)) != SUCCESS) break;
			make->from = make->to + 1;
			break;
		case '\0': make->context = 0; break;
		default: break;
	}
	return e;
}

/** Initialises {make} and clears {states}.
 @return {MakeReStatus}. */
static enum MakeReStatus MakeRe(struct MakeRe *const make,
	struct Regex *const re, const char *const compile) {
	assert(make && re && compile
		&& !StateVertexListFirst(&re->states.vertices));
	/* Initialise. */
	make->re = re;
	NestPool(&make->nests);
	make->context = &normal_context;
	make->from = make->to = compile;
	make->v = 0;
	/* Set up resources: implied parenthesis around all; starting state. */
	if(!NestPoolNew(&make->nests)
	   || !(make->v = StateVertexPoolNew(&re->vertices))) return RESOURCES;
	StateDigraphVertex(&re->states, make->v);
	printf("MakeRe<%s>: state machine ready.\n", compile);
	return SUCCESS;
}
/** Called from \see{Regex}.
 @returns Success, otherwise {errno} (may) be set. */
static int regex_compile(struct Regex *re, const char *const compile) {
	struct MakeRe make;
	enum MakeReStatus e = SUCCESS;
	assert(re && compile && !StateVertexListFirst(&re->states.vertices));
	printf("Regex<%s>::regex_compile.\n", compile);
	do { /* Try. */
		if((e = MakeRe(&make, re, compile)) != SUCCESS) break;
		do {
			if((e = make.context(&make)) != SUCCESS) break;
			if(!make.context) break;
			make.to++;
		} while(1);
		if(e) break;
		/* One last call to clean up the rest. */
		if((e = make_literals(&make)) != SUCCESS) break;
		/* Make sure the parentheses are matched. */
		if(!NestPoolPop(&make.nests) || NestPoolPeek(&make.nests))
			{ e = SYNTAX; break; }
	} while(0); {
		NestPool_(&make.nests);
	}
	/* Syntax errors are our own errors, so we have to set {errno} ourselves. */
	if(e == SYNTAX) { printf("syntax?\n"); errno = EILSEQ; }
	printf("regex_compile: e %d.\n", e);
	return !e;
}

/** Destructor. One can destruct anything in a valid state, including null and
 zero, it just does nothing.
 @param re: If null, does nothing, otherwise it is set to match zero characters
 and frees the memory. */
void Regex_(struct Regex **const pre) {
	struct Regex *re;
	if(!pre || !(re = *pre)) return;
	printf("~Regex<%s>.\n", re->title);
	LiteralsPoolForEach(&re->literals, &Literals_);
	LiteralsPool_(&re->literals);
	StateVertexPool_(&re->vertices);
	StateDigraph_(&re->states);
	free(re);
	*pre = 0;
}

/** Compiles a regular expression.
 @param compile: If null, returns null. Otherwise, this is a null-terminated
 modified UTF-8 string that gets compiled into a regular expression.
 @return The regular expression. Requires freeing with \see{Regex_}.
 @throws {malloc/realloc} errors: {IEEE Std 1003.1-2001}.
 @throws EILSEQ: The {re} could not be compiled, (required since 1994
 Amendment 1 to C89 standard.) */
struct Regex *Regex(const char *const compile) {
	struct Regex *re;
	if(!compile || !(re = malloc(sizeof *re))) return 0;
	re->title = compile;
	StateDigraph(&re->states);
	/* @fixme Also temp {Nest} pointers! */
	StateVertexPool(&re->vertices, &StateDigraphVertexMigrateAll, &re->states);
	LiteralsPool(&re->literals, &StateDigraphEdgeMigrateAll, &re->states);
	if(!regex_compile(re, compile)) Regex_(&re);
	return re;
}

/** Checks if the {root} of the DFA matches {arg}. Called in \see{RegexMatch}.
 @return The end of the match if it was a match or null.
 @fixme implements <State,char*>Predicate,
 if(!(e = StateEdgeListMatchShortCircuit(&s->out, &no_match, match))) would be
 nice. List doesn't have interfaces . . . yet? */
static int match_here(const struct StateVertex *const root,
	const char *const arg) {
	struct Match m;
	struct StateEdge *e;
	const char *end;
	enum { NOT_MATCHED, MATCHED } status = MATCHED;
	m.t = 0, m.vertex = root, m.next = arg;
	while(m.vertex) {
		/* Finished when the vertex has degree zero. */
		if(!(e = StateEdgeListFirst(&m.vertex->out))) break;
		status = NOT_MATCHED;
		do { /* Loop through all the edges out of this vertex. */
			m.t = &e->info;
			if(!(end = transition_match(&m))) continue;
			m.vertex = e->to, m.next = end, status = MATCHED;
			break;
		} while((e = StateEdgeListNext(e)));
		if(status == NOT_MATCHED) break;
	}
	return status == MATCHED ? 1 : 0;
}
/** Match {re}.
 @return The first point it matches or null if it doesn't. */
const char *RegexMatch(const struct Regex *const re, const char *const arg) {
	const struct StateVertex *root;
	const char *a;
	if(!re || !arg) return 0;
	root = StateDigraphGetRoot(&re->states);
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
	return StateDigraphOut(&re->states, fp);
}

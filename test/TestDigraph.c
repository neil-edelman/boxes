/** Unit test.

 @title		TestDigraph
 @author	Neil
 @std		C89/90
 @version	1.0; 2018-04
 @since		1.0; 2018-04 */

#include <stddef.h>	/* ptrdiff_t offset_of */
#include <stdlib.h> /* EXIT_ rand */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* memcmp strcmp */
#include <time.h>	/* clock */
#include <limits.h>	/* INT_MAX */
#include "Orcish.h"

/* Test Blank. */

#define DIGRAPH_NAME Blank
#define DIGRAPH_TEST
#include "../src/Digraph.h"

/* Test Colour. */

/* {Colour} is an {enum} that's used on {Edge}. */
enum Colour { White , Silver, Gray, Black, Red, Maroon, Bisque, Wheat, Tan,
	Sienna, Brown, Yellow, Khaki, Gold, Olive, Lime, Green, Aqua, Cyan, Teal,
	Salmon, Orange, Powder, Sky, Steel, Royal, Blue, Navy, Fuchsia, Pink,
	Purple };
static const char *const colour_names[] = { "White", "Silver", "Gray", "Black",
	"Red", "Maroon", "Bisque", "Wheat", "Tan", "Sienna", "Brown", "Yellow",
	"Khaki", "Gold", "Olive", "Lime", "Green", "Aqua", "Cyan", "Teal",
	"Salmon", "Orange", "Powder", "Sky", "Steel", "Royal", "Blue", "Navy",
	"Fuchsia", "Pink", "Purple" }; /* max 11 letters */
static const size_t colour_size = sizeof colour_names / sizeof *colour_names;
/** @implements <Colour>ToString */
static void Colour_to_string(const enum Colour colour, char (*const a)[12]) {
	assert(colour < colour_size);
	sprintf(*a, "%s", colour_names[colour]);
}
/** @implements <Colour>Action */
static void Colour_filler(enum Colour *const colour) {
	assert(colour);
	*colour = (enum Colour)(float)(rand() / (RAND_MAX + 1.0) * colour_size);
}

/* Vertex. */
struct V {
	char value[12];
};
/** @implements <V>ToString */
static void V_to_string(const struct V *v, char (*const a)[12]) {
	sprintf(*a, "%.11s", v->value);
}
/** @implements <V>Action */
static void V_filler(struct V *const v) {
	Orcish(v->value, sizeof v->value);
}

/* Edge. */
struct E {
	enum Colour colour;
};
/** @implements <E>ToString */
static void E_to_string(const struct E *e, char (*const a)[12]) {
	Colour_to_string(e->colour, a);
}
/** @implements <E>Action */
static void E_filler(struct E *const e) {
	Colour_filler(&e->colour);
}

#define DIGRAPH_NAME Colour
#define DIGRAPH_VDATA struct V
#define DIGRAPH_EDATA struct E
#define DIGRAPH_VDATA_TO_STRING &V_to_string
#define DIGRAPH_EDATA_TO_STRING &E_to_string
#define DIGRAPH_VDATA_TEST &V_filler
#define DIGRAPH_EDATA_TEST &E_filler
#define DIGRAPH_TEST
#include "../src/Digraph.h"






/* Regex. \url{ https://swtch.com/~rsc/regexp/regexp1.html }. */

struct Regex;
void Regex_(struct Regex **const pre);
struct Regex *Regex(const char *const compile);
const char *RegexMatch(const struct Regex *const re, const char *const match);
int RegexOut(const struct Regex *const re, FILE *const fp);

/** Intermediary structures created when walking the graph of regex and
 the remainder of the string that it's matched against. */
struct Match {
	const struct Transition *t;
	const struct StateVertex *vertex;
	const char *next;
};

/**
 * Transition virtual table, referenced from {Transition}. This forms the extra
 * data on the pointed to by the edges of the digraph.
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
	printf("Subconstructor Transition %s\n", vt->debug);
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
	unsigned text_size;
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
 @return Success.
 @throws {malloc} errors. */
static int Literals(struct Literals *const l, const char *const match,
	unsigned match_size) {
	assert(l && match && match_size);
	Transition(&l->edge.data.info, &literals_vt);
	l->text = 0;
	l->text_size = 0;
	/* Copy the literals; null terminator even thought it's not really used. */
	if(!(l->text = malloc(sizeof *match * (match_size + 1)))) return 0;
	memcpy(l->text, match, match_size);
	l->text[match_size] = '\0';
	l->text_size = match_size;
	printf("Literals %p: <%s>:%lu\n", (void *)l, l->text, (unsigned long)l->text_size);
	return 1;
}
#define POOL_NAME Literals
#define POOL_TYPE struct Literals
#define POOL_MIGRATE_ALL struct StateDigraph
#include "Pool.h"

/**
 * Regular expression contains all the above.
 */
struct Regex {
	int no;
	struct StateDigraph states;
	struct StateVertexPool vertices; /* Vertices are unadorned. */
	struct LiteralsPool literals;
};
/* Result of \see{regex_compile}. */
enum CompileResult { CR_SUCCESS, CR_RESOURCES_FAIL, CR_SYNTAX_FAIL };
/** Called from \see{Regex}. */
static enum CompileResult regex_compile(struct Regex *re,
	const char *const match) {
	struct StateVertex *start, *end;
	struct Literals *lit;
	const char *m = match;
	unsigned p = 0;
	int done = 0;
	while(*m) {
		p++, m++;
	}
	printf("compile: <%s> upto: %u.\n", match, p);
	do {
		if(!(start = StateVertexPoolNew(&re->vertices))) break;
		StateDigraphVertex(&re->states, start);
		if(!(end = StateVertexPoolNew(&re->vertices))) break;
		StateDigraphVertex(&re->states, end);
		if(!(lit = LiteralsPoolNew(&re->literals)) || !Literals(lit, match, p))
			break;
		StateDigraphEdge(&lit->edge.data, start, end);
		done = 1;
	} while(0); if(!done) return CR_RESOURCES_FAIL;
	return CR_SUCCESS;
}
/** Destructor. */
void Regex_(struct Regex **const pre) {
	struct Regex *re;
	if(!pre || !(re = *pre)) return;
	printf("~Regex: releasing #%d.\n", re->no);
	LiteralsPoolForEach(&re->literals, &Literals_);
	LiteralsPool_(&re->literals);
	StateVertexPool_(&re->vertices);
	StateDigraph_(&re->states);
}
/** Constructor. */
struct Regex *Regex(const char *const compile) {
	struct Regex *re;
	enum CompileResult cr;
	static int no;
	if(!compile) return 0;
	if(!(re = malloc(sizeof *re))) return 0;
	re->no = ++no;
	StateDigraph(&re->states);
	StateVertexPool(&re->vertices, &StateDigraphVertexMigrateAll, &re->states);
	LiteralsPool(&re->literals, &StateDigraphEdgeMigrateAll, &re->states);
	printf("Compiling %d: <%s>.\n", re->no, compile);
	cr = regex_compile(re, compile);
	switch(cr) {
		case CR_SUCCESS: break;
		case CR_SYNTAX_FAIL: errno = EILSEQ;
		case CR_RESOURCES_FAIL: Regex_(&re);
	}
	return re;
}
/** Checks the outbound edges from {m->vertex} (part of a compiled DFA) for a
 match on the string {m->next}.
 @return The end of the match if it was a match or null.
 @fixme implements <State,char*>Predicate,
 if(!(e = StateEdgeListMatchShortCircuit(&s->out, &no_match, match))) would be
 nice. List doesn't have interfaces . . . yet? */
static int match_here(const struct StateVertex *const root, const char *const arg) {
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

/** @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = (unsigned)clock();
	struct Regex *re = 0;
	const char *const fn = "graphs/regex.gv";
	FILE *fp = 0;
	int done = 0;
	srand(seed), rand(), printf("Seed %u.\n", seed);
	BlankDigraphTest();
	ColourDigraphTest();
	/* Custom. */
	do {
		if(!(re = Regex("hi"))) break;
		if(!(fp = fopen(fn, "w"))) break;
		if(!RegexOut(re, fp)) break;
		printf("hithere <%s>; null <%s>; hihi <%s>.\n", RegexMatch(re, "hithere"), RegexMatch(re, "bye"), RegexMatch(re, "therehihi"));
		done = 1;
	} while(0); if(!done) {
		perror(fn);
	} {
		fclose(fp);
		Regex_(&re);
	}
	if(done) printf("Test success.\n\n");

	return done ? EXIT_SUCCESS : EXIT_FAILURE;
}

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
#define DIGRAPH_VERTEX struct V
#define DIGRAPH_EDGE struct E
#define DIGRAPH_VERTEX_TO_STRING &V_to_string
#define DIGRAPH_EDGE_TO_STRING &E_to_string
#define DIGRAPH_VERTEX_TEST &V_filler
#define DIGRAPH_EDGE_TEST &E_filler
#define DIGRAPH_TEST
#include "../src/Digraph.h"






/* Regex. \url{ https://swtch.com/~rsc/regexp/regexp1.html }. */

struct Regex;
void Regex_(struct Regex **const pre);
struct Regex *Regex(const char *const match);
const char *RegexMatch(const struct Regex *const re, const char *const match);
int RegexOut(const struct Regex *const re, FILE *const fp);

/**
 * State virtual table, referenced from {State}.
 */
struct State;
typedef int (*StateMatch)(const struct State *state, const char *const match);
typedef void (*StateToString)(const struct State *, char (*const)[12]);
struct StateVt {
	const StateMatch match;
	const StateToString to_string;
};

/**
 * Abstract {State}.
 */
struct State {
	const struct StateVt *vt;
};
/** Constructor; only called from it's children. */
static void State(struct State *const state, const struct StateVt *const vt) {
	assert(state && vt);
	state->vt = vt;
	printf("State %p\n", (void *)vt);
}
/** @implements StateMatch */
static int state_match(const struct State *const state, const char *const match) {
	return state->vt->match(state, match);
}
/** @implements StateToString */
static void state_to_string(const struct State *state, char (*const a)[12]) {
	state->vt->to_string(state, a);
}
#define DIGRAPH_NAME State
#define DIGRAPH_EDGE struct State
#define DIGRAPH_EDGE_TO_STRING &state_to_string
#include "../src/Digraph.h" /* StateDigraph, StateVertex, StateEdge. */

/**
 * {StateVertex} container.
 */

#define POOL_NAME StateVertex
#define POOL_TYPE struct StateVertex
#include "Pool.h"

/**
 * {Literals} extends {StateEdge}.
 */
struct Literals {
	struct StateEdge state_edge;
	char *text;
	unsigned text_size;
};
/** {container_of}. */
static const struct Literals *
	literals_holds_state(const struct State *const state) {
	return (const struct Literals *)(const void *)
		((const char *)state
		- offsetof(struct Literals, state_edge)
		- offsetof(struct StateEdge, info));
}
/** @implements StateMatch */
static int literals_match(const struct State *state, const char *const match) {
	const struct Literals *l = literals_holds_state(state);
	return !memcmp(l->text, match, l->text_size);
}
/** @implements StateToString */
static void literals_to_string(const struct State *state, char (*const a)[12]) {
	state->vt->to_string(state, a);
}
static struct StateVt literals_vt = { literals_match, literals_to_string };
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
	State(&l->state_edge.info, &literals_vt);
	l->text = 0;
	l->text_size = 0;
	/* Copy the literals, with a null terminator, (debug use.) */
	if(!(l->text = malloc(sizeof *match * (match_size + 1)))) return 0;
	memcpy(l->text, match, match_size);
	l->text[match_size] = '\0';
	l->text_size = match_size;
	printf("Literals: <%s>:%lu\n", l->text, (unsigned long)l->text_size);
	return 1;
}
#define POOL_NAME Literals
#define POOL_TYPE struct Literals
/*#define POOL_MIGRATE_EACH &sloth_migrate*/
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
	struct StateVertex *start;
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
		StateDigraphVertexAdd(&re->states, start);
		StateDigraphStart(&re->states, start);
		if(!(lit = LiteralsPoolNew(&re->literals))) break;
		if(!Literals(lit, match, p)) break;
		done = 1;
	} while(0); if(!done) return CR_RESOURCES_FAIL;
	return CR_SUCCESS;
}
/** Destructor. */
void Regex_(struct Regex **const pre) {
	struct Regex *re;
	if(!pre || (re = *pre)) return;
	printf("Releasing #%d.\n", re->no);
	LiteralsPoolForEach(&re->literals, &Literals_);
	LiteralsPool_(&re->literals);
	StateVertexPool_(&re->vertices);
	StateDigraph_(&re->states);
}
/** Constructor. */
struct Regex *Regex(const char *const match) {
	struct Regex *re;
	enum CompileResult cr;
	static int no;
	if(!match) return 0;
	if(!(re = malloc(sizeof *re))) return 0;
	re->no = ++no;
	StateDigraph(&re->states);
	StateVertexPool(&re->vertices);
	LiteralsPool(&re->literals);
	printf("Compiling %d: <%s>.\n", re->no, match);
	cr = regex_compile(re, match);
	switch(cr) {
		case CR_SUCCESS: break;
		case CR_SYNTAX_FAIL: errno = EILSEQ;
		case CR_RESOURCES_FAIL: Regex_(&re); printf("nooo\n");return 0;/*@fixme remove this*/
	}
	printf("Made %d.\n", re->no);
	return re;
}
/** Match {re}.
 @return The first point it matches or null if it doesn't. */
const char *RegexMatch(const struct Regex *const re, const char *const match) {
	if(!re || !match) return 0;
	/* @fixme */
	state_match(&re->states, match);
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
	const char *const fn = "test.gv";
	FILE *fp = 0;
	const char *yes, *no;
	int done = 0;
	srand(seed), rand(), printf("Seed %u.\n", seed);
	BlankDigraphTest();
	ColourDigraphTest();
	/* Custom. */
	do {
		if(!(re = Regex("hi"))) break;
		printf("okay, constructor good.\n");
		if(!(fp = fopen(fn, "w"))) break;
		if(!RegexOut(re, fp)) break;
		yes = RegexMatch(re, "hithere");
		no = RegexMatch(re, "bye");
		printf("yes <%s> no <%s>.\n", yes, no);
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

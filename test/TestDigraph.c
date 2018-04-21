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

/**
 * State virtual table, referenced from {State}.
 */
struct Regex;
struct State;
typedef int (*StateMatch)(const struct State *, const char *const match);
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
/** Constructor; only called from it's children. Should be called {state}, but
 someone else is using that? */
static void state(struct State *const s, const struct StateVt *const vt) {
	assert(s && vt);
	s->vt = vt;
}
/** @implements StateMatch */
static int state_match(const struct State *state, const char *const match) {
	return state->vt->match(state, match);
}
/** @implements StateToString */
static void state_to_string(const struct State *state, char (*const a)[12]) {
	state->vt->to_string(state, a);
}
#define DIGRAPH_NAME State
#define DIGRAPH_EDGE struct State
#define DIGRAPH_EDGE_TO_STRING &state_to_string
#include "../src/Digraph.h"

/**
 * {Literals} extends {State}.
 */
struct Literals {
	struct State state;
	char *text;
	unsigned text_size;
};
/** {container_of}. */
static const struct Literals *
	literals_holds_state(const struct State *const state) {
	return (const struct Literals *)(const void *)
		((const char *)state - offsetof(struct Literals, state));
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
/** Constructor. */
static void literals(struct Literals *const l, const char *const match,
	unsigned match_size) {
	assert(l && match && match_size);
	state(&l->state, &literals_vt);
	/* Copy the literals, with a null terminator, (not used, but who knows.) */
	l->text = malloc(sizeof *match * (match_size + 1));
}
#define POOL_NAME Literals
#define POOL_TYPE struct Literals
/*#define POOL_MIGRATE_EACH &sloth_migrate*/
#include "Pool.h"

/**
 * Regular expression contains all the above.
 */
struct Regex {
	struct StateDigraph states;
	struct LiteralsPool literals;
};
/** Literals. */
/*static struct Literals *Literals(struct Regex *re) {
	struct Literals *const l = LiteralsPoolNew(&re->literals);
	assert(re && match && match_size > 0);
	if(!l) return 0;
	return l;
}*/


/** @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = (unsigned)clock();

	srand(seed), rand(), printf("Seed %u.\n", seed);
	BlankDigraphTest();
	ColourDigraphTest();
	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}

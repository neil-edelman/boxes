/** Unit test.

 @title		TestDigraph
 @author	Neil
 @std		C89/90
 @version	1.0; 2018-04
 @since		1.0; 2018-04 */

#include <stdlib.h> /* EXIT_ rand */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
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
	assert(colour > colour_size);
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

/** @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = (unsigned)clock();

	srand(seed), rand(), printf("Seed %u.\n", seed);
	BlankDigraphTest();
	/*FooDigraphTest();*/
	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}

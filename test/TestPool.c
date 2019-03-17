/** Unit test. */

#include <stdlib.h> /* EXIT_ malloc free */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
#include <time.h>	/* clock */
#include <limits.h>	/* INT_MAX */
#include "Orcish.h"

/** Define {A}. */
struct A {
	char value[4];
};
static void A_to_string(const struct A *this, char (*const a)[12]);
/** @implements <Foo>Action */
static void A_filler(struct A *const this) {
	Orcish(this->value, sizeof this->value);
}
/** @implements <A>ToString */
static void A_to_string(const struct A *this, char (*const a)[12]) {
	sprintf(*a, "%.11s", this->value);
}
#define POOL_NAME A
#define POOL_TYPE struct A
#define POOL_TO_STRING &A_to_string
#define POOL_TEST &A_filler
#include "../src/StablePool.h"




/** Define {Foo}. */
struct Foo {
	int key;
	char value[32];
};
/** Assumes {key} is {[0, 99]}.
 @implements <Foo>ToString */
static void Foo_to_string(const struct Foo *this, char (*const a)[12]) {
	sprintf(*a, "%d%.9s", this->key, this->value);
}
/** @implements <Foo>Action */
static void Foo_filler(struct Foo *const this) {
	this->key = (int)(float)(rand() / (RAND_MAX + 1.0) * 99.0);
	Orcish(this->value, sizeof this->value);
}
#define POOL_NAME Foo
#define POOL_TYPE struct Foo
#define POOL_TO_STRING &Foo_to_string
#define POOL_TEST &Foo_filler
#include "../src/StablePool.h"




/* Class {Int} is a single {int}. */
/** Assumes {[-9 999 999 999, 99 999 999 999]}.
 @implements <Int>ToString */
static void Int_to_string(const int *this, char (*const a)[12]) {
	sprintf(*a, "%d", *this);
}
#include <limits.h>	/* INT_MAX */
#if INT_MAX > 9999999999
#define LIST_NUM_MAX 9999999999
#else
#define LIST_NUM_MAX INT_MAX
#endif
/** @implements <Int>Action */
static void Int_filler(int *const this) {
	*this = (int)(float)((2.0 * rand() / (RAND_MAX + 1.0) - 1.0) *LIST_NUM_MAX);
}
#undef LIST_NUM_MAX
#define POOL_NAME Int
#define POOL_TYPE int
#define POOL_TO_STRING &Int_to_string
#define POOL_TEST &Int_filler
#include "../src/StablePool.h"



/* Class {Colour} is an {enum}. */
enum Colour { White, Silver, Gray, Black, Red, Maroon, Bisque, Wheat, Tan,
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
static void Colour_to_string(const enum Colour *this, char (*const a)[12]) {
	assert(*this < colour_size);
	sprintf(*a, "%s", colour_names[*this]);
}
/** @implements <Colour>Action */
static void Colour_filler(enum Colour *const this) {
	*this = (enum Colour)(float)(rand() / (RAND_MAX + 1.0) * colour_size);
}
#define POOL_NAME Colour
#define POOL_TYPE enum Colour
#define POOL_TO_STRING &Colour_to_string
#define POOL_TEST &Colour_filler
#include "../src/StablePool.h"



/** Entry point.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = /*2532*//*11632*/(unsigned)clock();

	srand(seed), rand(), printf("Seed %u.\n", seed);
	APoolTest();
	FooPoolTest();
	IntPoolTest();
	ColourPoolTest();
	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}

/** Unit test.

 @file		TestStack
 @author	Neil
 @std		C89/90
 @version	1.0; 20xx-xx
 @since		1.0; 20xx-xx
 @param
 @fixme
 @deprecated */

#include <stdlib.h> /* EXIT_ malloc free */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
#include <time.h>	/* clock */
#include <limits.h>	/* INT_MAX */
#include "Orcish.h"

/** Define class {A} */
struct A {
	char value[4];
};
static void A_to_string(const struct A *this, char (*const a)[12]);
/** @implements <Foo>Action */
static void A_filler(struct A *const this) {
	Orcish(this->value, sizeof this->value);
}
#define STACK_NAME A
#define STACK_TYPE struct A
#define STACK_TO_STRING &A_to_string
#define STACK_TEST &A_filler
#include "../src/Stack.h"
/** Assumes {key} is {[0, 99]}.
 @implements <Foo>ToString */
static void A_to_string(const struct A *this, char (*const a)[12]) {
	sprintf(*a, "%s", this->value);
}

/** Define class {Foo} */
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
#define STACK_NAME Foo
#define STACK_TYPE struct Foo
#define STACK_TO_STRING &Foo_to_string
#define STACK_TEST &Foo_filler
#define STACK_DEBUG
#include "../src/Stack.h"

/* Class {Int} is a single {int}. */
/** Assumes {[-9 999 999 999, 99 999 999 999]}.
 @implements <Int>ToString */
static void Int_to_string(const int *this, char (*const a)[12]) {
	sprintf(*a, "%d", *this);
}
#if INT_MAX > 9999999999
#define LIST_NUM_MAX 9999999999
#else
#define LIST_NUM_MAX INT_MAX
#endif
#include <limits.h>	/* INT_MAX */
/** @implements <Int>Action */
static void Int_filler(int *const this) {
	*this = (int)(float)((2.0 * rand() / (RAND_MAX + 1.0) - 1.0) *LIST_NUM_MAX);
}
#undef LIST_NUM_MAX
#define STACK_NAME Int
#define STACK_TYPE int
#define STACK_TO_STRING &Int_to_string
#define STACK_TEST &Int_filler
#include "../src/Stack.h"

/* Class {Colour} is an {enum}. */
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
static void Colour_to_string(const enum Colour *this, char (*const a)[12]) {
	sprintf(*a, "%s", colour_names[*this]);
}
/** @implements <Colour>Action */
static void Colour_filler(enum Colour *const this) {
	*this = (enum Colour)(float)(rand() / (RAND_MAX + 1.0) * colour_size);
}
#define STACK_NAME Colour
#define STACK_TYPE enum Colour
#define STACK_TO_STRING &Colour_to_string
#define STACK_TEST &Colour_filler
#include "../src/Stack.h"

/** Entry point.
 @param argc: The number of arguments, starting with the programme name.
 @param argv: The arguments.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = (unsigned)clock();

	srand(seed), rand(), printf("Seed %u.\n", seed);
	AStackTest();
	FooStackTest();
	IntStackTest();
	ColourStackTest();
	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}

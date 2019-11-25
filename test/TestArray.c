/** Unit test of Array.c.

 @file		TestArray
 @author	Neil
 @std		C89/90 */

#include <stdlib.h> /* EXIT_ malloc free */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
#include <time.h>	/* clock */
#include <limits.h>	/* INT_MAX */
#include "Orcish.h"

/** Define `CharArray`. */
#define ARRAY_NAME Char
#define ARRAY_TYPE char
#define ARRAY_PRINTF
#include "../src/Array.h"

/** Define {A}. */
struct A {
	char value[4];
};
static void A_to_string(const struct A *this, char (*const a)[12]);
/** @implements <Foo>Action */
static void A_filler(struct A *const this) {
	Orcish(this->value, sizeof this->value);
}
#define ARRAY_NAME A
#define ARRAY_TYPE struct A
#define ARRAY_TO_STRING &A_to_string
#define ARRAY_TEST &A_filler
#include "../src/Array.h"
/** @implements <A>ToString */
static void A_to_string(const struct A *this, char (*const a)[12]) {
	sprintf(*a, "%.11s", this->value);
}



#define ARRAY_NAME B
#define ARRAY_TYPE struct A
#define ARRAY_TAIL_DELETE
#define ARRAY_TO_STRING &A_to_string
#define ARRAY_TEST &A_filler
#include "../src/Array.h"



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
#define ARRAY_NAME Foo
#define ARRAY_TYPE struct Foo
#define ARRAY_TO_STRING &Foo_to_string
#define ARRAY_TEST &Foo_filler
#include "../src/Array.h"



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
#define ARRAY_NAME Int
#define ARRAY_TYPE int
#define ARRAY_MIGRATE_ALL int /* This is bs. */
#define ARRAY_TO_STRING &Int_to_string
#define ARRAY_TEST &Int_filler
#include "../src/Array.h"



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
#define ARRAY_NAME Colour
#define ARRAY_TYPE enum Colour
#define ARRAY_TO_STRING &Colour_to_string
#define ARRAY_TEST &Colour_filler
#include "../src/Array.h"



#define ARRAY_NAME ColourStack
#define ARRAY_TYPE enum Colour
#define ARRAY_TO_STRING &Colour_to_string
#define ARRAY_TEST &Colour_filler
#define ARRAY_STACK
#include "../src/Array.h"



/** Entry point.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = (unsigned)clock();

	srand(seed), rand(), printf("Seed %u.\n", seed);
	AArrayTest();
	BArrayTest();
	FooArrayTest();
	IntArrayTest();
	ColourArrayTest();
	ColourStackArrayTest();
	{
		struct CharArray string = ARRAY_ZERO;
		if(!CharArrayCatPrintf(&string, "%.3s bar", "foooo")
			|| !CharArrayCatPrintf(&string, " baz")) return EXIT_FAILURE;
		fprintf(stderr, "CharArrayCatPrintf: %s.\n", CharArrayGet(&string));
		assert(CharArraySize(&string) == 12
			&& !strcmp("foo bar baz", CharArrayGet(&string)));
		CharArray_(&string);
	}
	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}

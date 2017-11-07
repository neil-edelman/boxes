/** Unit test of Foo.c.

 @file		TestFoo
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
#include "Animal.h"

/** Define class {A} */
struct A {
	char value[4];
};
static void A_to_string(const struct A *this, char (*const a)[12]);
/** @implements <Foo>Action */
static void A_filler(struct A *const this) {
	this->value[0] = (char)(rand() / (RAND_MAX + 1.0) * 26.0) + 'A';
	this->value[1] = (char)(rand() / (RAND_MAX + 1.0) * 26.0) + 'a';
	this->value[2] = (char)(rand() / (RAND_MAX + 1.0) * 26.0) + 'a';
	this->value[3] = '\0';
	Orcish(this->value, sizeof this->value);
}
#define POOL_NAME A
#define POOL_TYPE struct A
#define POOL_TO_STRING &A_to_string
#define POOL_TEST &A_filler
#include "../src/Pool.h"
/** Assumes {key} is {[0, 99]}.
 @implements <Foo>ToString */
static void A_to_string(const struct A *this, char (*const a)[12]) {
	/* unusal */
	const struct pool_A_Element *const elem
		= (const struct pool_A_Element *)(const void *)this;
	sprintf(*a, "%ld<%s>%ld", elem->prev, this->value, elem->next);
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
#define POOL_NAME Foo
#define POOL_TYPE struct Foo
#define POOL_TO_STRING &Foo_to_string
#define POOL_TEST &Foo_filler
#define POOL_DEBUG
#include "../src/Pool.h"

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
#define POOL_NAME Int
#define POOL_TYPE int
#define POOL_TO_STRING &Int_to_string
#define POOL_TEST &Int_filler
#include "../src/Pool.h"

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
#include "../src/Pool.h"

static void AnimalsTest(void) {
	struct Animals *a = 0;
	enum { ERR_NO, ERR_ANIMALS } e = ERR_NO;
	do {
		unsigned i;
		if(!(a = Animals())) { e = ERR_ANIMALS; break; }
		for(i = 0; i < 100; i++) {
			if(rand() > RAND_MAX >> 1) {
				if(!AnimalsEmu(a)) { e = ERR_ANIMALS; break; }
			} else {
				if(!AnimalsSloth(a)) { e = ERR_ANIMALS; break; }
			}
		}
		if(e) break;
		AnimalsTransmogrify(a);
		AnimalsClear(a);
		Animals_(&a);
	} while(0); switch(e) {
		case ERR_NO: break;
		case ERR_ANIMALS: break; /* already printed */
	} {
		Animals_(&a);
	}
}

/** Entry point.
 @param argc: The number of arguments, starting with the programme name.
 @param argv: The arguments.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = (unsigned)clock();

	srand(seed), rand(), printf("Seed %u.\n", seed);
	APoolTest();
	FooPoolTest();
	IntPoolTest();
	ColourPoolTest();
	AnimalsTest();
	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}

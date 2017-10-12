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
		= (const struct pool_A_Element *)(void *)this;
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

/***********/

struct BarVt;
struct Bar {
 	const struct BarVt *vt;
 	unsigned key;
};
#define LIST_NAME Bar
#define LIST_TYPE struct Bar
#include "List.h"
static unsigned auto_key = 128;
static void Bar_filler(struct Bar *const bar, const struct BarVt *const vt) {
 	bar->vt  = vt;
 	bar->key = auto_key++;
}
struct BarVt {
 	BarAction transmogrify;
};
static void transmogrify(struct Bar *const bar) {
	bar->vt->transmogrify(bar);
}

struct BarA {
 	struct BarListNode bar;
 	int number;
};
#define POOL_NAME BarA
#define POOL_TYPE struct BarA
#include "../src/Pool.h"
static void A_transmogrify(struct BarA *const a) {
 	printf("Key%u %i!\n", a->bar.data.key, a->number);
}
static struct BarVt A_vt = { (BarAction)&A_transmogrify };
static void BarA_filler(struct BarA *const bar_a) {
	Bar_filler(&bar_a->bar.data, &A_vt);
	bar_a->number = (int)(100.0 * rand() / RAND_MAX);
}

struct BarB {
 	struct BarListNode bar;
 	char letter;
};
#define POOL_NAME BarB
#define POOL_TYPE struct BarB
#include "../src/Pool.h"
static void B_transmogrify(struct BarB *const b) {
 	printf("Key%u %c!\n", b->bar.data.key, b->letter);
}
static struct BarVt B_vt = { (BarAction)&B_transmogrify };
static void BarB_filler(struct BarB *const bar_b) {
	Bar_filler(&bar_b->bar.data, &B_vt);
	bar_b->letter = 'a' + (char)(26.0 * rand() / RAND_MAX);
}

/*static struct BarA *BarA(int number) {
 	struct BarA *a;
 	if(!(a = malloc(sizeof *a))) { perror("BarA"); return 0; }
 	Bar_filler(&a->bar.data, &A_vt);
 	a->number = number;
 	return a;
}

static struct BarB *BarB(char letter) {
 	struct BarB *b;
 	if(letter < 'a' || letter > 'z')
		{ fprintf(stderr, "Letter %c out-of-range.\n", letter); return 0; }
 	if(!(b = malloc(sizeof *b)))
		{ perror("BarB"); return 0; }
 	Bar_filler(&b->bar.data, &B_vt);
 	b->letter = letter;
 	return b;
}*/

static void BarPoolTest(void) {
	struct BarList bar;
	struct BarAPool *a_pool = 0;
	struct BarBPool *b_pool = 0;
	enum { NO, A, B } e = NO;
	do {
		struct BarA *a;
		struct BarB *b;
		BarListClear(&bar);
		if(!(a_pool = BarAPool(&BarListMigrate, &bar))) { e = A; break; }
		if(!(a = BarAPoolNew(a_pool))) { e = A; break; }
		BarA_filler(a);
		BarListPush(&bar, &a->bar);
		if(!(b_pool = BarBPool(&BarListMigrate, &bar))) { e = B; break; }
		if(!(b = BarBPoolNew(b_pool))) { e = B; break; }
		BarB_filler(b);
		BarListPush(&bar, &b->bar);
		BarListForEach(&bar, &transmogrify);
	} while(0); switch(e) {
		case NO: break;
		case A: fprintf(stderr, "A: %s.\n", BarAPoolGetError(a_pool)); break;
		case B: fprintf(stderr, "B: %s.\n", BarBPoolGetError(b_pool)); break;
	} {
		BarBPool_(&b_pool);
		BarAPool_(&a_pool);
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
	BarPoolTest();
	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}

/** Unit test. */

#include <stdlib.h> /* EXIT_ malloc free */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
#include <time.h>	/* clock */
#include <limits.h>	/* INT_MAX */
#include <assert.h> /* assert */
#include "Orcish.h"


#define PARAM(A) A
#define STRINGISE(A) #A
#define COLOUR(X) /* Max 11 letters. */ \
	X(White), X(Silver), X(Gray), X(Black), X(Red), X(Maroon), X(Bisque), \
	X(Wheat), X(Tan), X(Sienna), X(Brown), X(Yellow), X(Khaki), X(Gold), \
	X(Olive), X(Lime), X(Green), X(Aqua), X(Cyan), X(Teal), X(Salmon), \
	X(Orange), X(Powder), X(Sky), X(Steel), X(Royal), X(Blue), X(Navy), \
	X(Fuchsia), X(Pink), X(Purple)
enum Colour { COLOUR(PARAM) };
static const char *const colours[] = { COLOUR(STRINGISE) };
static const size_t colour_size = sizeof colours / sizeof *colours;
static void colour_to_string(const enum Colour *c, char (*const a)[12])
	{ assert(*c < colour_size); sprintf(*a, "%s", colours[*c]); }
static void colour_filler(enum Colour *const c)
	{ *c = rand() / (RAND_MAX / (int)colour_size + 1); }
#define POOL_NAME Colour
#define POOL_TYPE enum Colour
#define POOL_UNFINISHED
#include "../src/Pool.h"
#define POOL_TO_STRING &colour_to_string
#define POOL_TEST &colour_filler
#include "../src/Pool.h"


struct Str4 { char value[4]; };
static void str4_to_string(const struct Str4 *s, char (*const a)[12])
	{ sprintf(*a, "%.11s", s->value); }
static void str4_filler(struct Str4 *const s)
	{ Orcish(s->value, sizeof s->value); }
#define POOL_NAME Str4
#define POOL_TYPE struct Str4
#define POOL_UNFINISHED
#include "../src/Pool.h"
#define POOL_TO_STRING &str4_to_string
#define POOL_TEST &str4_filler
#include "../src/Pool.h"


#if 0

static void int_to_string(const int *i, char (*const a)[12])
{ sprintf(*a, "%d", *i); }
static void int_filler(int *const i)
{ *i = rand() / (RAND_MAX / 1998 + 1) - 999; }
static int int_cmp(const int *const a, const int *const b)
{ return (*a > *b) - (*b > *a); }
#define ARRAY_NAME Int
#define ARRAY_TYPE int
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_TO_STRING &int_to_string
#define ARRAY_TEST &int_filler
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_COMPARE &int_cmp
#include "../src/Array.h"


struct Keyval { int key; char value[12]; };
static void keyval_filler(struct Keyval *const kv)
{ kv->key = rand() / (RAND_MAX / 1098 + 1) - 99;
	Orcish(kv->value, sizeof kv->value); }
static void keyval_key_to_string(const struct Keyval *const kv,
								 char (*const a)[12]) { sprintf(*a, "%d_%.7s", kv->key, kv->value); }
static void keyval_value_to_string(const struct Keyval *const kv,
								   char (*const a)[12]) { sprintf(*a, "%.11s", kv->value); }
static int keyval_key_cmp(const struct Keyval *const a,
						  const struct Keyval *const b)
{ return (a->key > b->key) - (a->key < b->key); }
static int keyval_value_cmp(const struct Keyval *const a,
							const struct Keyval *const b) { return strcmp(a->value, b->value); }
#define ARRAY_NAME Keyval
#define ARRAY_TYPE struct Keyval
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_TO_STRING &keyval_key_to_string
#define ARRAY_TEST &keyval_filler
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_TO_STRING_NAME Value
#define ARRAY_TO_STRING &keyval_value_to_string
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_COMPARE &keyval_key_cmp
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_CONTRAST_NAME Value
#define ARRAY_COMPARE &keyval_value_cmp
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
#define POOL_NAME Foo
#define POOL_TYPE struct Foo
#define POOL_TO_STRING &Foo_to_string
#define POOL_TEST &Foo_filler
#include "../src/Pool.h"




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
#include "../src/Pool.h"

#endif


/** Entry point.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = /*12395*/ /*2532*//*11632*/(unsigned)clock();

	srand(seed), rand(), printf("Seed %u.\n", seed);
	ColourPoolTest();
	Str4PoolTest();
	/*FooPoolTest();
	IntPoolTest();
	ColourPoolTest();*/
	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}

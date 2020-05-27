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


static void int_to_string(const int *i, char (*const a)[12])
	{ sprintf(*a, "%d", *i); }
static void int_filler(int *const i)
	{ *i = rand() / (RAND_MAX / 1998 + 1) - 999; }
/*static int int_cmp(const int *const a, const int *const b)
	{ return (*a > *b) - (*b > *a); }*/
#define POOL_NAME Int
#define POOL_TYPE int
#define POOL_UNFINISHED
#include "../src/Pool.h"
#define POOL_TO_STRING &int_to_string
#define POOL_TEST &int_filler
#include "../src/Pool.h"


struct Keyval { int key; char value[12]; };
static void keyval_filler(struct Keyval *const kv)
	{ kv->key = rand() / (RAND_MAX / 1098 + 1) - 99;
	Orcish(kv->value, sizeof kv->value); }
static void keyval_key_to_string(const struct Keyval *const kv,
	char (*const a)[12]) { sprintf(*a, "%d_%.7s", kv->key, kv->value); }
static void keyval_value_to_string(const struct Keyval *const kv,
	char (*const a)[12]) { sprintf(*a, "%.11s", kv->value); }
/*static int keyval_key_cmp(const struct Keyval *const a,
	const struct Keyval *const b)
	{ return (a->key > b->key) - (a->key < b->key); }
static int keyval_value_cmp(const struct Keyval *const a,
	const struct Keyval *const b) { return strcmp(a->value, b->value); }*/
#define POOL_NAME Keyval
#define POOL_TYPE struct Keyval
#define POOL_UNFINISHED
#include "../src/Pool.h"
#define POOL_TO_STRING &keyval_key_to_string
#define POOL_TEST &keyval_filler
#define POOL_UNFINISHED
#include "../src/Pool.h"
#define POOL_TO_STRING_NAME Value
#define POOL_TO_STRING &keyval_value_to_string
#include "../src/Pool.h"


/** Entry point.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = /*12395*/ /*2532*//*11632*/(unsigned)clock();

	srand(seed), rand(), printf("Seed %u.\n", seed);
	ColourPoolTest();
	Str4PoolTest();
	IntPoolTest();
	KeyvalPoolTest();
	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}

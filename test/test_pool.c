/** Unit test. */

#include <stdlib.h> /* EXIT_ malloc free rand */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
#include <time.h>	/* clock */
#include <limits.h>	/* INT_MAX */
#include <assert.h> /* assert */
#include "orcish.h"


#define PARAM(A) A
#define STRINGIZE(A) #A
#define COLOUR(X) /* Max 11 letters. */ \
	X(White), X(Silver), X(Gray), X(Black), X(Red), X(Maroon), X(Bisque), \
	X(Wheat), X(Tan), X(Sienna), X(Brown), X(Yellow), X(Khaki), X(Gold), \
	X(Olive), X(Lime), X(Green), X(Aqua), X(Cyan), X(Teal), X(Salmon), \
	X(Orange), X(Powder), X(Sky), X(Steel), X(Royal), X(Blue), X(Navy), \
	X(Fuchsia), X(Pink), X(Purple)
enum colour { COLOUR(PARAM) };
static const char *const colours[] = { COLOUR(STRINGIZE) };
static const size_t colour_size = sizeof colours / sizeof *colours;
static void colour_to_string(const enum colour *c, char (*const a)[12])
	{ assert(*c < colour_size); sprintf(*a, "%s", colours[*c]); }
static void colour_filler(enum colour *const c)
	{ *c = (unsigned)rand() / (RAND_MAX / colour_size + 1); }
#define POOL_NAME colour
#define POOL_TYPE enum colour
#define POOL_TEST &colour_filler
#define POOL_EXPECT_TRAIT
#include "../src/pool.h"
#define POOL_TO_STRING &colour_to_string
#include "../src/pool.h"


struct str4 { char value[4]; };
static void str4_to_string(const struct str4 *s, char (*const a)[12])
	{ sprintf(*a, "%.11s", s->value); }
static void str4_filler(struct str4 *const s)
	{ orcish(s->value, sizeof s->value); }
#define POOL_NAME str4
#define POOL_TYPE struct str4
#define POOL_TEST &str4_filler
#define POOL_EXPECT_TRAIT
#include "../src/pool.h"
#define POOL_TO_STRING &str4_to_string
#include "../src/pool.h"


static void int_to_string(const int *i, char (*const a)[12])
	{ sprintf(*a, "%d", *i); }
static void int_filler(int *const i)
	{ *i = rand() / (RAND_MAX / 1998 + 1) - 999; }
/*static int int_cmp(const int *const a, const int *const b)
	{ return (*a > *b) - (*b > *a); }*/
#define POOL_NAME int
#define POOL_TYPE int
#define POOL_TEST &int_filler
#define POOL_EXPECT_TRAIT
#include "../src/pool.h"
#define POOL_TO_STRING &int_to_string
#include "../src/pool.h"


struct keyval { int key; char value[12]; };
static void keyval_filler(struct keyval *const kv)
	{ kv->key = rand() / (RAND_MAX / 1098 + 1) - 99;
	orcish(kv->value, sizeof kv->value); }
static void keyval_key_to_string(const struct keyval *const kv,
	char (*const a)[12]) { sprintf(*a, "%d_%.7s", kv->key, kv->value); }
static void keyval_value_to_string(const struct keyval *const kv,
	char (*const a)[12]) { sprintf(*a, "%.11s", kv->value); }
/*static int keyval_key_cmp(const struct Keyval *const a,
	const struct Keyval *const b)
	{ return (a->key > b->key) - (a->key < b->key); }
static int keyval_value_cmp(const struct Keyval *const a,
	const struct Keyval *const b) { return strcmp(a->value, b->value); }*/
#define POOL_NAME keyval
#define POOL_TYPE struct keyval
#define POOL_TEST &keyval_filler
#define POOL_EXPECT_TRAIT
#include "../src/pool.h"
#define POOL_TO_STRING &keyval_key_to_string
#define POOL_EXPECT_TRAIT
#include "../src/pool.h"
#define POOL_TO_STRING_NAME value
#define POOL_TO_STRING &keyval_value_to_string
#include "../src/pool.h"


/** For paper. */
static void special(void) {
	struct keyval_pool kvp = keyval_pool();
	struct keyval *kv[30];
	int is_kv[sizeof kv / sizeof *kv];
	size_t i;
	for(i = 0; i < sizeof kv / sizeof *kv; i++) kv[i] = 0, is_kv[i] = 0;
	for(i = 0; i < sizeof kv / sizeof *kv; i++) {
		if(!(kv[i] = keyval_pool_new(&kvp))) goto finally;
		keyval_filler(kv[i]);
		is_kv[i] = 1;
	}
	for(i = 0; i < sizeof kv / sizeof *kv; i++) {
		size_t r = (size_t)rand() / (RAND_MAX / (sizeof kv / sizeof *kv) + 1);
		if(!is_kv[r]) continue;
		if(!keyval_pool_remove(&kvp, kv[r])) goto finally;
		is_kv[r] = 0;
	}
finally:
	pool_keyval_graph(&kvp, "graph/kvpool.gv");
	keyval_pool_(&kvp);
}

/** Entry point.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = (unsigned)clock();

	srand(seed), rand(), printf("Seed %u.\n", seed);
	colour_pool_test();
	str4_pool_test();
	int_pool_test();
	keyval_pool_test();
	special();
	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}

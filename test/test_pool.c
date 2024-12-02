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
#define POOL_TO_STRING
#define POOL_TEST
#include "../src/pool.h"


#if 0
struct str4 { char value[4]; };
static void str4_to_string(const struct str4 *s, char (*const a)[12])
	{ sprintf(*a, "%.11s", s->value); }
static void str4_filler(struct str4 *const s)
	{ orcish(s->value, sizeof s->value); }
#define POOL_NAME str4
#define POOL_TYPE struct str4
#define POOL_TEST
#define POOL_TO_STRING
#include "../src/pool.h"


static void int_to_string(const int *i, char (*const a)[12])
	{ sprintf(*a, "%d", *i); }
static void int_filler(int *const i)
	{ *i = rand() / (RAND_MAX / 1998 + 1) - 999; }
/*static int int_cmp(const int *const a, const int *const b)
	{ return (*a > *b) - (*b > *a); }*/
#define POOL_NAME int
#define POOL_TYPE int
#define POOL_TEST
#define POOL_TO_STRING
#include "../src/pool.h"


struct keyval { int key; char value[12]; };
static void keyval_filler(struct keyval *const kv)
	{ kv->key = rand() / (RAND_MAX / 1098 + 1) - 99;
	orcish(kv->value, sizeof kv->value); }
static void keyval_to_string(const struct keyval *const kv,
	char (*const a)[12]) { sprintf(*a, "%d_%.7s", kv->key, kv->value); }
#define POOL_NAME keyval
#define POOL_TYPE struct keyval
#define POOL_TEST
#define POOL_TO_STRING
#include "../src/pool.h"


/* Separate head/body. */
#define POOL_NAME public
#define POOL_TYPE enum colour
#define POOL_HEAD
#include "../src/pool.h"
static void public_to_string(const enum colour *c, char (*const a)[12])
	{ colour_to_string(c, a); }
static void public_filler(enum colour *const c) { colour_filler(c); }
#define POOL_NAME public
#define POOL_TYPE enum colour
#define POOL_TO_STRING
#define POOL_TEST
#define POOL_BODY
#include "../src/pool.h"


/** For paper. */
static void special(void) {
	struct colour_pool pool = colour_pool();
	enum colour *colour[35];
	int is[sizeof colour / sizeof *colour];
	size_t i;
	for(i = 0; i < sizeof colour / sizeof *colour; i++) colour[i] = 0, is[i] = 0;
	for(i = 0; i < sizeof colour / sizeof *colour; i++) {
		if(!(colour[i] = colour_pool_new(&pool))) goto finally;
		colour_filler(colour[i]);
		is[i] = 1;
	}
	for(i = 0; i < sizeof colour / sizeof *colour; i++) {
		size_t r = (size_t)rand()
			/ (RAND_MAX / (sizeof colour / sizeof *colour) + 1);
		if(!is[r]) continue;
		if(!colour_pool_remove(&pool, colour[r])) goto finally;
		is[r] = 0;
	}
finally:
	pool_colour_graph(&pool, "graph/paper.gv");
	colour_pool_(&pool);
}
#endif


/** Entry point.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = (unsigned)clock();

	srand(seed), rand(), printf("Seed %u.\n", seed);
	colour_pool_test();
	/*str4_pool_test();
	int_pool_test();
	keyval_pool_test();
	public_pool_test();
	special();*/
	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}

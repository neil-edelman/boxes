#include <stdlib.h> /* EXIT_ malloc free */
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
#include "../src/deque_pool.h"
#define POOL_TO_STRING &colour_to_string
#include "../src/deque_pool.h"

#define POOL_NAME oldcolour
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
#include "../src/deque_pool.h"
#define POOL_TO_STRING &str4_to_string
#include "../src/deque_pool.h"


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
#include "../src/deque_pool.h"
#define POOL_TO_STRING &int_to_string
#include "../src/deque_pool.h"


struct keyval { int key; char value[12]; };
static void keyval_filler(struct keyval *const kv)
	{ kv->key = rand() / (RAND_MAX / 1098 + 1) - 99;
	orcish(kv->value, sizeof kv->value); }
static void keyval_key_to_string(const struct keyval *const kv,
	char (*const a)[12]) { sprintf(*a, "%d_%.7s", kv->key, kv->value); }
static void keyval_value_to_string(const struct keyval *const kv,
	char (*const a)[12]) { sprintf(*a, "%.11s", kv->value); }
#define POOL_NAME keyval
#define POOL_TYPE struct keyval
#define POOL_TEST &keyval_filler
#define POOL_EXPECT_TRAIT
#include "../src/deque_pool.h"
#define POOL_TO_STRING &keyval_key_to_string
#define POOL_EXPECT_TRAIT
#include "../src/deque_pool.h"
#define POOL_TO_STRING_NAME value
#define POOL_TO_STRING &keyval_value_to_string
#include "../src/deque_pool.h"

#define POOL_NAME oldkeyval
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

#define ARRAY_NAME keyvalref
#define ARRAY_TYPE struct keyval *
#include "array.h"

/** Returns a time diffecence in microseconds from `then`. */
static double diff_us(clock_t then)
	{ return 1000000.0 / CLOCKS_PER_SEC * (clock() - then); }

static void timing(const size_t length,
	FILE *const fp_time, FILE *const fp_space) {
	struct keyval_pool a = POOL_IDLE;
	struct oldkeyval_pool b = POOL_IDLE;
	struct keyvalref_array c = ARRAY_IDLE;
	size_t i;
	/*const size_t length = 120000;*/ /* Controls how many iterations. */
	const double growth = 50.0;
	const unsigned seed = (unsigned)clock();
	clock_t t;

	keyvalref_array_buffer(&c, length);
	srand(seed)/*, printf("Seed %u.\n", seed)*/;

	t = clock();
	for(i = 0; i < length; i++) {
		double r = rand() / (RAND_MAX + 1.0);
		if(r > c.size / growth) {
			struct keyval *data = keyval_pool_new(&a),
				**ref = keyvalref_array_new(&c);
			assert(data && ref);
			keyval_filler(data);
			*ref = data;
		} else {
			size_t idx = (unsigned)rand() / (RAND_MAX / c.size + 1);
			struct keyval **kv_ad = c.data + idx;
			keyval_pool_remove(&a, *kv_ad);
			keyvalref_array_remove(&c, kv_ad);
		}
	}
	fprintf(fp_time, "%lu\t%f", length, diff_us(t));
	{
		size_t size = sizeof a + a.slots.capacity * sizeof a.slots.data
			+ a.free0.a.capacity * sizeof a.free0.a.data;
		for(i = 0; i < a.slots.size; i++) size += sizeof a.slots.data[i]
			+ a.slots.data[i]->size * sizeof(pool_keyval_type);
		fprintf(fp_space, "%lu\t%lu", length, (unsigned long)size);
	}
	keyval_pool_(&a);
	keyvalref_array_clear(&c);
	t = clock();
	for(i = 0; i < length; i++) {
		double r = rand() / (RAND_MAX + 1.0);
		if(r > c.size / growth) {
			struct keyval *data = oldkeyval_pool_new(&b),
				**ref = keyvalref_array_new(&c);
			assert(data && ref);
			keyval_filler(data);
			*ref = data;
		} else {
			size_t idx = (unsigned)rand() / (RAND_MAX / c.size + 1);
			struct keyval **kv_ad = c.data + idx;
			oldkeyval_pool_remove(&b, *kv_ad);
			keyvalref_array_remove(&c, kv_ad);
		}
	}
	fprintf(fp_time, "\t%f\n", diff_us(t));
	{
		size_t size = sizeof b;
		struct pool_oldkeyval_block *p;
		for(p = b.largest; p; p = p->smaller)
			size += sizeof *p + p->capacity * sizeof(struct pool_oldkeyval_node);
		fprintf(fp_space, "\t%lu\n", (unsigned long)size);
	}
	oldkeyval_pool_(&b);
	keyvalref_array_(&c);
}

int main(void) {
	unsigned seed = (unsigned)clock();
	size_t length;
	FILE *fp_time = 0, *fp_space = 0;
	const char *const fn_time = "pool_vs_pool_time.data",
		*const fn_space = "pool_vs_pool_space.data";
	int success = EXIT_FAILURE;

	srand(seed), rand(), printf("Seed %u.\n", seed);
	colour_pool_test();
	oldcolour_pool_test();
	str4_pool_test();
	int_pool_test();
	keyval_pool_test();
	oldkeyval_pool_test();
	printf("Test success.\n\n");

	if(!(fp_time = fopen(fn_time, "w")) || !(fp_space = fopen(fn_space, "w")))
		goto catch;
	fprintf(fp_time, "# size\tnew\told\n");
	fprintf(fp_space, "# size\tnew\told\n");
	for(length = 5; length < 100000000; length <<= 1)
		timing(length, fp_time, fp_space);
	success = EXIT_SUCCESS;
	goto finally;
catch:
	perror("file");
finally:
	if(fp_time) fclose(fp_time);
	if(fp_space) fclose(fp_space);
	return success;
}

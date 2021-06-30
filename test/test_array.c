/** Unit test of array.h. @author Neil @std C89/90 */

#include <stdlib.h> /* EXIT_ */
#include <stdio.h>  /* s?printf */
#include <string.h>	/* strcmp */
#include <time.h>	/* clock */
#include <assert.h> /* assert */
#include "orcish.h"


#define PARAM(A) A
#define STRINGISE(A) #A
#define COLOUR(X) /* Max 11 letters. */ \
	X(White), X(Silver), X(Gray), X(Black), X(Red), X(Maroon), X(Bisque), \
	X(Wheat), X(Tan), X(Sienna), X(Brown), X(Yellow), X(Khaki), X(Gold), \
	X(Olive), X(Lime), X(Green), X(Aqua), X(Cyan), X(Teal), X(Salmon), \
	X(Orange), X(Powder), X(Sky), X(Steel), X(Royal), X(Blue), X(Navy), \
	X(Fuchsia), X(Pink), X(Purple)
enum colour { COLOUR(PARAM) };
static const char *const colours[] = { COLOUR(STRINGISE) };
static const size_t colour_size = sizeof colours / sizeof *colours;
static void colour_to_string(const enum colour *c, char (*const a)[12])
	{ assert(*c < colour_size); sprintf(*a, "%s", colours[*c]); }
static void colour_filler(enum colour *const c)
	{ *c = (unsigned)rand() / (RAND_MAX / colour_size + 1); }
#define ARRAY_NAME colour
#define ARRAY_TYPE enum colour
#define ARRAY_FUNCTION
#define ARRAY_TEST &colour_filler
#define ARRAY_EXPECT_TRAIT
#include "../src/array.h"
#define ARRAY_TO_STRING &colour_to_string
#include "../src/array.h"

struct str4 { char value[4]; };
static void str4_to_string(const struct str4 *s, char (*const a)[12])
	{ sprintf(*a, "%.11s", s->value); }
static void str4_filler(struct str4 *const s)
	{ orcish(s->value, sizeof s->value); }
#define ARRAY_NAME str4
#define ARRAY_TYPE struct str4
#define ARRAY_TEST &str4_filler
#define ARRAY_EXPECT_TRAIT
#include "../src/array.h"
#define ARRAY_TO_STRING &str4_to_string
#include "../src/array.h"


static void int_to_string(const int *i, char (*const a)[12])
	{ sprintf(*a, "%d", *i); }
static void int_filler(int *const i)
	{ *i = rand() / (RAND_MAX / 1998 + 1) - 999; }
static int int_cmp(const int *const a, const int *const b)
	{ return (*a > *b) - (*b > *a); }
#define ARRAY_NAME int
#define ARRAY_TYPE int
#define ARRAY_TEST &int_filler
#define ARRAY_EXPECT_TRAIT
#include "../src/array.h"
#define ARRAY_COMPARE &int_cmp
#define ARRAY_EXPECT_TRAIT
#include "../src/array.h"
#define ARRAY_TO_STRING &int_to_string
#include "../src/array.h"


struct keyval { int key; char value[12]; };
static void keyval_filler(struct keyval *const kv)
	{ kv->key = rand() / (RAND_MAX / 1098 + 1) - 99;
	orcish(kv->value, sizeof kv->value); }
static void keyval_key_to_string(const struct keyval *const kv,
	char (*const a)[12]) { sprintf(*a, "%d_%.7s", kv->key, kv->value); }
static void keyval_value_to_string(const struct keyval *const kv,
	char (*const a)[12]) { sprintf(*a, "%.11s", kv->value); }
static int keyval_key_cmp(const struct keyval *const a,
	const struct keyval *const b)
	{ return (a->key > b->key) - (a->key < b->key); }
static int keyval_value_cmp(const struct keyval *const a,
	const struct keyval *const b) { return strcmp(a->value, b->value); }
#define ARRAY_NAME keyval
#define ARRAY_TYPE struct keyval
#define ARRAY_TEST &keyval_filler
#define ARRAY_EXPECT_TRAIT
#include "../src/array.h"
#define ARRAY_TO_STRING &keyval_key_to_string
#define ARRAY_EXPECT_TRAIT
#include "../src/array.h"
#define ARRAY_TO_STRING_NAME value
#define ARRAY_TO_STRING &keyval_value_to_string
#define ARRAY_EXPECT_TRAIT
#include "../src/array.h"
#define ARRAY_COMPARE &keyval_key_cmp
#define ARRAY_EXPECT_TRAIT
#include "../src/array.h"
#define ARRAY_COMPARE_NAME value
#define ARRAY_COMPARE &keyval_value_cmp
#include "../src/array.h"


/** Tests. @return `EXIT_SUCCESS`. */
int main(void) {
	unsigned seed = (unsigned)clock();

	srand(seed), rand(), printf("Seed %u.\n", seed);
    errno = 0;
	colour_array_test();
	str4_array_test();
	int_array_test();
	//int_array_comparable_test();
	keyval_array_test();
	//keyval_array_comparable_test();
	//keyval_array_value_comparable_test();
	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}

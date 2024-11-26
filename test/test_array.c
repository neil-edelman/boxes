/** Test of array.h.
 @std C89/90 */

#include <stdlib.h> /* EXIT_ */
#include <stdio.h>  /* printf */
#include <string.h> /* strcmp */
#include <time.h>   /* clock */
#include <assert.h> /* assert */
#include "orcish.h"


/* Not used because it's not set up for testing. The minimal array. */
#define ARRAY_NAME number
#define ARRAY_TYPE int
#include "../src/array.h"


/* Struct array. */
struct str4 { char value[4]; };
static void str4_to_string(const struct str4 *const s, char (*const a)[12])
	{ sprintf(*a, "%.11s", s->value); }
static void str4_filler(struct str4 *const s)
	{ orcish(s->value, sizeof s->value); }
#define ARRAY_NAME str4
#define ARRAY_TYPE struct str4
#define ARRAY_TEST
#define ARRAY_TO_STRING
#include "../src/array.h"

#if 0

#define HAVE_ITERATE_H /* More tests. */

/* Enum array. */
#define PARAM(A) A
#define STRINGISE(A) #A
#define COLOUR(X) \
	X(White), X(Silver), X(Gray), X(Black), X(Red), X(Maroon), X(Bisque), \
	X(Wheat), X(Tan), X(Sienna), X(Brown), X(Yellow), X(Khaki), X(Gold), \
	X(Olive), X(Lime), X(Green), X(Aqua), X(Cyan), X(Teal), X(Salmon), \
	X(Orange), X(Powder), X(Sky), X(Steel), X(Royal), X(Blue), X(Navy), \
	X(Fuchsia), X(Pink), X(Purple)
enum colour { COLOUR(PARAM) };
static const char *const colours[] = { COLOUR(STRINGISE) };
static const size_t colour_size = sizeof colours / sizeof *colours;
static void colour_filler(enum colour *const c)
	{ *c = (unsigned)rand() / (RAND_MAX / colour_size + 1); }
static void colour_to_string(const enum colour *const c, char (*const a)[12])
	{ assert(*c < colour_size); sprintf(*a, "%.11s", colours[*c]); }
static int colour_is_equal(const enum colour *const a,
	const enum colour *const b) { return *a == *b; }
#define ARRAY_NAME colour
#define ARRAY_TYPE enum colour
#define ARRAY_TEST
#define ARRAY_IS_EQUAL
#define ARRAY_TO_STRING
#include "../src/array.h"


/* Int array with compare. */
static void int_to_string(const int *i, char (*const a)[12])
	{ sprintf(*a, "%d", *i); }
static void int_filler(int *const i)
	{ *i = rand() / (RAND_MAX / 1998 + 1) - 999; }
static int int_compare(const int *const a, const int *const b)
	{ return (*a > *b) - (*b > *a); }
#define ARRAY_NAME int
#define ARRAY_TYPE int
#define ARRAY_TEST
#define ARRAY_COMPARE
#define ARRAY_TO_STRING
#include "../src/array.h"


/* Array with two traits. */
struct keyval { int key; char value[12]; };
static void keyval_filler(struct keyval *const kv)
	{ kv->key = rand() / (RAND_MAX / 1098 + 1) - 99;
	orcish(kv->value, sizeof kv->value); }
static void keyval_to_string(const struct keyval *const kv,
	char (*const a)[12]) { sprintf(*a, "%d_%.7s", kv->key, kv->value); }
static void keyval_value_to_string(const struct keyval *const kv,
	char (*const a)[12]) { sprintf(*a, "%.11s", kv->value); }
static int keyval_compare(const struct keyval *const a,
	const struct keyval *const b)
	{ return (a->key > b->key) - (a->key < b->key); }
static int keyval_value_compare(const struct keyval *const a,
	const struct keyval *const b) { return strcmp(a->value, b->value); }
#define ARRAY_NAME keyval
#define ARRAY_TYPE struct keyval
#define ARRAY_TEST
#define ARRAY_COMPARE
#define ARRAY_TO_STRING
#define ARRAY_EXPECT_TRAIT
#include "../src/array.h"
#define ARRAY_TRAIT value
#define ARRAY_TO_STRING
#define ARRAY_COMPARE
#include "../src/array.h"


static int targets[] = { 4, 2, 8, 2, 6, 5, 3, 6, 1, 2, 9, 3 };
static void pointer_to_string(const int *const*const i, char (*const a)[12])
	{ sprintf(*a, "%d", **i); }
static void pointer_filler(int **const i)
	{ *i = targets
	+ rand() / (RAND_MAX / (int)(sizeof targets / sizeof *targets) + 1); }
static int pointer_compare(const int *const*const a, const int *const*const b)
	{ return int_compare(*a, *b); }
#define ARRAY_NAME pointer
#define ARRAY_TYPE int *
#define ARRAY_TEST
#define ARRAY_COMPARE
#define ARRAY_TO_STRING
#include "../src/array.h"


/* This shows how one might use a header. Make sure the required functions are
 declared before defining the array. */
static void static_to_string(const int *i, char (*const a)[12])
	{ int_to_string(i, a); }
static void static_filler(int *const i) { int_filler(i); }
static int static_compare(const int *const a, const int *const b)
	{ return int_compare(a, b); }
#define INTEGER_ARRAY_DEFINE /* Invert meaning. */
#include "integer_array.h"
/* Wrapping all needed functions. (Compound laterals are C99.) */
struct integer_array integer_array(void)
	{ struct integer_array _; _._ = static_array(); return _; }
void integer_array_(struct integer_array *const _) { static_array_(&_->_); }
void integer_array_test(void) { static_array_test(); }

#endif

/** Tests; assert crashes on failed test. @return `EXIT_SUCCESS`. */
int main(void) {
	unsigned seed = (unsigned)clock() /*2394*/;

	srand(seed), rand(), printf("Seed %u.\n", seed);
	errno = 0;
	str4_array_test();
#if 0
	colour_array_test();
	colour_array_compare_test();
	int_array_test();
	int_array_compare_test();
	keyval_array_test();
	keyval_array_compare_test();
	keyval_array_value_compare_test();
	pointer_array_test();
	/*pointer_array_compare_test();
	 <- probably the test is wrong, assumes contiguous. I don't know what it's
	 doing, wrote 10 years ago. */
	(void)pointer_array_compare_test;
	integer_array_test(); /* Visible to #include "integer_array.h". */
	static_array_compare_test(); /* Still static version—we have yet to define
	 a wrapper function. */
#endif
	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}

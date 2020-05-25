/** Unit test of Array.c. @author Neil @std C89/90 */

#include <stdlib.h> /* EXIT_ malloc free */
#include <stdio.h>  /* fprintf */
#include <string.h>	/* strcmp */
#include <time.h>	/* clock */
#include <limits.h>	/* INT_MAX */
#include <assert.h> /* assert */
#include "Orcish.h"


/* X-macro. */
#define PARAM(A) A
#define STRINGISE(A) #A
 /* Max 11 letters. */
#define COLOUR(X) \
	X(White), X(Silver), X(Gray), X(Black), X(Red), X(Maroon), X(Bisque), \
	X(Wheat), X(Tan), X(Sienna), X(Brown), X(Yellow), X(Khaki), X(Gold), \
	X(Olive), X(Lime), X(Green), X(Aqua), X(Cyan), X(Teal), X(Salmon), \
	X(Orange), X(Powder), X(Sky), X(Steel), X(Royal), X(Blue), X(Navy), \
	X(Fuchsia), X(Pink), X(Purple)
enum Colour { COLOUR(PARAM) };
static const char *const colours[] = { COLOUR(STRINGISE) };
static const size_t colour_size = sizeof colours / sizeof *colours;
static void Colour_to_string(const enum Colour *c, char (*const a)[12])
	{ assert(*c < colour_size); sprintf(*a, "%s", colours[*c]); }
static void Colour_filler(enum Colour *const c)
	{ *c = (enum Colour)(float)(rand() / (RAND_MAX + 1.0) * colour_size); }
#define ARRAY_NAME Colour
#define ARRAY_TYPE enum Colour
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_TO_STRING &Colour_to_string
#define ARRAY_TEST &Colour_filler
#include "../src/Array.h"



struct Str4 { char value[4]; };
static void Str4_to_string(const struct Str4 *s, char (*const a)[12])
	{ sprintf(*a, "%.11s", s->value); }
static void Str4_filler(struct Str4 *const s)
	{ Orcish(s->value, sizeof s->value); }
#define ARRAY_NAME Str4
#define ARRAY_TYPE struct Str4
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_TO_STRING &Str4_to_string
#define ARRAY_TEST &Str4_filler
#include "../src/Array.h"


/* fixme */
#if RAND_MAX > 9999999999
#error RAND_MAX is too big.
#endif
static void Int_to_string(const int *i, char (*const a)[12])
	{ sprintf(*a, "%d", *i); }
static void Int_filler(int *const i)
	{ *i = rand() / (RAND_MAX / 1998 + 1) - 999; }
static int Int_cmp(const int *const a, const int *const b)
	{ return (*a > *b) - (*b > *a); }
#define ARRAY_NAME Int
#define ARRAY_TYPE int
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_TO_STRING &Int_to_string
#define ARRAY_TEST &Int_filler
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_COMPARE &Int_cmp
#include "../src/Array.h"


struct Keyval { int key; char value[12]; };
static void Keyval_filler(struct Keyval *const kv)
	{ kv->key = rand() / (RAND_MAX / 1098 + 1) - 99;
	Orcish(kv->value, sizeof kv->value); }
static void Keyval_key_to_string(const struct Keyval *const kv,
	char (*const a)[12]) { sprintf(*a, "%d_%.7s", kv->key, kv->value); }
static void Keyval_value_to_string(const struct Keyval *const kv,
	char (*const a)[12]) { sprintf(*a, "%.11s", kv->value); }
static int Keyval_key_cmp(const struct Keyval *const a,
	const struct Keyval *const b)
	{ return (a->key > b->key) - (a->key < b->key); }
static int Keyval_value_cmp(const struct Keyval *const a,
	const struct Keyval *const b) { return strcmp(a->value, b->value); }
#define ARRAY_NAME Keyval
#define ARRAY_TYPE struct Keyval
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_TO_STRING &Keyval_key_to_string
#define ARRAY_TEST &Keyval_filler
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_TO_STRING_NAME Value
#define ARRAY_TO_STRING &Keyval_value_to_string
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_COMPARE &Keyval_key_cmp
#define ARRAY_UNFINISHED
#include "../src/Array.h"
#define ARRAY_CONTRAST_NAME Value
#define ARRAY_COMPARE &Keyval_value_cmp
#include "../src/Array.h"


/** Entry point.
 @return Either EXIT_SUCCESS or EXIT_FAILURE. */
int main(void) {
	unsigned seed = (unsigned)clock();

	srand(seed), rand(), printf("Seed %u.\n", seed);
	ColourArrayTest();
	Str4ArrayTest();
	IntArrayTest();
	IntArrayContrastTest();
	KeyvalArrayTest();
	KeyvalArrayContrastTest();
	KeyvalArrayValueContrastTest();
	printf("Test success.\n\n");

	return EXIT_SUCCESS;
}

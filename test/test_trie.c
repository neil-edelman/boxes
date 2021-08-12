/* Test Trie. */

#include <stdlib.h> /* EXIT malloc free rand */
#include <stdio.h>  /* *printf */
#include <assert.h> /* assert */
#include <errno.h>  /* errno */
#include <time.h>   /* clock time */
#include "orcish.h"

#define TRIE_NAME str
#define TRIE_TO_STRING
#include "../src/trie.h"

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
static void colour_filler(enum colour *const c)
	{ *c = (unsigned)rand() / (RAND_MAX / colour_size + 1); }
static const char *colour_key(const enum colour *const c)
	{ return colours[*c]; }
#define TRIE_NAME colour
#define TRIE_TYPE enum colour
#define TRIE_KEY &colour_key
#define TRIE_TEST &colour_filler
#define TRIE_TO_STRING
#include "../src/trie.h"

struct str4 { char value[4]; };
static void str4_filler(struct str4 *const s)
	{ orcish(s->value, sizeof s->value); }
static const char *str4_key(const struct str4 *const s) { return s->value; }
#define TRIE_NAME str4
#define TRIE_TYPE struct str4
#define TRIE_KEY &str4_key
#define TRIE_TEST &str4_filler
#define TRIE_TO_STRING
#include "../src/trie.h"

/* This is organized by value; the key doesn't do anything. */
struct keyval { int key; char value[12]; };
static void keyval_filler(struct keyval *const kv)
	{ kv->key = rand() / (RAND_MAX / 1098 + 1) - 99;
	orcish(kv->value, sizeof kv->value); }
static const char *keyval_key(const struct keyval *const kv)
	{ return kv->value; }
#define TRIE_NAME keyval
#define TRIE_TYPE struct keyval
#define TRIE_KEY &keyval_key
#define TRIE_TEST &keyval_filler
#define TRIE_TO_STRING
#include "../src/trie.h"

static void str_trie_test(void) {
	struct str_trie strs = TRIE_IDLE;
	struct str_trie_iterator it;
	const char *str;
	const char *test[] = { "b", "f", "a", "" };
	const size_t test_size = sizeof test / sizeof *test;
	size_t i;
	str_trie_add(&strs, "bar");
	str_trie_add(&strs, "baz");
	str_trie_add(&strs, "foo");
	for(i = 0; i < test_size; i++) {
		str = test[i];
		str_trie_prefix(&strs, str, &it);
		printf("%s: (%u, %u) size %lu\n",
			str, it.leaf, it.leaf_end, (unsigned long)str_trie_size(&it));
		while(str = str_trie_next(&it))
			printf("got: %s.\n", str);
	}
	str_trie_(&strs);
}

int main(void) {
	unsigned seed = (unsigned)clock();
	srand(seed), rand(), printf("Seed %u.\n", seed);
	str_trie_test(); /* Special. */
	colour_trie_test();
	str4_trie_test();
	keyval_trie_test();
	return EXIT_SUCCESS;
}

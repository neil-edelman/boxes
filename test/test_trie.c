/* Case-insensitive insert compare ignore on
 bit == 2
 && 0xC0 & key[i] == 0x40 ('@')
 && sample branch's key is not <\0\7F@`^~_>? */

#include <stdlib.h> /* EXIT malloc free rand */
#include <stdio.h>  /* *printf */
#include <assert.h> /* assert */
#include <errno.h>  /* errno */
#include <time.h>   /* clock time */
#include "orcish.h"


/** For testing -- have a pool of random names. */
struct str32 { char str[32]; };
#define POOL_NAME str32
#define POOL_TYPE struct str32
#include "pool.h"
static struct str32_pool global_pool;


/* A set of strings stored somewhere else; one must keep the storage for the
 duration of the trie*. Good if one needs a prefix-tree with string literals.
 *Unless one only wants to use <fn:<T>trie_match>, which only looks at the
 index and not the keys themselves; the key strings are not accessed, then. */
/** Generate a random name from `global_pool` and assign it to `key`. */
static void str32_filler(const char **const key) {
	struct str32 *backing = str32_pool_new(&global_pool);
	/* Unlikely to fail, but for tests, we don't have recourse to back out. */
	assert(backing && key);
	orcish(backing->str, sizeof backing->str);
	*key = backing->str;
}
#define TRIE_NAME str
#define TRIE_TO_STRING /* Uses the keys as strings. For test. */
#define TRIE_TEST &str32_filler
#include "../src/trie.h"


/* This is a custom key; uses `TRIE_KEY` and `TRIE_KEY_TO_STRING` to forward
 the keys to `colours`. Internally, a trie is a collection of fixed trees that
 have `union` leaves with a pointer-to-tree; there therefore is no optimization
 to shrinking the size of the data past a pointer. */
#define COLOUR \
	X(White), X(Silver), X(Gray), X(Black), X(Red), X(Maroon), X(Bisque), \
	X(Wheat), X(Tan), X(Sienna), X(Brown), X(Yellow), X(Khaki), X(Gold), \
	X(Olive), X(Lime), X(Green), X(Aqua), X(Cyan), X(Teal), X(Salmon), \
	X(Orange), X(Powder), X(Sky), X(Steel), X(Royal), X(Blue), X(Navy), \
	X(Fuchsia), X(Pink), X(Purple)
#define X(n) n
enum colour { COLOUR };
#undef X
#define X(n) #n
static const char *const colours[] = { COLOUR };
#undef X
#undef COLOUR
static const size_t colour_size = sizeof colours / sizeof *colours;
static const char *colour_string(const enum colour c)
	{ return colours[c]; }
static void colour_filler(enum colour *const c)
	{ *c = (unsigned)rand() / (RAND_MAX / colour_size + 1); }
#define TRIE_NAME colour
#define TRIE_KEY enum colour /* This and . . . */
#define TRIE_KEY_TO_STRING &colour_string /* this must be mutually set. */
#define TRIE_TEST &colour_filler
#define TRIE_TO_STRING
#include "../src/trie.h"


/* An unsigned value associated with an external string as a map. This is a
 convenience that defines `mapint_trie_entry` with key and value already there.
 We expect it to be slightly slower on update because the entry is larger. */
/** Generate a `key` (from `global_pool`) and `value`. */
static void mapint_filler(const char **const key, unsigned *const value) {
	assert(key), str32_filler(key);
	assert(value), *value = 42;
}
#define TRIE_NAME mapint
#define TRIE_VALUE unsigned
#define TRIE_TO_STRING
#define TRIE_TEST &mapint_filler
#include "../src/trie.h"


/** This is functionally very similar to a map, except there is only one
 pointer. The key is now part of the value, so one has to define projection
 functions. */
struct foo { int foo; const char *key; };
static const char *foo_read_key(const struct foo *const foo)
	{ return foo->key; }
static void foo_filler(struct foo *const foo)
	{ foo->foo = 42; str32_filler(&foo->key); }
#define TRIE_NAME foo
#define TRIE_VALUE struct foo
#define TRIE_KEY_IN_VALUE &foo_read_key
#define TRIE_TO_STRING
#define TRIE_TEST &foo_filler
#include "../src/trie.h"


/* Stores a value in the leaf itself and not externally. This structure is
 sensitive to the size of the leaves; optimally, would be a pointer's length,
 (which this example is on 64-byte machines.) The attraction of this is not
 that it is faster, but it's convenient to not store keys somewhere else,
 (somewhere bigger.) */
struct str8 { char str[8]; };
static const char *str8_read_key(const struct str8 *const s) { return s->str; }
static void str8_filler(struct str8 *const s) {
#if 0
	/* This is not enough range. */
	orcish(s->str, sizeof s->str);
#else
	static const char alphabet[] = "-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz~";
	unsigned i;
	for(i = 0; i < 7; i++) s->str[i]
		= alphabet[rand() / (RAND_MAX / ((int)sizeof alphabet - 1) + 1)];
	s->str[i] = '\0';
#endif
}
#define TRIE_NAME str8
#define TRIE_VALUE struct str8
#define TRIE_KEY_IN_VALUE &str8_read_key
#define TRIE_TEST &str8_filler
#define TRIE_TO_STRING
#include "../src/trie.h"


/* <https://en.wikipedia.org/wiki/List_of_brightest_stars> and light-years from
 Sol. */
#define STARS \
	X(Sol, 0), X(Sirius, 8.6), X(Canopus, 310), X(Rigil Kentaurus, 4.4), \
	X(Toliman, 4.4), X(Arcturus, 37), X(Vega, 25), X(Capella, 43), \
	X(Rigel, 860), X(Procyon, 11), X(Achernar, 139), X(Betelgeuse, 700), \
	X(Hadar, 390), X(Altair, 17), X(Acrux, 320), X(Aldebaran, 65), \
	X(Antares, 550), X(Spica, 250), X(Pollux, 34), X(Fomalhaut, 25), \
	X(Deneb, 2615), X(Mimosa, 280), X(Regulus, 79), X(Adhara, 430), \
	X(Shaula, 570), X(Castor, 52), X(Gacrux, 88), X(Bellatrix, 240), \
	X(Elnath, 130), X(Miaplacidus, 110), X(Alnilam, 2000), X(Regor, 840), \
	X(Alnair, 100), X(Alioth, 81), X(Alnitak, 820), X(Dubhe, 120), \
	X(Mirfak, 590), X(Wezen, 1800), X(Sargas, 270), X(Kaus Australis, 140), \
	X(Avior, 630), X(Alkaid, 100), X(Menkalinan, 100), X(Atria, 420), \
	X(Alhena, 100), X(Peacock, 180), X(Alsephina, 80), X(Mirzam, 500), \
	X(Alphard, 180), X(Polaris, 430), X(Hamal, 66), X(Algieba, 130), \
	X(Diphda, 96), X(Mizar, 78), X(Nunki, 220), X(Menkent, 61), \
	X(Mirach, 200), X(Alpheratz, 97), X(Rasalhague, 47), X(Kochab, 130), \
	X(Saiph, 720), X(Denebola, 36), X(Algol, 93), X(Tiaki, 170), \
	X(Muhlifain, 130), X(Aspidiske, 690), X(Suhail, 570), X(Alphecca, 75), \
	X(Mintaka, 900), X(Sadr, 1500), X(Eltanin, 150), X(Schedar, 230), \
	X(Naos, 1080), X(Almach, 350), X(Caph, 54), X(Izar, 202), \
	X(2.30 (2.29–2.34var), 550), X(2.30 (2.29–2.31var), 380), \
	X(Dschubba, 400), X(Larawag, 65), X(2.35 (2.30–2.41var), 310), \
	X(Merak, 79), X(Ankaa, 77), X(Girtab, 460), X(Enif, 670), X(Scheat, 200), \
	X(Sabik, 88), X(Phecda, 84), X(Aludra, 2000), X(Markeb, 540), \
	X(Navi, 610), X(Markab, 140), X(Aljanah, 72), X(Acrab, 404)
struct star { const char *name; double distance; };
#define X(A, B) { #A, B }
static const struct star stars[] = { STARS };
#undef X
static const size_t stars_size = sizeof stars / sizeof *stars;
static const char *star_read_key(const struct star *const star)
	{ return star->name; }
static void star_filler(struct star *const star) {
	const struct star *table = stars
		+ (unsigned)rand() / (RAND_MAX / stars_size + 1);
	star->name = table->name;
	star->distance = table->distance;
}
#define TRIE_NAME star
#define TRIE_VALUE struct star
#define TRIE_KEY_IN_VALUE &star_read_key
#define TRIE_TEST &star_filler
#define TRIE_TO_STRING
#include "../src/trie.h"


/** This is an external pointer; the trie is just an index. We will be
 responsible for assigning keys. */
struct keyval { char key[12]; int value; };
#define POOL_NAME keyval
#define POOL_TYPE struct keyval
#include "pool.h"
static struct keyval_pool kv_pool;
static const char *keyval_read_key(/*fixme! If possible? const*/
	struct keyval *const*const kv_ptr)
	{ return (*kv_ptr)->key; }
static void keyval_filler(struct keyval **const kv_ptr) {
	struct keyval *kv = keyval_pool_new(&kv_pool);
	assert(kv); /* Not testing malloc. */
	kv->value = rand() / (RAND_MAX / 1098 + 1) - 99;
	orcish(kv->key, sizeof kv->key);
	*kv_ptr = kv;
}
#define TRIE_NAME keyval
#define TRIE_VALUE struct keyval *
#define TRIE_KEY_IN_VALUE &keyval_read_key
#define TRIE_TEST &keyval_filler
#define TRIE_TO_STRING
#include "../src/trie.h"


#if 0
	for( ; i; i--) {
		int is;
		show = 1/*!(i & (i - 1))*/;
		if(show) trie_str_no++;
		if(show) printf("\"%s\" remove.\n", str_array[i - 1]);
		is = str_trie_remove(&strs, str_array[i - 1]);
		if(show) trie_str_graph(&strs, "graph/str-deleted.gv");
		for(j = 0; j < sizeof str_array / sizeof *str_array; j++) {
			const char *get = str_trie_get(&strs, str_array[j]);
			const int want_to_get = j < i - 1;
			printf("Test get(%s) = %s, (%swant to get.)\n",
				str_array[j], get ? get : "<didn't find>",
				want_to_get ? "" : "DON'T ");
			assert(!(want_to_get ^ (get == str_array[j])));
		}
	}
#endif

static void contrived_test(void) {
	const char *words[] = {
		"foo", "bar", "baz", "quxx",
		"a", "b", "c", "ba", "bb", "", "A", "Z", "z",
		"a", "b", "â",
		"foobar", "foo", "dictionary", "dictionaries"
	};
	unsigned i;
	struct str_trie t = str_trie();
	struct str_trie_cursor cur;
	printf("Contrived manual test of set <str>trie.\n");
	for(i = 0; i < sizeof words / sizeof *words; i++) {
		/* printf("word: %s\n", words[i]); */
		switch(str_trie_try(&t, words[i])) {
		case TRIE_ERROR: assert(0); break;
		case TRIE_UNIQUE: break;
		case TRIE_PRESENT: printf("\"%s\" already there.\n", words[i]);
			continue;
		}
		trie_str_graph(&t, "graph/contrived-insert.gv", i);
	}
	for(i = 0; i < sizeof words / sizeof *words; i++) {
		const char *get;
		int success = str_trie_query(&t, words[i], &get);
		assert(success && words[i] == get);
	}
	str_trie_prefix(&t, "b", &cur);
	printf("b: %lu\n", str_trie_size(&cur));
	str_trie_prefix(&t, "d", &cur);
	printf("d: %lu\n", str_trie_size(&cur));
	str_trie_prefix(&t, "f", &cur);
	printf("f: %lu\n", str_trie_size(&cur));
	str_trie_prefix(&t, "q", &cur);
	printf("q: %lu\n", str_trie_size(&cur));
	str_trie_(&t);
}

int main(void) {
	unsigned seed = (unsigned)clock();
	srand(seed), rand(), printf("Seed %u.\n", seed);
	errno = 0;
	contrived_test(), str32_pool_clear(&global_pool);
#if 0
	str_trie_test(), str32_pool_clear(&global_pool); /* Key set. */
	colour_trie_test(); /* Custom key set with enum string backing. */
	mapint_trie_test(), str32_pool_clear(&global_pool); /* `string -> int`. */
	foo_trie_test(), str32_pool_clear(&global_pool); /* Custom value. */
	str8_trie_test(); /* Small key set with no dependancy on outside keys. */
	star_trie_test(); /* Custom value with enum strings backing. */
	keyval_trie_test(); /*  */
#endif
	str32_pool_(&global_pool); /* Destroy global string pool. */
	return EXIT_SUCCESS;
}

/* Test Trie. */

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


/** Generate a random name and assign it to `pointer`. */
static void str32_filler(const char **const key_p) {
	struct str32 *backing = str32_pool_new(&global_pool);
	assert(backing && key_p);
	orcish(backing->str, sizeof backing->str);
	*key_p = backing->str;
}
/* A set of strings. Really don't have any parameters here. */
#define TRIE_NAME str
#define TRIE_TO_STRING
#define TRIE_TEST &str32_filler
#include "../src/trie.h"


#if 0
/** Generate a `key_p` and `value` from `global_pool`. */
static void mapint_filler(const char **const key_p, unsigned *const value) {
	assert(key_p), str32_filler(key_p);
	assert(value), *value = 42;
}
/* An unsigned value associated with string as a map. */
#define TRIE_NAME mapint
#define TRIE_VALUE unsigned
#define TRIE_TO_STRING
#define TRIE_TEST &mapint_filler
#include "../src/trie.h"
#endif


#if 0
struct foo { int foo; const char *key; };
static const char **foo_key(struct foo *const foo) { return &foo->key; }
static void foo_filler(struct foo *const foo)
	{ foo->foo = 42; str32_filler(&foo->key); }
/* A structure including a string that's used as the key. */
#define TRIE_NAME foo
#define TRIE_VALUE struct foo
#define TRIE_KEY_IN_VALUE &foo_key
#define TRIE_TO_STRING
#define TRIE_TEST &foo_filler
#include "../src/trie.h"
#endif

#if 0
/* This is a custom key that uses `TRIE_KEY` and `TRIE_KEY_TO_STRING`. */
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
static void colour_filler(enum colour *const c)
	{ *c = (unsigned)rand() / (RAND_MAX / colour_size + 1); }
static const char *colour_string(const enum colour c)
	{ return colours[c]; }
#define TRIE_NAME colour
#define TRIE_KEY enum colour
#define TRIE_KEY_TO_STRING &colour_string
#define TRIE_TEST &colour_filler
#define TRIE_TO_STRING
#include "../src/trie.h"
#endif



#if 0

/* <https://en.wikipedia.org/wiki/List_of_brightest_stars> and light-years from
 Sol. This is just a little bit more complex than colours, storing a pointer to
 a static name and the distance in a struct. */
#define PARAM2(A, B) { #A, B }
#define STARS(X) \
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
/* Tau Ceti? */
struct star { char *name; double distance; };
static const struct star stars[] = { STARS(PARAM2) };
static const size_t stars_size = sizeof stars / sizeof *stars;
static void star_filler(struct star *const s) {
	const struct star *r = stars
		+ (unsigned)rand() / (RAND_MAX / stars_size + 1);
	s->name = r->name;
	s->distance = r->distance;
}
static const char *star_key(const struct star *const star)
	{ return star->name; }
#define TRIE_NAME star
#define TRIE_VALUE struct star
#define TRIE_KEY_IN_VALUE &star_key
#define TRIE_TEST &star_filler
#define TRIE_TO_STRING
#include "../src/trie.h"

/* Stores a value in the item itself. If the string changes while in the trie,
 the trie is now undefined, don't do that. */
struct str4 { char value[4]; };
static void str4_filler(struct str4 *const s)
	{ orcish(s->value, sizeof s->value); }
static const char *str4_key(const struct str4 *const s) { return s->value; }
#define TRIE_NAME str4
#define TRIE_VALUE struct str4
#define TRIE_KEY_IN_VALUE &str4_key
#define TRIE_TEST &str4_filler
#define TRIE_TO_STRING
#include "../src/trie.h"

/* This is organized, alphabetized, and supports range-queries by key. */
struct keyval { char key[12]; int value; };
static void keyval_filler(struct keyval *const kv)
	{ kv->value = rand() / (RAND_MAX / 1098 + 1) - 99;
	orcish(kv->key, sizeof kv->key); }
static const char **keyval_key(const struct keyval *const kv)
	{ return &kv->key; }
#define TRIE_NAME keyval
#define TRIE_VALUE struct keyval
#define TRIE_KEY_IN_VALUE &keyval_key
#define TRIE_TEST &keyval_filler
#define TRIE_TO_STRING
#include "../src/trie.h"

#endif /* 0 */



#if 0
/** Manual testing for default string trie, that is, no associated information,
 just a set of `char *`. */
static void contrived_str_test(void) {
	struct str_trie strs = str_trie();
	size_t i, j;
	int show;
	size_t str_array_size = sizeof str_array / sizeof *str_array;
	printf("Contrived manual test of string set trie.\n");
	trie_str_no = 0;
	for(i = 0; i < sizeof str_array / sizeof *str_array; i++) {
		show = 1;//!((i + 1) & i) || i + 1 == str_array_size;
		if(show) trie_str_no++;
		if(show) printf("%lu: adding \"%s\".\n",
			(unsigned long)i, str_array[i]);
		if(!str_trie_try(&strs, str_array[i]))
			{ printf("%lu: %s not unique?\n", (unsigned long)i, str_array[i]);
			assert(0); continue; }
		if(show) trie_str_graph(&strs, "graph/str-add.gv");
		for(j = 0; j <= i; j++) {
			const char *sz = str_trie_get(&strs, str_array[j]);
			/*printf("Test get(\"%s\") = \"%s\".\n",
				str_array[j], sz ? sz : "<didn't find>");*/
			assert(sz == str_array[j]);
		}
	}
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
	str_trie_(&strs);
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
	printf("Contrived:\n");
	for(i = 0; i < sizeof words / sizeof *words; i++) {
		printf("word: %s\n", words[i]);
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
		assert(success);
		printf("get: %s\n", get);
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
	str_trie_test(), str32_pool_clear(&global_pool); /* key set */
	//int_trie_test(), str32_pool_clear(&global_pool); /* key -> int map */
	//foo_trie_test(), str32_pool_clear(&global_pool); /* custom with pointers */
	//colour_trie_test();
	/*star_trie_test();
	str4_trie_test();
	keyval_trie_test();*/
	str32_pool_(&global_pool);
	return EXIT_SUCCESS;
}

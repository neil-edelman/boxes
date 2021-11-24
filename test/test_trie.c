/* Test Trie. */

#include <stdlib.h> /* EXIT malloc free rand */
#include <stdio.h>  /* *printf */
#include <assert.h> /* assert */
#include <errno.h>  /* errno */
#include <time.h>   /* clock time */
#include "orcish.h"

/* A set of strings. `TRIE_TO_STRING` and `TRIE_TEST` are for graphing; one
 doesn't need them otherwise. */
/** Manually tested in <fn:contrived_str_test>. This will not, and can not
 work, leaving the strings uninitialized. Do _not_ call <fn:str_trie_test>. */
static void str_filler(const char *c) { assert(c != 0); }
#define TRIE_NAME str
#define TRIE_TO_STRING
#define TRIE_TEST &str_filler
#include "../src/trie.h"

/* You can have an `enum` in a `trie`, pointing to a fixed set of strings. */
#define PARAM(A) A
#define STRINGIZE(A) #A
#define COLOUR(X) \
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

/* <https://en.wikipedia.org/wiki/List_of_brightest_stars> and light-years from
 Sol. This is just a little bit more complex than colours, storing a pointer to
 a static name and the distance in a struct. */
#define PARAM2(A, B) { #A, B }
#define STARS(X) \
	X(Sun, 0), X(Sirius, 8.6), X(Canopus, 310), X(Rigil Kentaurus, 4.4), \
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
#define TRIE_TYPE struct star
#define TRIE_KEY &star_key
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
#define TRIE_TYPE struct str4
#define TRIE_KEY &str4_key
#define TRIE_TEST &str4_filler
#define TRIE_TO_STRING
#include "../src/trie.h"

/* This is organized, alphabetized, and supports range-queries by key. */
struct keyval { char key[12]; int value; };
static void keyval_filler(struct keyval *const kv)
	{ kv->value = rand() / (RAND_MAX / 1098 + 1) - 99;
	orcish(kv->key, sizeof kv->key); }
static const char *keyval_key(const struct keyval *const kv)
	{ return kv->key; }
#define TRIE_NAME keyval
#define TRIE_TYPE struct keyval
#define TRIE_KEY &keyval_key
#define TRIE_TEST &keyval_filler
#define TRIE_TO_STRING
#include "../src/trie.h"

/** Manual testing for default string trie, that is, no associated information,
 just a set of `char *`. */
static void contrived_str_test(void) {
	struct str_trie strs = TRIE_IDLE;
	size_t i, j;
	const char *str_array[] = {"a", "b", "c", "ba", "bb",
		"", "A", "Z", /*"a",*/ "z", "â", "foobar", "foo" };
	for(i = 0; i < sizeof str_array / sizeof *str_array; i++) {
		/* The items in the array are unique, right? */
		if(!str_trie_add(&strs, str_array[i]))
			{ printf("This does not make sense.\n"); assert(0); continue; }
		for(j = 0; j <= i; j++) {
			const char *sz = str_trie_get(&strs, str_array[j]);
			printf("test get(%s) = %s\n",
				str_array[j], sz ? sz : "<didn't find>");
			assert(sz == str_array[j]);
		}
	}
	trie_str_graph(&strs, "graph/str-contrived.gv");
	for( ; i; i--) {
		const char *const rm = str_trie_remove(&strs, str_array[i-1]);
		assert(rm);
		printf("\"%s\" removed.\n", rm);
		trie_str_no++;
		trie_str_graph(&strs, "graph/str-deleted.gv");
		for(j = 0; j < sizeof str_array / sizeof *str_array; j++) {
			const char *get = str_trie_get(&strs, str_array[j]);
			printf("test get(%s) = %s\n",
				str_array[j], get ? get : "<didn't find>");
			assert((j >= i-1) ^ (get == str_array[j]));
		}
	}
	str_trie_(&strs);
}

int main(void) {
	unsigned seed = (unsigned)clock();
	srand(seed), rand(), printf("Seed %u.\n", seed);
	contrived_str_test();
	colour_trie_test();
	star_trie_test();
	str4_trie_test();
	keyval_trie_test();
	return EXIT_SUCCESS;
}

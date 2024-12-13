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
#include "../src/pool.h"
static struct str32_pool str_pool; /* Global random string buffer. */


/* A set of strings stored somewhere else; one must keep the storage for the
 duration of the trie*. Good if one needs a prefix-tree with string literals.
 *Unless one only wants to use <fn:<T>match>, which only looks at the
 index and not the keys themselves; the key strings are not accessed, then. */
/** Generate a random name from `global_pool` and assign it to `key`. */
static void str_filler(const char **const key) {
	struct str32 *backing = str32_pool_new(&str_pool);
	/* Unlikely to fail, but for tests, we don't have the set-up to back-out. */
	assert(backing && key); if(!backing || !key) exit(EXIT_FAILURE);
	orcish(backing->str, sizeof backing->str);
	*key = backing->str;
}
#define TRIE_NAME str
#define TRIE_TO_STRING /* Uses the keys as strings. For test. */
#define TRIE_TEST
#include "../src/trie.h"


static void contrived_test(void) {
	const char *const words[] = { "foo", "bar", "baz", "quxx",
		"a", "b", "c", "ba", "bb", "", "A", "Z", "z",
		"a", "b", "â", "cc", "ccc", "cccc", "ccccc", "cccccc",
		"foobar", "foo", "dictionary", "dictionaries" };
	unsigned i, count_insert, count_retrieve, count3 = 0, first_letters[UCHAR_MAX];
	struct str_trie t = str_trie();
	enum trie_result r;
	int success;
	/* Histogram of letters. */
	memset(first_letters, 0, sizeof first_letters);
	printf("Contrived manual test of set <str>trie.\n");

	/* Info about offsets. */
	printf("offset in <str>tree:\n"
		" bsize: %lu\n"
		" branch: %lu\n"
		" bmp: %lu\n"
		" leaf: %lu\n"
		" whole struct: %lu\n",
		(unsigned long)offsetof(struct private_str_trie_tree, bsize),
		(unsigned long)offsetof(struct private_str_trie_tree, branch),
		(unsigned long)offsetof(struct private_str_trie_tree, bmp),
		(unsigned long)offsetof(struct private_str_trie_tree, leaf),
		(unsigned long)sizeof(struct private_str_trie_tree));
	assert(CHAR_BIT == 8 && ' ' ^ '!' == 1); /* Assumed UTF-8 for tests. */
	errno = 0;

	/* Test limits of tries. */
	r = str_trie_try(&t, "aaaaaaa aaaaaaa aaaaaaa aaaaaaa ");
	assert(r == TRIE_ABSENT);
	r = str_trie_try(&t, "aaaaaaa aaaaaaa aaaaaaa aaaaaaa ä"); /* 256 */
	assert(r == TRIE_ERROR && errno == EILSEQ), errno = 0;
	r = str_trie_try(&t, "aaaaaaa aaaaaaa aaaaaaa aaaaaaa!"); /* 255 */
	assert(r == TRIE_ABSENT);
	r = str_trie_try(&t, "aaaaaaa aaaaaaa aaaaaaa aaaaaaa ä"); /* 0 */
	assert(r == TRIE_ABSENT);
	success = str_trie_remove(&t, "aaaaaaa aaaaaaa aaaaaaa aaaaaaa!");
	assert(!success && errno == EILSEQ), errno = 0;
	private_str_trie_graph(&t, "graph/trie/contrived-max.gv", 0);
	str_trie_clear(&t);

	/* Insert all words. */
	for(count_insert = 0, i = 0; i < sizeof words / sizeof *words; i++) {
		const char *const word = words[i];
		/* printf("word: %s\n", word); */
		switch(str_trie_try(&t, word)) {
		case TRIE_ERROR:
			perror("trie"); assert(0); break;
		case TRIE_ABSENT:
			count_insert++;
			first_letters[(unsigned char)*word]++;
			break;
		case TRIE_PRESENT:
			printf("\"%s\" already there.\n", word); continue;
		}
		private_str_trie_graph(&t, "graph/trie/contrived-insert.gv", i);
	}
	for(i = 0; i < sizeof words / sizeof *words; i++) {
		const char *const get = str_trie_get(&t, words[i]);
		assert(get && !strcmp(words[i], get));
	}

	/* Add up all the letters; should be equal to the overall count. */
	for(count_retrieve = 0, i = 0;
		i < sizeof first_letters / sizeof *first_letters; i++) {
		char letter[2];
		unsigned count_letter = 0;
		struct str_trie_cursor cur;
		const char *str;
		int output = 0;
		letter[0] = (char)i, letter[1] = '\0';
		for(cur = str_trie_prefix(&t, letter); str_trie_exists(&cur);
			str_trie_next(&cur)) {
			str = str_trie_entry(&cur);
			count_letter++;
			printf("%s %u:\"%s\":<%s>:%u", output ? ", " : "", i, letter, str, count_letter);
			output = 1;
		}
		if(output) printf("\n");
		if(i) {
			assert(count_letter == first_letters[i]);
			count_retrieve += count_letter;
		} else { /* Sentinel. */
			count3 = count_letter;
			if(str_trie_get(&t, "")) count_retrieve++;
		}
	}
	assert(count_retrieve == count_insert);
	assert(count3 == count_insert);
	{
		r = str_trie_try(&t, "a"), assert(r == TRIE_PRESENT);
		r = str_trie_try(&t, "yo"), assert(r == TRIE_ABSENT);
		private_str_trie_graph(&t, "graph/trie/yo.gv", 0);
		success = str_trie_remove(&t, "yo"), assert(success);
		private_str_trie_graph(&t, "graph/trie/yo.gv", 1);
	}
	success = str_trie_remove(&t, "yo"), assert(!success);
	for(i = 0; i < sizeof words / sizeof *words; i++) {
		const char *const word = words[i];
		printf("Delete <%s>.\n", word);
		success = str_trie_remove(&t, word);
		if(success) count_insert--;
		else printf("Didn't find <%s>.\n", word);
		private_str_trie_graph(&t, "graph/trie/contrived-delete.gv", i);
	}
	assert(!count_insert);
	str_trie_(&t);
}


#if 0
/* Set of `enum colour`. The set is necessarily alphabetically ordered, and can
 efficiently tell which colour names are starting with a prefix. This stores
 2 bytes overhead and an `enum colour` for each in the set. It forwards one
 request _per_ query _via_ `colour_string` (required iff `TRIE_KEY`) to a
 static string array `colours`. */
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
#define TRIE_KEY enum colour
#define TRIE_TO_STRING
#define TRIE_TEST
#include "../src/trie.h"

static void fixed_colour_test(void) {
	struct colour_trie trie = colour_trie();
	struct colour_trie_iterator it;
	int ret;
	if(!colour_trie_try(&trie, Black)
		|| !colour_trie_try(&trie, Red)
		|| !colour_trie_try(&trie, Yellow)
		|| !colour_trie_try(&trie, Lime)
		|| !colour_trie_try(&trie, Steel)) { assert(0); goto catch; }
	trie_colour_graph(&trie, "graph/trie/colour-fixed.gv", 0);
	colour_trie_remove(&trie, "Steel");
	trie_colour_graph(&trie, "graph/trie/colour-fixed.gv", 1);
	it = colour_trie_prefix(&trie, "");
	ret = colour_trie_next(&it), assert(ret && colour_trie_entry(&it) == Black);
	ret = colour_trie_next(&it), assert(ret && colour_trie_entry(&it) == Lime);
	ret = colour_trie_next(&it), assert(ret && colour_trie_entry(&it) == Red);
	ret = colour_trie_next(&it), assert(ret && colour_trie_entry(&it) == Yellow);
	ret = colour_trie_next(&it), assert(!ret);
	goto finally;
catch:
	perror("fixed colour trie");
finally:
	colour_trie_(&trie);
}


/* Public separating header/body. */
#define TRIE_NAME public
#define TRIE_KEY enum colour
#define TRIE_HEAD
#include "../src/trie.h"
static const char *public_string(const enum colour c)
	{ return colour_string(c); }
static void public_filler(enum colour *const c) { colour_filler(c); }
#define TRIE_NAME public
#define TRIE_KEY enum colour
#define TRIE_TO_STRING
#define TRIE_TEST
#define TRIE_BODY
#include "../src/trie.h"


/* Stores a value in the leaf itself and not externally. Optimally, would be a
 pointer's length. It's entirely self-contained inside the trie, with the
 downside of moving data. */
struct str8 { char str[8]; };
static const char *str8_key(const struct str8 *const s) { return s->str; }
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
#define TRIE_ENTRY struct str8
#define TRIE_TO_STRING
#define TRIE_TEST
#include "../src/trie.h"


/** A map from string key implemented as a pointer to unsigned value. We no
 longer have a way to output the keys automatically. The user must copy the key
 into the pointer manually. */
struct kv1 { const char *key; unsigned value; };
/** @return Picks the key from `kv`. @implements <typedef:<T>key> */
static const char *kv1_key(const struct kv1 *const kv) { return kv->key; }
/** Generate a `key` (from `str_pool`) and `value`.
 @implements <typedef:<T>filler> */
static void kv1_filler(struct kv1 *const kv)
	{ assert(kv), str_filler(&kv->key), kv->value = 42; }
#define TRIE_NAME kv1
#define TRIE_ENTRY struct kv1
#define TRIE_TO_STRING
#define TRIE_TEST
#include "../src/trie.h"


/** Or we could have the key directly in the value. */
struct kv2 { char key[12]; int value; };
static const char *kv2_key(const struct kv2 *const kv) { return kv->key; }
static void kv2_filler(struct kv2 *const kv) {
	orcish(kv->key, sizeof kv->key);
	kv->value = rand() / (RAND_MAX / 1098 + 1) - 99;
}
#define TRIE_NAME kv2
#define TRIE_ENTRY struct kv2
#define TRIE_TO_STRING
#define TRIE_TEST
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
static const char *star_key(const struct star *const star)
	{ return star->name; }
static void star_filler(struct star *const star) {
	const struct star *table = stars
		+ (unsigned)rand() / (RAND_MAX / stars_size + 1);
	star->name = table->name;
	star->distance = table->distance;
}
#define TRIE_NAME star
#define TRIE_ENTRY struct star
#define TRIE_TEST
#define TRIE_TO_STRING
#include "../src/trie.h"


static void article_test(void) {
	/* Set TRIE_LEFT_MAX = 5. */
	unsigned list1[] = { 0, 5, 6, 10 },
		list2[] = { 1, 2, 6, 8, 11, 13, 17, 18, 22, 25, 26, 49, 19, 7 },
		i;
	struct star_trie trie = star_trie();
	for(i = 0; i < sizeof list1 / sizeof *list1; i++) {
		const struct star *const star = stars + list1[i];
		struct star *entry;
		if(!star_trie_try(&trie, star->name, &entry)) { assert(0); break; }
		entry->name = star->name;
		entry->distance = star->distance;
	}
	trie_star_graph(&trie, "graph/trie/article.gv", 0);
	star_trie_clear(&trie);
	for(i = 0; i < sizeof list2 / sizeof *list2; i++) {
		const struct star *const star = stars + list2[i];
		struct star *entry;
		if(!star_trie_try(&trie, star->name, &entry)) { assert(0); break; }
		entry->name = star->name;
		entry->distance = star->distance;
		trie_star_graph(&trie, "graph/trie/article.gv", i + 1000);
	}
	trie_star_graph(&trie, "graph/trie/article.gv", 1);
	star_trie_(&trie);
}
#endif

int main(void) {
	unsigned seed = (unsigned)clock();
	srand(seed), rand(), printf("Seed %u.\n", seed);
	errno = 0;
	str_trie_test(), str32_pool_clear(&str_pool);
	contrived_test(), str32_pool_clear(&str_pool);
#if 0
	fixed_colour_test();
	colour_trie_test();
	public_trie_test();
	str8_trie_test();
	kv1_trie_test(), str32_pool_(&str_pool);
	kv2_trie_test();
	star_trie_test();
	article_test();
#endif
	return EXIT_SUCCESS;
}

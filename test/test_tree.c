/* Test Trie. */

#include <stdlib.h> /* EXIT malloc free rand */
#include <stdio.h>  /* *printf */
#include <assert.h> /* assert */
#include <errno.h>  /* errno */
#include <time.h>   /* clock time */

/* Unsigned numbers: this is the minimum tree, but not useful to test. */
#define TREE_NAME foo
#include "../src/tree.h"


/* Unsigned numbers: testing framework. */
/** @implements <typedef:<PB>action_fn> */
static void int_filler(unsigned *x)
	{ *x = (unsigned)rand() / (RAND_MAX / 1000 + 1); }
/** @implements <typedef:<PSZ>to_string_fn> */
static void int_to_string(const unsigned *x, char (*const z)[12])
	{ sprintf(*z, "%u", *x); }
#define TREE_NAME int
#define TREE_TEST &int_filler
#define TREE_EXPECT_TRAIT
#include "../src/tree.h"
#define TREE_TO_STRING &int_to_string
#include "../src/tree.h"


/* Unsigned numbers and values. Prototype a value. */
struct pair_tree_test;
static void pair_filler(struct pair_tree_test *);
struct pair_tree_entry_c;
static void pair_to_string(const struct pair_tree_entry_c, char (*)[12]);
#define TREE_NAME pair
#define TREE_VALUE unsigned
#define TREE_TEST &pair_filler
#define TREE_EXPECT_TRAIT
#include "../src/tree.h"
#define TREE_TO_STRING &pair_to_string
#include "../src/tree.h"
/** @implements <typedef:<PB>action_fn> */
static void pair_filler(struct pair_tree_test *x) {
	int_filler(&x->key);
	int_filler(&x->value), x->value += 10000;
	printf("generated %u->%u\n", x->key, x->value);
}
/** @implements <typedef:<PSZ>to_string_fn> */
static void pair_to_string(const struct pair_tree_entry_c x,
	char (*const z)[12]) { sprintf(*z, "%u→%u", *x.key, *x.value); } /* 3+3+5 */


/* <https://en.wikipedia.org/wiki/List_of_brightest_stars> and light-years from
 Sol. We define a tree of ascending distances. */
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
	X(2.30 2.29-2.34var, 550), X(2.30 2.29-2.31var, 380), \
	X(Dschubba, 400), X(Larawag, 65), X(2.35 2.30-2.41var, 310), \
	X(Merak, 79), X(Ankaa, 77), X(Girtab, 460), X(Enif, 670), X(Scheat, 200), \
	X(Sabik, 88), X(Phecda, 84), X(Aludra, 2000), X(Markeb, 540), \
	X(Navi, 610), X(Markab, 140), X(Aljanah, 72), X(Acrab, 404)
#define X(n, m) #n
static const char *star_names[] = { STARS };
#undef X
#define X(n, m) m
static double star_distances[] = { STARS };
#undef X
static size_t star_size = sizeof star_names / sizeof *star_names;
struct star_tree_test;
static void star_filler(struct star_tree_test *);
struct star_tree_entry_c;
static void star_to_string(struct star_tree_entry_c, char (*)[12]);
/* It is impossible to have a `const char *` without getting warnings about
 duplicate `const`. This is because we want them to be `const` sometimes. This
 is a workaround. */
typedef const char *const_str;
#define TREE_NAME star
#define TREE_KEY double
/*#define TREE_MULTIPLE_KEY work in progress; do I want to have extra stuff in
 the tree, or the iterator? Both do not sound fun. */ /* Different stars can
 have the same distance. */
#define TREE_VALUE const_str
#define TREE_TEST &star_filler
#define TREE_EXPECT_TRAIT
#include "../src/tree.h"
#define TREE_TO_STRING &star_to_string
#include "../src/tree.h"
/** @implements <typedef:<PB>action_fn> */
static void star_filler(struct star_tree_test *x) {
	const unsigned i = (unsigned)rand() / (RAND_MAX / star_size + 1);
	x->key = star_distances[i], x->value = star_names[i];
}
/** @implements <typedef:<PSZ>to_string_fn> */
static void star_to_string(const struct star_tree_entry_c x,
	char (*const z)[12]) { sprintf(*z, "%.11s", *x.value); }


/* §6.7.2.1/P11 implementation defined; hopefully it will work. This is so
 convenient, but completely unspecified; the other option is to manually
 mask-off the bits for every value, and use a union name, which is painful. */
#include <stdint.h> /* C99 */
union date32 {
	uint32_t u32;
	struct { unsigned day : 5, month : 4, year : 23; } d; /* Usually works. */
};
static int entry_compare(const union date32 a, const union date32 b)
	{ return a.u32 > b.u32; }
struct entry_tree_test;
static void entry_filler(struct entry_tree_test *);
struct entry_tree_entry_c;
static void entry_to_string(struct entry_tree_entry_c, char (*)[12]);
#define TREE_NAME entry
#define TREE_KEY union date32
#define TREE_COMPARE &entry_compare
#define TREE_VALUE int
#define TREE_TEST &entry_filler
#define TREE_EXPECT_TRAIT
#include "../src/tree.h"
#define TREE_TO_STRING &entry_to_string
#include "../src/tree.h"
static void entry_filler(struct entry_tree_test *test) {
	test->key.u32 = (uint32_t)rand();
	test->key.d.year %= 10000;
	test->key.d.month = test->key.d.month % 12 + 1;
	test->key.d.day = test->key.d.day % 31 + 1;
	test->value = 42;
}
static void entry_to_string(const struct entry_tree_entry_c entry,
	char (*const z)[12]) {
	assert(entry.key->d.year < 10000 && entry.key->d.month
		&& entry.key->d.month <= 31
		&& entry.key->d.day && entry.key->d.day <= 31);
	sprintf(*z, "%u-%2.2u-%2.2u",
		entry.key->d.year % 10000, entry.key->d.month, entry.key->d.day);
}


static void manual_int(void) {
	struct int_tree equal = int_tree(), step = int_tree(),
		discrete = int_tree();
	/*size_t i;
	unsigned *x;*/
	struct int_tree_iterator it;
	unsigned *v;
	int ret;

	/*for(i = 0; i < 5; i++) if(!unsigned_tree_bulk_add(&equal, 0)) goto catch;
	for(i = 0; i < 15; i++) if(!unsigned_tree_bulk_add(&equal, 1)) goto catch;
	tree_unsigned_graph(&equal, "graph/manual-equal.gv");
	unsigned_tree_bulk_finish(&equal);
	tree_unsigned_graph(&equal, "graph/manual-equal-finalize.gv");
	x = unsigned_tree_get(&equal, 1), assert(x);
	printf("equal: x = %u\n", *x);
	ti._ = tree_unsigned_lower(&equal, 1);
	printf("equal: %s:%u\n", orcify(ti._.end.node), ti._.end.idx);*/

	if(!int_tree_bulk_add(&step, 100)
		|| !int_tree_bulk_add(&step, 200)
		|| !int_tree_bulk_add(&step, 300)) goto catch;
	tree_int_graph(&step, "graph/step.gv");
	ret = int_tree_bulk_finish(&step);
	assert(ret);
	tree_int_graph(&step, "graph/step-finalize.gv");
	it = int_tree_lower(&step, 50);
	printf("step: 50: %s:%u.\n", orcify(it._.ref.node), it._.ref.idx);
	v = int_tree_next(&it), assert(v && *v == 100);
	printf("It's %u.\n", *v);
	it = int_tree_lower(&step, 150);
	printf("step: 150: %s:%u\n", orcify(it._.ref.node), it._.ref.idx);
	v = int_tree_next(&it), assert(v && *v == 200);
	printf("It's %u.\n", *v);
	it = int_tree_lower(&step, 250);
	printf("step: 250: %s:%u\n", orcify(it._.ref.node), it._.ref.idx);
	v = int_tree_next(&it), assert(v && *v == 300);
	printf("It's %u.\n", *v);
	it = int_tree_lower(&step, 350);
	printf("step: 350: %s:%u\n", orcify(it._.ref.node), it._.ref.idx);
	v = int_tree_next(&it), assert(!v);
	printf("It's null.\n");
	v = int_tree_get_next(&step, 50), assert(v && *v == 100);
	v = int_tree_get_next(&step, 150), assert(v && *v == 200);
	v = int_tree_get_next(&step, 250), assert(v && *v == 300);
	v = int_tree_get_next(&step, 300), assert(v && *v == 300);
	v = int_tree_get_next(&step, 350), assert(!v);

	/* For m=3 add test. */
	int_tree_bulk_add(&discrete, 2);
	int_tree_bulk_add(&discrete, 4);
	int_tree_bulk_add(&discrete, 6);
	int_tree_bulk_add(&discrete, 8);
	int_tree_bulk_add(&discrete, 10);
	int_tree_bulk_add(&discrete, 12);
	int_tree_bulk_add(&discrete, 14);
	int_tree_bulk_add(&discrete, 16);
	int_tree_bulk_add(&discrete, 18);
	int_tree_bulk_add(&discrete, 20);
	int_tree_bulk_add(&discrete, 22);
	int_tree_bulk_add(&discrete, 24);
	int_tree_bulk_add(&discrete, 26);
	int_tree_bulk_add(&discrete, 28);
	int_tree_bulk_add(&discrete, 30);
	int_tree_bulk_add(&discrete, 32);
	int_tree_bulk_add(&discrete, 34);
	int_tree_bulk_add(&discrete, 36);
	int_tree_bulk_add(&discrete, 38);
	int_tree_bulk_add(&discrete, 40);
	int_tree_bulk_add(&discrete, 42);
	int_tree_bulk_add(&discrete, 44);
	int_tree_bulk_add(&discrete, 46);
	int_tree_bulk_add(&discrete, 48);
	int_tree_bulk_add(&discrete, 50);
	int_tree_bulk_add(&discrete, 52);
	int_tree_bulk_finish(&discrete);
	tree_int_graph(&discrete, "graph/discrete-1.gv");
	/*int_tree_add(&discrete, 27);*/
	int_tree_add(&discrete, 21);
	tree_int_graph(&discrete, "graph/discrete-2.gv");
	goto finally;
catch:
	perror("manual_unsigned");
	assert(0);
finally:
	int_tree_(&equal);
	int_tree_(&step);
	int_tree_(&discrete);
	printf("\n");
}

int main(void) {
	unsigned seed = (unsigned)clock();
	srand(seed), rand(), printf("Seed %u.\n", seed);
	int_tree_test();
	pair_tree_test();
	/* 649516 fails with two levels of equal keys at the end, (of course; it's
	 lower!) */
	star_tree_test();
	entry_tree_test();
	manual_int();
	return EXIT_SUCCESS;
}

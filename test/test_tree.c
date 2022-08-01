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
	{ /*assert(*x < 10000000000),*/ sprintf(*z, "%u", *x); }
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
	int_filler(&x->value), x->value += /*42*/10000;
	printf("generated %u->%u\n", x->key, x->value);
}
/** @implements <typedef:<PSZ>to_string_fn> */
static void pair_to_string(const struct pair_tree_entry_c x,
	char (*const z)[12])
	{ assert(*x.key < 1000 && *x.value < 100000); /* Arrow 3. */
	sprintf(*z, "%u→%u", *x.key, *x.value); }


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
	char (*const z)[12]) { sprintf(*z, "%.11s", *x.value/*"%u", *x.key*/); }


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
	struct int_tree between = int_tree(),
		rnd = int_tree(),
		even = int_tree(), even_clone = int_tree(),
		consecutive = int_tree(),
		removal = int_tree();
	size_t i;
	const size_t size_rnd = 100;
	/*unsigned *x;*/
	struct int_tree_iterator it;
	unsigned *v;
	int ret;

	/* Multi-maps. This is less useful.
	for(i = 0; i < 5; i++) if(!unsigned_tree_bulk_add(&equal, 0)) goto catch;
	for(i = 0; i < 15; i++) if(!unsigned_tree_bulk_add(&equal, 1)) goto catch;
	tree_unsigned_graph(&equal, "graph/manual-equal.gv");
	unsigned_tree_bulk_finish(&equal);
	tree_unsigned_graph(&equal, "graph/manual-equal-finalize.gv");
	x = unsigned_tree_get(&equal, 1), assert(x);
	printf("equal: x = %u\n", *x);
	ti._ = tree_unsigned_lower(&equal, 1);
	printf("equal: %s:%u\n", orcify(ti._.end.node), ti._.end.idx);*/

	/* Lookup between nodes. */
	if(!int_tree_bulk_add(&between, 100)
		|| !int_tree_bulk_add(&between, 200)
		|| !int_tree_bulk_add(&between, 300)) goto catch;
	ret = int_tree_bulk_finish(&between);
	assert(ret);
	tree_int_graph(&between, "graph/between.gv");
	it = int_tree_lower_iterator(&between, 50);
	printf("step: 50: %s:%u.\n", orcify(it._.i.node), it._.i.idx);
	v = int_tree_next(&it), assert(v && *v == 100);
	printf("It's %u.\n", *v);
	it = int_tree_lower_iterator(&between, 150);
	printf("step: 150: %s:%u\n", orcify(it._.i.node), it._.i.idx);
	v = int_tree_next(&it), assert(v && *v == 200);
	printf("It's %u.\n", *v);
	it = int_tree_lower_iterator(&between, 250);
	printf("step: 250: %s:%u\n", orcify(it._.i.node), it._.i.idx);
	v = int_tree_next(&it), assert(v && *v == 300);
	printf("It's %u.\n", *v);
	it = int_tree_lower_iterator(&between, 350);
	printf("step: 350: %s:%u\n", orcify(it._.i.node), it._.i.idx);
	assert(!it._.i.node);
	v = int_tree_next(&it), /*assert(!v) not ideal*/ assert(v && *v == 100);
	/* The value of the iterator. */
	v = int_tree_lower_value(&between, 50), assert(v && *v == 100);
	v = int_tree_lower_value(&between, 150), assert(v && *v == 200);
	v = int_tree_lower_value(&between, 250), assert(v && *v == 300);
	v = int_tree_lower_value(&between, 300), assert(v && *v == 300);
	v = int_tree_lower_value(&between, 350), assert(!v);

	/* Random. */
	for(i = 0; i < size_rnd; i++) {
		unsigned x = rand() & 65535;
		printf("__%u) add random value %u__\n", (unsigned)i, x);
		switch(int_tree_add(&rnd, x)) {
		case TREE_ERROR: goto catch;
		case TREE_YIELD: printf("%u already in tree\n", x); break;
		case TREE_UNIQUE: printf("%u added\n", x); break;
		}
		if(!(i & (i + 1)) || i == size_rnd - 1) {
			char fn[64];
			sprintf(fn, "graph/rnd-%u.gv", (unsigned)i);
			tree_int_graph(&rnd, fn);
		}
	}

#if TREE_ORDER < 10 /* <!-- small */

	{ /* Full add test, nodes, `\sum_{n=0}^{h} m^n = \frac{m^{h+1}-1}{m-1}`,
		 gives keys, `m^{h+1}-1`, three levels. */
		const size_t size = TREE_ORDER * TREE_ORDER * TREE_ORDER - 1;
		for(i = 0; i < size; i++) /* Even for odd spaces between them. */
			if(!int_tree_bulk_add(&even, ((unsigned)i + 1) * 2)) assert(0);
		int_tree_bulk_finish(&even); /* Does nothing, in this case. */
		tree_int_graph(&even, "graph/even-1.gv");
		for(i = 0; i <= size; i++) {
			char fn[64];
			if(!int_tree_clone(&even_clone, &even)) goto catch;
			if(!int_tree_add(&even_clone, (unsigned)i * 2 + 1)) goto catch;
			sprintf(fn, "graph/even-clone-%u.gv", (unsigned)i * 2 + 1);
			tree_int_graph(&even_clone, fn);
		}
	}

#if TREE_ORDER == 3
	{
		int in[TREE_ORDER * TREE_ORDER * TREE_ORDER - 1];
		const unsigned size = sizeof in / sizeof *in;
		unsigned n;
		memset(&in, 0, sizeof in);
		for(n = 0; n < size; n++) {
			if(!(int_tree_bulk_add(&removal, n + 1))) goto catch;
			in[n] = 1;
		}
		int_tree_bulk_finish(&even);
		/* Leaf, no redistributing needed. */
		tree_int_graph(&removal, "graph/removal-0.gv");
		int_tree_remove(&removal, 14), in[13] = 0;
		tree_int_graph(&removal, "graph/removal-1.gv");
		for(n = 0; n < size; n++)
			assert(int_tree_contains(&removal, n + 1) == in[n]);
		/* Leaf, free the node; distribute one level above. */
		int_tree_remove(&removal, 13), in[12] = 0;
		tree_int_graph(&removal, "graph/removal-2.gv");
		for(n = 0; n < size; n++)
			assert(int_tree_contains(&removal, n + 1) == in[n]);
		/* Other side. */
		int_tree_remove(&removal, 12), in[11] = 0;
		tree_int_graph(&removal, "graph/removal-3.gv");
		for(n = 0; n < size; n++)
			assert(int_tree_contains(&removal, n + 1) == in[n]);
		/* Merge. */
		int_tree_remove(&removal, 15), in[14] = 0;
		tree_int_graph(&removal, "graph/removal-4.gv");
		for(n = 0; n < size; n++)
			assert(int_tree_contains(&removal, n + 1) == in[n]);
	}
#endif

	{ /* One level. */
		const size_t size = TREE_ORDER + 1;
		char fn[64];
		for(i = 0; i < size; i++)
			if(!int_tree_bulk_add(&consecutive, (unsigned)i + 1)) goto catch;
		int_tree_bulk_finish(&consecutive);
		tree_int_graph(&consecutive, "graph/small.gv");
		int_tree_remove(&consecutive, size);
		int_tree_clear(&consecutive);
	}

	{ /* Consecutive. */
		const size_t size = 4 * TREE_ORDER - 1;
		char fn[64];
		const unsigned rm = 1;
		for(i = 0; i < size; i++) {
			unsigned x = (unsigned)i + 1;
			printf("__%u) Going to add consecutive %u__\n", (unsigned)i, x);
			switch(int_tree_add(&consecutive, x)) {
			case TREE_ERROR: goto catch;
			case TREE_YIELD: printf("%u already in tree\n", x); break;
			case TREE_UNIQUE: printf("%u added\n", x); break;
			}
			sprintf(fn, "graph/consecutive-%u.gv", (unsigned)i);
			tree_int_graph(&consecutive, fn);
		}
		for(i = 0; i < size; i++) {
			unsigned x = (unsigned)i + 1;
			printf("__%u) Going to remove %u__\n", (unsigned)i, x);
			if(!int_tree_remove(&consecutive, x)) assert(0);
			sprintf(fn, "graph/consecutive-rm-%u.gv", (unsigned)x);
			tree_int_graph(&consecutive, fn);
		}
	}

#endif /* small --> */

	goto finally;
catch:
	perror("manual_unsigned");
	assert(0);
finally:
	int_tree_(&between);
	int_tree_(&rnd);
	int_tree_(&removal);
	int_tree_(&even), int_tree_(&even_clone);
	int_tree_(&consecutive);
}

int main(void) {
	unsigned seed = (unsigned)clock();
	srand(seed), rand(), printf("Seed %u.\n", seed);
	manual_int();
	int_tree_test();
	pair_tree_test();
	star_tree_test();
	entry_tree_test();
	return EXIT_SUCCESS;
}

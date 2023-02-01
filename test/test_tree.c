#include <stdlib.h> /* EXIT malloc free rand */
#include <stdio.h>  /* *printf */
#include <assert.h> /* assert */
#include <errno.h>  /* errno */
#include <time.h>   /* clock time */


/* Unsigned numbers: this is the minimum tree, but not useful to test. */
#define TREE_NAME foo
#include "../src/tree.h"


/* For testing bounds. */
static void char_to_string(const char x, char (*const z)[12])
	{ (*z)[0] = x; (*z)[1] = '\0'; }
static void char_filler(char *x)
	{ *x = (char)(rand() / (RAND_MAX / ('z' - 'a' + 1) + 1)) + 'a'; }
#define TREE_NAME char
#define TREE_KEY char
#define TREE_ORDER 3
#define TREE_DEFAULT '_'
#define TREE_TO_STRING
#define TREE_TEST
#include "../src/tree.h"


/* Unsigned numbers: testing framework. */
/** @implements <typedef:<PSZ>to_string_fn> */
static void int_to_string(const unsigned x, char (*const z)[12])
	{ /*assert(*x < 10000000000),*/ sprintf(*z, "%u", x); }
/** @implements <typedef:<PB>action_fn> */
static void int_filler(unsigned *x)
	{ *x = (unsigned)rand() / (RAND_MAX / 1000 + 1); }
#define TREE_NAME int
#define TREE_TO_STRING
#define TREE_TEST
#include "../src/tree.h"


static void order3_filler(unsigned *x) { int_filler(x); }
static void order3_to_string(const unsigned x, char (*const z)[12])
	{ int_to_string(x, z); }
#define TREE_NAME order3
#define TREE_TEST
#define TREE_ORDER 3
#define TREE_TO_STRING
#include "../src/tree.h"


/* Unsigned numbers: testing framework. */
static void redblack_to_string(const unsigned key, const unsigned *const value,
	char (*const a)[12])
	{ /*assert(*x < 10000000000),*/ sprintf(*a, "%u", key); (void)value; }
static void redblack_filler(unsigned *const k, unsigned *const v) {
	*k = *v = (unsigned)rand() / (RAND_MAX / 1000 + 1); }
#define TREE_NAME redblack
#define TREE_VALUE unsigned
#define TREE_ORDER 4
#define TREE_TO_STRING
#define TREE_TEST
#include "../src/tree.h"


/* Unsigned numbers and values. Prototype a value. */
static void pair_filler(unsigned *const x, unsigned *const y) {
	int_filler(x);
	int_filler(y), *y += 10000;
	/*printf("generated %u->%u\n", x->key, x->value);*/
}
static void pair_to_string(const unsigned k, const unsigned *const v,
	char (*const a)[12])
	{ assert(k < 1000 && v && *v < 100000); sprintf(*a, "%u→%u", k, *v); }
#define TREE_NAME pair
#define TREE_VALUE unsigned
#define TREE_TEST
#define TREE_TO_STRING
#include "../src/tree.h"


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
static void star_filler(double *k, const char **v) {
	const unsigned i = (unsigned)rand() / (RAND_MAX / star_size + 1);
	*k = star_distances[i], *v = star_names[i];
}
static void star_to_string(const double k, const char *const*const v,
	char (*const a)[12]) { sprintf(*a, "%.11s", *v); (void)k; }
/* It is impossible to have a `const char *` without getting warnings about
 duplicate `const`. This is because we want them to be `const` sometimes. This
 is a workaround. */
typedef const char *const_str;
#define TREE_NAME star
#define TREE_KEY double
#define TREE_VALUE const_str
#define TREE_TEST
#define TREE_TO_STRING
#include "../src/tree.h"


/* §6.7.2.1/P11 implementation defined; hopefully it will work. This is so
 convenient, but completely unspecified; the other option is to manually
 mask-off the bits for every value, and use a union name, which is painful. */
#include <stdint.h> /* C99 -- Augh! mixing the standards. */
union date32 {
	uint32_t u32;
	struct { unsigned day : 5, month : 4, year : 23; } d; /* Usually works. */
};
static int entry_compare(const union date32 a, const union date32 b)
	{ return a.u32 > b.u32; }
static void entry_filler(union date32 *const k, int *const v) {
	k->u32 = (uint32_t)rand();
	k->d.year %= 10000;
	k->d.month = k->d.month % 12 + 1;
	k->d.day = k->d.day % 31 + 1;
	*v = 42;
}
static void entry_to_string(const union date32 k, const int *const v,
	char (*const z)[12]) {
	assert(k.d.year < 10000 && k.d.month && k.d.month <= 12
		&& k.d.day && k.d.day <= 31);
	sprintf(*z, "%u-%2.2u-%2.2u", k.d.year % 10000, k.d.month, k.d.day);
	(void)v;
}
#define TREE_NAME entry
#define TREE_KEY union date32
#define TREE_COMPARE
#define TREE_VALUE int
#define TREE_TEST
#define TREE_TO_STRING
#include "../src/tree.h"


/** Static bounds check. */
static void char_bounds(void) {
	struct char_tree tree = char_tree();
	struct char_tree_iterator it;
	const char correct_right[] = {
		/*a*/'b', 'b', 'd', 'd',
		/*e*/'f', 'f', 'h', 'h',
		/*i*/'j', 'j', '_', '_' },
	correct_left[] = {
		/*a*/'_', 'b', 'b', 'd',
		/*e*/'d', 'f', 'f', 'h',
		/*i*/'h', 'j', 'j', 'j' };
	char i;
	char_tree_bulk_add(&tree, 'b');
	char_tree_bulk_add(&tree, 'd');
	char_tree_bulk_add(&tree, 'f');
	char_tree_bulk_add(&tree, 'h');
	char_tree_bulk_add(&tree, 'j');
	char_tree_bulk_finish(&tree);
	tree_char_graph(&tree, "graph/char-bounds.gv");
	printf("right:\n");
	for(i = 'a'; i < 'm'; i++) {
		char right = char_tree_right(&tree, i);
		printf("%c\t%c\t(%c)\n", i, right, correct_right[(int)i-'a']);
		assert(right == correct_right[(int)i-'a']);
	}
	printf("left:\n");
	for(i = 'a'; i < 'm'; i++) {
		char left = char_tree_left(&tree, i);
		printf("%c\t%c\t(%c)\n", i, left, correct_left[(int)i-'a']);
		assert(left == correct_left[(int)i-'a']);
	}
	char_tree_(&tree);
}

/** Order 3 integer tree set; it's hard to make a simpler example. */
static void order3(void) {
	struct order3_tree between = order3_tree(),
		rnd = order3_tree(),
		even = order3_tree(), even_clone = order3_tree(),
		consecutive = order3_tree(),
		removal = order3_tree(),
		copy = order3_tree();
	unsigned i;
	const unsigned size_rnd = 100;
	struct order3_tree_iterator it;
	unsigned v;
	int ret;
	const size_t order3_order
		= sizeof rnd.root.node->key / sizeof *rnd.root.node->key + 1;
	printf("manual: order3 order %lu\n", order3_order);
	assert(order3_order == 3);

	/* Lookup between nodes. */
	if(!order3_tree_bulk_add(&between, 100)
		|| !order3_tree_bulk_add(&between, 200)
		|| !order3_tree_bulk_add(&between, 300)) goto catch;
	ret = order3_tree_bulk_finish(&between);
	assert(ret);
	tree_order3_graph_horiz(&between, "graph/between.gv");

	v = order3_tree_right_or(&between, 50, 0), assert(v == 100);
	v = order3_tree_right_or(&between, 150, 0), assert(v == 200);
	v = order3_tree_right_or(&between, 250, 0), assert(v == 300);
	v = order3_tree_right_or(&between, 300, 0), assert(v == 300);
	v = order3_tree_right_or(&between, 350, 0), assert(!v);

	it = order3_tree_right_next(&between, 50);
	printf("right next 50, %s:%u %sseen.\n",
		orcify(it._.ref.node), it._.ref.idx, it._.seen ? "" : "not ");
	ret = order3_tree_next(&it, &v), assert(ret && v == 100);
	printf("next %u.\n", v);
	it = order3_tree_right_next(&between, 50);
	ret = order3_tree_previous(&it, &v), assert(!ret);
	printf("previous dne.\n");

	it = order3_tree_right_next(&between, 150);
	printf("right next 150, %s:%u %sseen.\n",
		orcify(it._.ref.node), it._.ref.idx, it._.seen ? "" : "not ");
	ret = order3_tree_next(&it, &v), assert(ret && v == 200);
	printf("next %u.\n", v);
	it = order3_tree_right_next(&between, 150);
	ret = order3_tree_previous(&it, &v), assert(ret && v == 100);
	printf("previous %u.\n", v);

	it = order3_tree_right_next(&between, 200);
	printf("right next 200, %s:%u %sseen.\n",
		orcify(it._.ref.node), it._.ref.idx, it._.seen ? "" : "not ");
	ret = order3_tree_next(&it, &v), assert(ret && v == 200);
	printf("next %u.\n", v);
	it = order3_tree_right_next(&between, 200);
	ret = order3_tree_previous(&it, &v), assert(ret && v == 100);
	printf("previous %u.\n", v);

	it = order3_tree_right_next(&between, 250);
	printf("right next 250, %s:%u %sseen.\n",
		orcify(it._.ref.node), it._.ref.idx, it._.seen ? "" : "not ");
	ret = order3_tree_next(&it, &v), assert(ret && v == 300);
	printf("next %u.\n", v);
	it = order3_tree_right_next(&between, 250);
	ret = order3_tree_previous(&it, &v), assert(ret && v == 200);
	printf("previous %u.\n", v);

	it = order3_tree_right_next(&between, 350);
	printf("right next 350, %s:%u %sseen\n",
		orcify(it._.ref.node), it._.ref.idx, it._.seen ? "" : "not ");
	ret = order3_tree_next(&it, &v), assert(!ret);
	printf("next dne.\n");
	it = order3_tree_right_next(&between, 350);
	ret = order3_tree_previous(&it, &v), assert(ret && v == 300);
	printf("previous %u.\n", v);

	v = order3_tree_left_or(&between, 50, 0), assert(!v);
	v = order3_tree_left_or(&between, 150, 0), assert(v == 100);
	v = order3_tree_left_or(&between, 250, 0), assert(v == 200);
	v = order3_tree_left_or(&between, 300, 0), assert(v == 300);
	v = order3_tree_left_or(&between, 350, 0), assert(v == 300);

	it = order3_tree_left_previous(&between, 50);
	printf("left previous 50, %s:%u %sseen.\n",
		orcify(it._.ref.node), it._.ref.idx, it._.seen ? "" : "not ");
	ret = order3_tree_next(&it, &v), assert(ret && v == 100);
	printf("next %u.\n", v);
	it = order3_tree_left_previous(&between, 50);
	ret = order3_tree_previous(&it, &v), assert(!ret);
	printf("previous dne.\n");

	it = order3_tree_left_previous(&between, 150);
	printf("left previous 150, %s:%u %sseen.\n",
		orcify(it._.ref.node), it._.ref.idx, it._.seen ? "" : "not ");
	ret = order3_tree_next(&it, &v), assert(ret && v == 200);
	printf("next %u.\n", v);
	it = order3_tree_left_previous(&between, 150);
	ret = order3_tree_previous(&it, &v), assert(ret && v == 100);
	printf("previous %u.\n", v);

	it = order3_tree_left_previous(&between, 200);
	printf("left previous 200, %s:%u %sseen.\n",
		orcify(it._.ref.node), it._.ref.idx, it._.seen ? "" : "not ");
	ret = order3_tree_next(&it, &v), assert(ret && v == 300);
	printf("next %u.\n", v);
	it = order3_tree_left_previous(&between, 200);
	ret = order3_tree_previous(&it, &v), assert(ret && v == 200);
	printf("previous %u.\n", v);

	it = order3_tree_left_previous(&between, 250);
	printf("left previous 250, %s:%u %sseen.\n",
		orcify(it._.ref.node), it._.ref.idx, it._.seen ? "" : "not ");
	ret = order3_tree_next(&it, &v), assert(ret && v == 300);
	printf("next %u.\n", v);
	it = order3_tree_left_previous(&between, 250);
	ret = order3_tree_previous(&it, &v), assert(ret && v == 200);
	printf("previous %u.\n", v);

	it = order3_tree_left_previous(&between, 350);
	printf("left previous 350, %s:%u %sseen\n",
		orcify(it._.ref.node), it._.ref.idx, it._.seen ? "" : "not ");
	ret = order3_tree_next(&it, &v), assert(!ret);
	printf("next dne.\n");
	it = order3_tree_left_previous(&between, 350);
	ret = order3_tree_previous(&it, &v), assert(ret && v == 300);
	printf("previous %u.\n", v);

	/* Test greatest lower/least higher bound again. */
	order3_tree_clear(&between);
	order3_tree_bulk_add(&between, 10);
	order3_tree_bulk_add(&between, 20);
	order3_tree_bulk_add(&between, 30);
	order3_tree_bulk_add(&between, 40);
	order3_tree_bulk_add(&between, 50);
	order3_tree_bulk_add(&between, 60);
	order3_tree_bulk_add(&between, 70);
	order3_tree_bulk_add(&between, 80);
	order3_tree_bulk_finish(&between);
	tree_order3_graph_horiz(&between, "graph/left.gv");
	v = order3_tree_left_or(&between, 10, 0), assert(v == 10);
	v = order3_tree_left_or(&between, 80, 0), assert(v == 80);
	v = order3_tree_left_or(&between, 55, 0), assert(v == 50);
	v = order3_tree_left_or(&between, 85, 0), assert(v == 80);
	v = order3_tree_left_or(&between, 15, 0), assert(v == 10);
	v = order3_tree_left_or(&between, 5, 0), assert(!v);
	v = order3_tree_right_or(&between, 10, 0), assert(v == 10);
	v = order3_tree_right_or(&between, 80, 0), assert(v == 80);
	v = order3_tree_right_or(&between, 55, 0), assert(v == 60);
	v = order3_tree_right_or(&between, 85, 0), assert(!v);
	v = order3_tree_right_or(&between, 15, 0), assert(v == 20);
	v = order3_tree_right_or(&between, 5, 0), assert(v == 10);

	/* For the paper. */
	order3_tree_clear(&between);
	order3_tree_bulk_add(&between, 1);
	order3_tree_bulk_add(&between, 2);
	if(!order3_tree_clone(&copy, &between)) goto catch;
	tree_order3_graph_horiz(&copy, "graph/bulk2.gv");
	order3_tree_bulk_add(&between, 3);
	if(!order3_tree_clone(&copy, &between)) goto catch;
	tree_order3_graph_horiz(&copy, "graph/bulk3.gv");
	order3_tree_bulk_add(&copy, 4);
	tree_order3_graph_horiz(&copy, "graph/bulk4.gv");
	if(!order3_tree_clone(&copy, &between)) goto catch;
	order3_tree_bulk_finish(&copy);
	tree_order3_graph_horiz(&copy, "graph/bulk3-finish.gv");

	/* Random. */
	for(i = 0; i < size_rnd; i++) {
		unsigned x = rand() & 65535;
		printf("__%u) add random value %u__\n", (unsigned)i, x);
		switch(order3_tree_try(&rnd, x)) {
		case TREE_ERROR: goto catch;
		case TREE_PRESENT: printf("%u already in tree\n", x); break;
		case TREE_ABSENT: printf("%u added\n", x); break;
		}
		if(!(i & (i + 1)) || i == size_rnd - 1) {
			char fn[64];
			sprintf(fn, "graph/rnd-%u.gv", (unsigned)i);
			tree_order3_graph(&rnd, fn);
		}
	}

	{ /* Full add test, nodes, `\sum_{n=0}^{h} m^n = \frac{m^{h+1}-1}{m-1}`,
		 gives keys, `m^{h+1}-1`, three levels. */
		const size_t size = order3_order * order3_order * order3_order - 1;
		for(i = 0; i < size; i++) /* Even for odd spaces between them. */
			if(!order3_tree_bulk_add(&even, ((unsigned)i + 1) * 2)) assert(0);
		order3_tree_bulk_finish(&even); /* Does nothing, in this case. */
		tree_order3_graph_horiz(&even, "graph/even-1.gv");
		for(i = 0; i <= size; i++) {
			char fn[64];
			if(!order3_tree_clone(&even_clone, &even)) goto catch;
			if(i == 4) tree_order3_graph_horiz(&even_clone,
				"graph/even-clone-9-pre.gv");
			if(!order3_tree_try(&even_clone, (unsigned)i * 2 + 1)) goto catch;
			sprintf(fn, "graph/even-clone-%u.gv", (unsigned)i * 2 + 1);
			tree_order3_graph_horiz(&even_clone, fn);
		}
	}

	{ /* Rm tdd. */
		int in[3 * 3 * 3 - 1];
		const unsigned size = sizeof in / sizeof *in;
		unsigned n;
		memset(&in, 0, sizeof in);
		for(n = 0; n < size; n++) {
			if(!(order3_tree_bulk_add(&removal, n + 1))) goto catch;
			in[n] = 1;
		}
		order3_tree_bulk_finish(&even);
		/* Leaf, no redistributing needed. */
		tree_order3_graph_horiz(&removal, "graph/removal-0.gv");
		order3_tree_remove(&removal, 14), in[13] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-1.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		/* Leaf, free the node; distribute one level above. */
		order3_tree_remove(&removal, 13), in[12] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-2.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		/* Other side. */
		order3_tree_remove(&removal, 12), in[11] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-3.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		/* Merge three ways: lower. */
		if(!order3_tree_clone(&copy, &removal)) goto catch;
		tree_order3_graph_horiz(&copy, "graph/removal-4a0.gv");
		order3_tree_remove(&copy, 10), in[9] = 0;
		tree_order3_graph_horiz(&copy, "graph/removal-4a1.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&copy, n + 1) == in[n]);
		in[9] = 1;
		/* Middle. */
		if(!order3_tree_clone(&copy, &removal)) goto catch;
		tree_order3_graph_horiz(&copy, "graph/removal-4b0.gv");
		order3_tree_remove(&copy, 15), in[14] = 0;
		tree_order3_graph_horiz(&copy, "graph/removal-4b1.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&copy, n + 1) == in[n]);
		in[14] = 1;
		/* High. */
		if(!order3_tree_clone(&copy, &removal)) goto catch;
		tree_order3_graph_horiz(&copy, "graph/removal-4c0.gv");
		order3_tree_remove(&copy, 17), in[16] = 0;
		tree_order3_graph_horiz(&copy, "graph/removal-4c1.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&copy, n + 1) == in[n]);
		in[16] = 1;
		/* Already done. */
		order3_tree_remove(&removal, 15), in[14] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-5b.gv");
		order3_tree_remove(&removal, 16), in[15] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-5c.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		/* Multi-level. */
		order3_tree_remove(&removal, 10), in[9] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-6.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		/* In root. */
		order3_tree_remove(&removal, 18), in[17] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-7.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		order3_tree_remove(&removal, 17), in[16] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-8.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		order3_tree_remove(&removal, 19), in[18] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-9.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		order3_tree_remove(&removal, 20), in[19] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-10.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		order3_tree_remove(&removal, 21), in[20] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-11.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		order3_tree_remove(&removal, 22), in[21] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-12.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		order3_tree_remove(&removal, 11), in[10] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-13.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		order3_tree_remove(&removal, 9), in[8] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-14.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		order3_tree_remove(&removal, 8), in[7] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-15.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		order3_tree_remove(&removal, 7), in[6] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-16.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		order3_tree_remove(&removal, 6), in[5] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-17.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		order3_tree_remove(&removal, 5), in[4] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-18.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		order3_tree_remove(&removal, 4), in[3] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-19.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		/* There are some states that haven't been tested. */
		if(!order3_tree_clone(&removal, &copy)) goto catch;
		for(n = 0; n < size; n++) in[n] = 1;
		tree_order3_graph_horiz(&removal, "graph/removal-b-6.gv");
		in[11] = 0, in[12] = 0, in[13] = 0, in[16] = 0;
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		order3_tree_remove(&removal, 3), in[2] = 0;
		order3_tree_remove(&removal, 2), in[1] = 0;
		order3_tree_remove(&removal, 4), in[3] = 0;
		order3_tree_remove(&removal, 6), in[5] = 0;
		order3_tree_remove(&removal, 5), in[4] = 0;
		order3_tree_remove(&removal, 7), in[6] = 0;
		order3_tree_remove(&removal, 9), in[8] = 0;
		order3_tree_remove(&removal, 1), in[0] = 0;
		order3_tree_remove(&removal, 8), in[7] = 0;
		order3_tree_remove(&removal, 10), in[9] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-b-7.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		order3_tree_remove(&removal, 11), in[10] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-b-8.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		/* Yay. Now to try deleting the rest. */
		order3_tree_remove(&removal, 21), in[20] = 0;
		order3_tree_remove(&removal, 20), in[19] = 0;
		order3_tree_remove(&removal, 22), in[21] = 0;
		order3_tree_remove(&removal, 19), in[18] = 0;
		order3_tree_remove(&removal, 18), in[17] = 0;
		order3_tree_remove(&removal, 16), in[15] = 0;
		order3_tree_remove(&removal, 24), in[23] = 0;
		order3_tree_remove(&removal, 23), in[22] = 0;
		order3_tree_remove(&removal, 25), in[24] = 0;
		order3_tree_remove(&removal, 15), in[14] = 0;
		order3_tree_remove(&removal, 26), in[25] = 0;
		tree_order3_graph_horiz(&removal, "graph/removal-b-9.gv");
		for(n = 0; n < size; n++)
			assert(order3_tree_contains(&removal, n + 1) == in[n]);
		assert(removal.root.height == UINT_MAX && removal.root.node);
		for(n = 0; n < size; n++) assert(!in[n]);
		{
			int success = order3_tree_remove(&removal, 0);
			assert(!success);
		}
	}

	{ /* Consecutive. */
		const size_t size = 4 * order3_order - 1;
		char fn[64];
		for(i = 0; i < size; i++) {
			unsigned x = (unsigned)i + 1;
			/*printf("__%u) Going to add consecutive %u__\n", (unsigned)i, x);*/
			switch(order3_tree_try(&consecutive, x)) {
			case TREE_ERROR: goto catch;
			case TREE_PRESENT: /*printf("%u already in tree\n", x);*/ break;
			case TREE_ABSENT: /*printf("%u added\n", x);*/ break;
			}
			sprintf(fn, "graph/consecutive-%u.gv", (unsigned)i);
			tree_order3_graph(&consecutive, fn);
		}
		for(i = 0; i < size; i++) {
			unsigned x = (unsigned)i + 1;
			/*printf("__%u) Going to remove %u__\n", (unsigned)i, x);*/
			if(!order3_tree_remove(&consecutive, x)) assert(0);
			sprintf(fn, "graph/consecutive-rm-%u.gv", (unsigned)x);
			tree_order3_graph(&consecutive, fn);
		}
	}

	goto finally;
catch:
	perror("manual_unsigned");
	assert(0);
finally:
	order3_tree_(&between);
	order3_tree_(&rnd);
	order3_tree_(&removal);
	order3_tree_(&even), order3_tree_(&even_clone);
	order3_tree_(&consecutive);
	order3_tree_(&copy);
}

/** This tests the order 4 tree map. */
static void redblack(void) {
	struct redblack_tree tree = redblack_tree();
	unsigned i, n, *value, calc_size;
	struct { unsigned x; int in; } rnd[10];
	const unsigned rnd_size = sizeof rnd / sizeof *rnd;
	const size_t redblack_order
		= sizeof tree.root.node->key / sizeof *tree.root.node->key + 1;
	printf("Redblack: order %lu.\n", redblack_order);
	assert(redblack_order == 4);

	/* Random. */
	for(i = 0; i < rnd_size; i++) rnd[i].x = rand() & 65535, rnd[i].in = 0;
	{
		unsigned v = redblack_tree_get_or(&tree, 0, 42);
		assert(v == 42);
	}

	/* In tree. */
	for(n = 0, i = 0; i < rnd_size; i++) {
		unsigned v;
		switch(redblack_tree_try(&tree, rnd[i].x, &value)) {
		case TREE_ERROR: goto catch;
		case TREE_PRESENT: printf("%u already in tree\n", rnd[i].x); break;
		case TREE_ABSENT: *value = rnd[i].x; rnd[i].in = 1; n++; break;
		}
		if(!(i & (i + 1)) || i == rnd_size - 1) {
			char fn[64];
			sprintf(fn, "graph/rb-rnd-%u.gv", (unsigned)i);
			tree_redblack_graph(&tree, fn);
		}
		v = redblack_tree_get_or(&tree, rnd[i].x, 0);
		assert(v == *value && v == rnd[i].x);
	}
	printf("Redblack tree %u/%u: %s.\n",
		n, rnd_size, redblack_tree_to_string(&tree));
	calc_size = (unsigned)redblack_tree_count(&tree);
	assert(calc_size == n);
	while(n) {
		assert(tree.root.height != UINT_MAX);
		i = (unsigned)rand() / (RAND_MAX / n + 1);
		printf("Drew %u -> %u which is %sin.\n",
			i, rnd[i].x, rnd[i].in ? "" : "not ");
		if(redblack_tree_remove(&tree, rnd[i].x)) {
			assert(rnd[i].in);
			rnd[i].in = 0;
			if(/*!(n & (n + 1)) || n == rnd_size - 1*/1) {
				char fn[64];
				sprintf(fn, "graph/rb-rnd-rm-%u.gv", calc_size - n);
				tree_redblack_graph(&tree, fn);
			}
			n--;
			if(i != n) rnd[i] = rnd[n];
		} else {
			assert(!rnd[i].in);
		}
		{ /* Verify */
			size_t count = redblack_tree_count(&tree);
			assert(n == count);
		}
		for(i = 0; i <= n; i++) {
			unsigned v = redblack_tree_get_or(&tree, rnd[i].x, 0);
			assert(!rnd[i].in || rnd[i].x == v);
		}
	}
	assert(tree.root.height == UINT_MAX);
	goto finally;
catch:
	perror("redblack");
	assert(0);
finally:
	redblack_tree_(&tree);
}


/* Has distinguishable keys going to the same key value. This may be useful,
 for example, if one allocates keys. Also has default values. */
/** @implements <typedef:<PB>action_fn> */
static void loop_filler(unsigned *const x)
	{ *x = (unsigned)rand() / (RAND_MAX / 100000 + 1); }
static int loop_compare(const unsigned a, const unsigned b)
	{ return (a % 100) > (b % 100); }
static void loop_to_string(const unsigned x, char (*const z)[12])
	{ int_to_string(x, z); }
#define TREE_NAME loop
#define TREE_TEST
#define TREE_COMPARE
#define TREE_ORDER 5
#define TREE_DEFAULT 0
#define TREE_TO_STRING
#define TREE_EXPECT_TRAIT
#include "../src/tree.h"
#define TREE_TRAIT meaning
#define TREE_DEFAULT 42
#include "../src/tree.h"
/** Tests try and assign. */
static void loop(void) {
	struct loop_tree tree = loop_tree();
	enum tree_result status;
	unsigned ret, eject;
	status = loop_tree_try(&tree, 1), assert(status == TREE_ABSENT);
	status = loop_tree_try(&tree, 2), assert(status == TREE_ABSENT);
	status = loop_tree_try(&tree, 3), assert(status == TREE_ABSENT);
	status = loop_tree_try(&tree, 101), assert(status == TREE_PRESENT);
	ret = loop_tree_get_or(&tree, 1, 0), assert(ret == 1);
	tree_loop_graph_horiz(&tree, "graph/loop1.gv");
	status = loop_tree_assign(&tree, 101, &eject);
	assert(status == TREE_PRESENT && eject == 1);
	ret = loop_tree_get_or(&tree, 1, 0), assert(ret == 101); /* ~= 1 */
	tree_loop_graph_horiz(&tree, "graph/loop2.gv");
	ret = loop_tree_get_or(&tree, 3, 0), assert(ret == 3);
	ret = loop_tree_get_or(&tree, 4, 0), assert(ret == 0);
	ret = loop_tree_get(&tree, 3), assert(ret == 3);
	ret = loop_tree_get(&tree, 4), assert(ret == 0);
	ret = loop_tree_meaning_get(&tree, 3), assert(ret == 3);
	ret = loop_tree_meaning_get(&tree, 0), assert(ret == 42);

	ret = loop_tree_right_or(&tree, 3, 0), assert(ret == 3);
	ret = loop_tree_right_or(&tree, 4, 0), assert(ret == 0);
	ret = loop_tree_right(&tree, 0), assert(ret == 101);
	ret = loop_tree_right(&tree, 4), assert(ret == 0);
	ret = loop_tree_meaning_right(&tree, 0), assert(ret == 101);
	ret = loop_tree_meaning_right(&tree, 4), assert(ret == 42); /* No glb. */

	ret = loop_tree_left_or(&tree, 3, 0), assert(ret == 3);
	ret = loop_tree_left_or(&tree, 4, 0), assert(ret == 3);
	ret = loop_tree_left(&tree, 0), assert(ret == 0);
	ret = loop_tree_left(&tree, 4), assert(ret == 3);
	ret = loop_tree_meaning_left(&tree, 0), assert(ret == 42); /* No lub. */
	ret = loop_tree_meaning_left(&tree, 4), assert(ret == 3);

	loop_tree_(&tree);
	if(!loop_tree_try(&tree, 8)) { assert(0); return; }
	if(!loop_tree_try(&tree, 4)) { assert(0); return; }
	if(!loop_tree_try(&tree, 16)) { assert(0); return; }
	if(!loop_tree_try(&tree, 10)) { assert(0); return; }
	if(!loop_tree_try(&tree, 2)) { assert(0); return; }
	if(!loop_tree_try(&tree, 6)) { assert(0); return; }
	if(!loop_tree_try(&tree, 14)) { assert(0); return; }
	if(!loop_tree_try(&tree, 12)) { assert(0); return; }
	if(!loop_tree_try(&tree, 7)) { assert(0); return; }
	if(!loop_tree_try(&tree, 3)) { assert(0); return; }
	if(!loop_tree_try(&tree, 15)) { assert(0); return; }
	if(!loop_tree_try(&tree, 9)) { assert(0); return; }
	if(!loop_tree_try(&tree, 1)) { assert(0); return; }
	if(!loop_tree_try(&tree, 5)) { assert(0); return; }
	if(!loop_tree_try(&tree, 13)) { assert(0); return; }
	if(!loop_tree_try(&tree, 11)) { assert(0); return; }
	if(!loop_tree_try(&tree, 17)) { assert(0); return; }
	if(!loop_tree_try(&tree, 18)) { assert(0); return; }
	if(!loop_tree_try(&tree, 19)) { assert(0); return; }
	if(!loop_tree_try(&tree, 20)) { assert(0); return; }
	if(!loop_tree_try(&tree, 21)) { assert(0); return; }
	if(!loop_tree_try(&tree, 22)) { assert(0); return; }
	tree_loop_graph_horiz(&tree, "graph/loop3.gv"); /* For title. */
	loop_tree_(&tree);
}


struct typical_value { int a, b; };
static void typical_filler(unsigned *const k, struct typical_value **v)
	{ int_filler(k); *v = 0; }
static void typical_to_string(const unsigned k,
	/*const:[*/struct typical_value *const*const v, char (*const z)[12])
	{ sprintf(*z, "%u", k); (void)v; }
#define TREE_NAME typical
#define TREE_VALUE struct typical_value *
#define TREE_TO_STRING
#define TREE_TEST
#include "../src/tree.h"


int main(void) {
	unsigned seed = (unsigned)clock();
	srand(seed), rand(), printf("Seed %u.\n", seed);
	char_tree_test();
	int_tree_test();
	order3_tree_test();
	redblack_tree_test();
	pair_tree_test();
	star_tree_test();
	entry_tree_test();
	char_bounds();
	order3();
	redblack();
	loop_tree_test();
	loop();
	typical_tree_test();
	return EXIT_SUCCESS;
}

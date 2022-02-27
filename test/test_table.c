/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT). */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include "orcish.h"


/* Zodiac is a bounded set of `enum`. An X-macro allows printing. This is
 preferable to `TABLE_KEY const char *`. */
#define ZODIAC(X) X(Aries), X(Taurus), X(Gemini), X(Cancer), X(Leo), X(Virgo), \
	X(Libra), X(Scorpio), X(Sagittarius), X(Capricorn), X(Aquarius), X(Pisces),\
	X(ZodiacCount)
#define X(n) n
enum zodiac { ZODIAC(X) };
#undef X
#define X(n) #n
static const char *zodiac[] = { ZODIAC(X) };
#undef X
/** Sequential monotonic values make a pretty good hash.
 @implements <zodiac>hash_fn */
static unsigned hash_zodiac(const enum zodiac z) { return z; }
/** This is a discrete set with a simple homomorphism between keys and hash
 values, therefore it's simpler to work in hash space. This saves us from
 having to define <typedef:<PN>is_equal_fn> and saves the key from even being
 stored. @implements <zodiac>inverse_hash_fn */
static enum zodiac hash_inv_zodiac(const unsigned z) { return z; }
/** This is not necessary except for testing.
@implements <zodiac>to_string_fn */
static void zodiac_to_string(const enum zodiac z, char (*const a)[12])
	{ strcpy(*a, zodiac[z]); /* strlen z < 12 */ }
#define TABLE_NAME zodiac
#define TABLE_KEY enum zodiac
#define TABLE_HASH &hash_zodiac
/* Generally, if you can, inverse is less space and simpler than equals. */
#define TABLE_INVERSE &hash_inv_zodiac
/* There are less than 256/2 keys, so a byte would suffice, but speed-wise, we
 expect type coercion between different sizes to be slower. */
#define TABLE_UINT unsigned /*char*/
#define TABLE_TEST /* Testing requires to string. */
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
#define TABLE_TO_STRING &zodiac_to_string
#include "../src/table.h"
/* For testing; there is no extra memory required to generate random `enum`.
 @implements <zodiac>fill_fn */
static int fill_zodiac(void *const zero, enum zodiac *const z) {
	(void)zero, assert(!zero);
	*z = (enum zodiac)(rand() / (RAND_MAX / ZodiacCount + 1));
	return 1;
}


/* String set. */

/** One must supply the hash: djb2 <http://www.cse.yorku.ca/~oz/hash.html> is
 a simple one (fast) that is mostly `size_t`-length agnostic.
 @implements <string>hash_fn */
static size_t djb2_hash(const char *s) {
	const unsigned char *str = (const unsigned char *)s;
	size_t hash = 5381, c;
	while(c = *str++) hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	return hash;
}
/** @implements <string>is_equal_fn */
static int string_is_equal(const char *const a, const char *const b)
	{ return !strcmp(a, b); }
/** @implements <string>to_string_fn */
static void string_to_string(const char *const s, char (*const a)[12])
	{ strncpy(*a, s, sizeof(*a) - 1), (*a)[sizeof(*a) - 1] = '\0'; }
#define TABLE_NAME string
#define TABLE_KEY char * /* Parameter of <fn:djb2_hash> (without `const`.) */
#define TABLE_HASH &djb2_hash /* Default returns `size_t`. */
#define TABLE_IS_EQUAL &string_is_equal
#define TABLE_TEST /* Testing requires to string. */
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
/* Enables `string_get`; this is just a convenient function that saves
 `string_get_or(..., 0)`. The reason we have to do this is the key is opaque;
 does *not* assume 0 is a special meaning. (We can get away with this because
 `next` stores the sentinel values.) One has to tell it that we want null to
 indicate not found. */
#define TABLE_DEFAULT 0
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
#define TABLE_TO_STRING &string_to_string
#include "../src/table.h"
/* A pool is convenient for testing because it allows deletion at random. */
struct str16 { char str[16]; };
#define POOL_NAME str16
#define POOL_TYPE struct str16
#include "pool.h"
/** For testing: `s16s` is a pool of `str16`. */
static char *str16_from_pool(struct str16_pool *const s16s) {
	struct str16 *s16 = str16_pool_new(s16s);
	if(!s16) return 0;
	orcish(s16->str, sizeof s16->str);
	return s16->str;
}
/** @implements <string>test_new_fn */
static int str16_from_void(void *const s16s, char **const string) {
	return !!(*string = str16_from_pool(s16s));
}


/* Integer set with inverse hash to avoid storing the hash at all. */
#if UINT_MAX >= 4294967295 /* >= 32-bits */
/** <https://nullprogram.com/blog/2018/07/31/>
 <https://github.com/skeeto/hash-prospector>. It was meant to work on
 `uint32_t`, but that's not guaranteed to exist in the C90 standard.
 @implements <int>hash_fn */
static unsigned lowbias32(unsigned x) {
	x ^= x >> 16;
	x *= 0x7feb352dU;
	x ^= x >> 15;
	x *= 0x846ca68bU;
	x ^= x >> 16;
	return x;
}
/** @implements <int>inverse_hash_fn */
static unsigned lowbias32_r(unsigned x) {
	x ^= x >> 16;
	x *= 0x43021123U;
	x ^= x >> 15 ^ x >> 30;
	x *= 0x1d69e2a5U;
	x ^= x >> 16;
	return x;
}
#else /* < 32 bits */
/** Uniform values don't really need a hash, and I'm lazy.
 @implements <int>hash_fn */
static unsigned lowbias32(unsigned x) { return x; }
/** @implements <int>inverse_hash_fn */
static unsigned lowbias32_r(unsigned x) { return x; }
#endif /* < 32 bits */
/** @implements <int>to_string_fn */
static void uint_to_string(const unsigned x, char (*const a)[12])
	{ sprintf(*a, "%u", x); }
#define TABLE_NAME uint
#define TABLE_KEY unsigned /* Parameter of <fn:lowbias32>. */
#define TABLE_UINT unsigned /* Return key of <fn:lowbias32>. */
#define TABLE_HASH &lowbias32
#define TABLE_INVERSE &lowbias32_r /* Invertible means no key storage at all. */
#define TABLE_TEST
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
#define TABLE_TO_STRING &uint_to_string
#include "../src/table.h"
/** @implements <int>test_new_fn */
static int uint_from_void(void *const zero, unsigned *const u) {
	(void)zero, assert(!zero && RAND_MAX <= 99999999999l); /* For printing. */
	*u = (unsigned)rand();
	return 1;
}


/* Check to see that the prototypes are correct by making a signed integer.
 Also testing `TABLE_DEFAULT`. */
/** @implements <int>hash_fn */
static unsigned int_hash(int d)
	{ return lowbias32((unsigned)d - (unsigned)INT_MIN); }
/** @implements <int>inverse_hash_fn */
static int int_inv_hash(unsigned u)
	{ return (int)(lowbias32_r(u) + (unsigned)INT_MIN); }
/** @implements <int>to_string_fn */
static void int_to_string(const int d, char (*const a)[12])
	{ sprintf(*a, "%d", d); }
#define TABLE_NAME int
#define TABLE_KEY int
#define TABLE_UINT unsigned
#define TABLE_HASH &int_hash
#define TABLE_INVERSE &int_inv_hash
#define TABLE_TEST
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
#define TABLE_DEFAULT 0
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
#define TABLE_DEFAULT 42
#define TABLE_DEFAULT_NAME 42
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
#define TABLE_TO_STRING &int_to_string
#include "../src/table.h"
/** @implements <int>test_new_fn */
static int int_from_void(void *const zero, int *const s) {
	(void)zero, assert(!zero && RAND_MAX <= 9999999999l); /* With '-'. */
	*s = rand() - RAND_MAX / 2;
	return 1;
}


/** Vector hash implemented as a pointer. This is kind of a silly example
 because it's easily homomorphic to a set of integers, but pretend we had a big
 problem space, (such an example would be difficult to describe succinctly.) */
struct vec4 { char a[2], unused[2]; int n[2]; };
/** @implements <vec4>hash_fn */
static unsigned vec4_hash(const struct vec4 *const v4) {
	return (unsigned)(1 * v4->n[0] + 10 * v4->n[1]
		+ 100 * (v4->a[0] - 'A') + 26000 * (v4->a[1] - 'a'));
}
/** @implements <vec4>is_equal_fn */
static int vec4_is_equal(const struct vec4 *a, const struct vec4 *const b) {
	return a->a[0] == b->a[0] && a->a[1] == b->a[1]
		&& a->n[0] == b->n[0] && a->n[1] == b->n[1];
}
/** @implements <vec4>to_string_fn */
static void vec4_to_string(const struct vec4 *const v4, char (*const a)[12])
	{ sprintf(*a, "%c%d%c%d",
	v4->a[0], v4->n[0] % 100, v4->a[1], v4->n[1] % 100); }
#define TABLE_NAME vec4
#define TABLE_KEY struct vec4 *
#define TABLE_HASH &vec4_hash
#define TABLE_IS_EQUAL &vec4_is_equal
/* Because of alignment, doesn't buy anything in terms of space savings. */
#define TABLE_UINT unsigned
#define TABLE_TEST
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
#define TABLE_TO_STRING &vec4_to_string
#include "../src/table.h"
#define POOL_NAME vec4
#define POOL_TYPE struct vec4
#include "pool.h"
/** For testing: `s4s` is a pool of vectors. */
static struct vec4 *vec4_from_pool(struct vec4_pool *const v4s) {
	struct vec4 *v4 = vec4_pool_new(v4s);
	if(!v4) return 0;
	v4->a[0] = 'A' + (char)(rand() / (RAND_MAX / 26 + 1));
	v4->a[1] = 'a' + (char)(rand() / (RAND_MAX / 26 + 1));
	v4->n[0] = rand() / (RAND_MAX / 9 + 1);
	v4->n[1] = rand() / (RAND_MAX / 9 + 1);
	return v4;
}
/** @implements <vec4>test_new_fn */
static int vec4_from_void(void *const vec4s, struct vec4 **const v)
	{ return assert(vec4s), !!(*v = vec4_from_pool(vec4s)); }


/** Too lazy to do separate tests. */
static void test_default(void) {
	struct int_table t = TABLE_IDLE;
	int one, two, def;
	printf("Testing get defaults.\n");
	int_table_try(&t, 1);
	int_table_try(&t, 2);
	printf("Table %s.\n", int_table_to_string(&t));
	one = int_table_get_or(&t, 1, 7);
	two = int_table_get_or(&t, 2, 7);
	def = int_table_get_or(&t, 3, 7);
	printf("get or default(7): 1:%u, 2:%u, 3:%u\n", one, two, def);
	assert(one == 1 && two == 2 && def == 7);
	one = int_table_get(&t, 1);
	two = int_table_get(&t, 2);
	def = int_table_get(&t, 3);
	printf("get or 0: 1:%u, 2:%u, 3:%u\n", one, two, def);
	assert(one == 1 && two == 2 && def == 0);
	one = int_table_42_get(&t, 1);
	two = int_table_42_get(&t, 2);
	def = int_table_42_get(&t, 3);
	printf("get or 42: 1:%u, 2:%u, 3:%u\n", one, two, def);
	assert(one == 1 && two == 2 && def == 42);
	int_table_(&t);
	printf("\n");
}


/** Test iteration removal. */
static void test_it(void) {
	struct int_table t = TABLE_IDLE;
	struct int_table_iterator it;
	const int ko = 1000, ko2 = ko * 2;
	int n, i;
	printf("Testing iteration.\n");
	for(n = 0; n < ko2; n++) if(!int_table_try(&t, n)) goto catch;
	table_int_graph(&t, "graph/it0.gv");
	assert(t.size == ko2);
	printf("Remove: ");
	for(int_table_begin(&it, &t); int_table_has_next(&it); ) {
		int_table_next(&it, &i);
		if(i & 1) continue; /* Odd ones left. */
		int_table_iterator_remove(&it);
		assert(int_table_iterator_remove(&it) == 0);
	}
	printf("done.\n");
	table_int_graph(&t, "graph/it1.gv");
	assert(t.size == ko);
	for(int_table_begin(&it, &t); int_table_has_next(&it); )
		int_table_next(&it, &i), assert(i & 1);
	goto finally;
catch:
	perror("it"), assert(0);
finally:
	int_table_(&t);
	printf("\n");
}


/** This is stored in the map value of `<boat>table`. */
struct boat_record { int best_time, points; };
#define TABLE_NAME boat
#define TABLE_KEY int
#define TABLE_UINT unsigned
#define TABLE_VALUE struct boat_record
#define TABLE_HASH &int_hash
#define TABLE_INVERSE &int_inv_hash
#include "../src/table.h"
/** <https://stackoverflow.com/q/59091226/2472827>. */
static void boat_club(void) {
	struct boat_table boats = TABLE_IDLE;
	size_t i;
	int success = 0;
	printf("Boat club races:\n");
	for(i = 0; i < 1000; i++) {
		/* Pigeon-hole principle ensures collisions. */
		const int id = rand() / (RAND_MAX / /*89<-spam*/29 + 1) + 10,
			time = rand() / (RAND_MAX / 100 + 1) + 50,
			points = 151 - time;
		struct boat_record *record;
		/*printf("Boat #%d had a time of %d, giving them %d points.\n",
			id, time, points);*/
		switch(boat_table_compute(&boats, id, &record)) {
		case TABLE_UNIQUE:
			record->best_time = time; record->points = points; break;
		case TABLE_YIELD:
			if(time < record->best_time) {
				printf("#%d best time %d -> %d.\n", id, record->best_time, time);
				record->best_time = time;
			}
			/*printf("#%d points %d -> %d.\n",
				id, record->points, record->points + points);*/
			record->points += points;
			break;
		case TABLE_ERROR: case TABLE_REPLACE: goto catch;
		}
	}
	{
		struct boat_table_entry e;
		struct boat_table_iterator it;
		printf("Final score:\n"
			"id\tbest\tpoints\n");
		boat_table_begin(&it, &boats);
		while(boat_table_next(&it, &e))
			printf("%d\t%d\t%d\n", e.key, e.value.best_time, e.value.points);
	}
	{ success = 1; goto finally; }
catch:
	perror("boats"), assert(0);
finally:
	boat_table_(&boats);
	printf("\n");
}


/* <https://en.wikipedia.org/wiki/List_of_brightest_stars> and light-years from
 Sol. As a real example, this is silly; it would be much better suited to
 `gperf` because the data is known beforehand. */
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
	/* Messes with graph?
	X(2.30 (2.29–2.34var), 550), X(2.30 (2.29–2.31var), 380),*/ \
	X(Dschubba, 400), X(Larawag, 65), /*X(2.35 (2.30–2.41var), 310),*/ \
	X(Merak, 79), X(Ankaa, 77), X(Girtab, 460), X(Enif, 670), X(Scheat, 200), \
	X(Sabik, 88), X(Phecda, 84), X(Aludra, 2000), X(Markeb, 540), \
	X(Navi, 610), X(Markab, 140), X(Aljanah, 72), X(Acrab, 404)
#define X(n, m) #n
static /*const*/ char *star_names[] = { STARS };
#undef X
static const size_t stars_size = sizeof star_names / sizeof *star_names;
#define X(n, m) m
static const double star_distances[] = { STARS };
#undef X
static unsigned char djb2_restrict(const char *const s)
	{ return (unsigned char)djb2_hash(s); }
#define TABLE_NAME star
#define TABLE_KEY char *
#define TABLE_VALUE double
#define TABLE_UINT unsigned char
#define TABLE_HASH &djb2_restrict
#define TABLE_IS_EQUAL &string_is_equal
#define TABLE_TEST
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
#define TABLE_DEFAULT 0
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
#define TABLE_TO_STRING &string_to_string
#include "../src/table.h"
/* @implements <zodiac>fill_fn */
static int fill_star(void *const zero, struct star_table_entry *const star) {
	size_t r = (size_t)rand() / (RAND_MAX / stars_size + 1);
	(void)zero, assert(!zero);
	star->key = star_names[r];
	star->value = star_distances[r];
	return 1;
}


/* Linked dictionary: linked-list with a comparison order indexed by a map.
 This is just the thing we said in `TABLE_VALUE` to avoid, and probably is the
 worst-case in terms of design of the table. It's associated, so we don't
 really have a nice choice; were we to aggregate it, then it would be a fixed
 size or face alignment issues. */
struct dict_listlink;
static int dict_compare(const struct dict_listlink *,
	const struct dict_listlink *);
#define LIST_NAME dict
#define LIST_EXPECT_TRAIT
#include "list.h"
#define LIST_COMPARE &dict_compare
#include "list.h"
/** Associated with the word and a link to other words. */
struct dict { struct dict_listlink link; char *word; };
/** `container_of` `link`; `offsetof`, in this case, is zero; we could have
 used a cast. */
static const struct dict *link_upcast(const struct dict_listlink *const link)
	{ return (const struct dict *)(const void *)
	((const char *)link - offsetof(struct dict, link)); }
/** Compare `a` and `b`. */
static int dict_compare(const struct dict_listlink *const a,
	const struct dict_listlink *const b)
	{ return strcmp(link_upcast(a)->word, link_upcast(b)->word); }
/* We take them on-line as they come, so we need a stable pool. */
#define POOL_NAME dict
#define POOL_TYPE struct dict
#include "pool.h"
/* Words which we read all at once from a file, modify them in-place. */
#define ARRAY_NAME char
#define ARRAY_TYPE char
#include "array.h"
/** Append a text file, `fn`, to `c`, and add a '\0'.
 @return Success. A partial read is failure. @throws[fopen, fread, malloc]
 @throws[EISEQ] The text file has embedded nulls.
 @throws[ERANGE] If the standard library does not follow POSIX. */
static int append_file(struct char_array *c, const char *const fn) {
	FILE *fp = 0;
	const size_t granularity = 1024;
	size_t nread;
	char *cursor;
	int success = 0;
	assert(c && fn);
	if(!(fp = fopen(fn, "r"))) goto catch;
	/* Read entire file in chunks. */
	do if(!(cursor = char_array_buffer(c, granularity))
		|| (nread = fread(cursor, 1, granularity, fp), ferror(fp))
		|| !char_array_append(c, nread)) goto catch;
	while(nread == granularity);
	/* File to `C` string. */
	if(!(cursor = char_array_new(c))) goto catch;
	*cursor = '\0';
	/* Binary files with embedded '\0' are not allowed. */
	if(strchr(c->data, '\0') != cursor) { errno = EILSEQ; goto catch; }
	{ success = 1; goto finally; }
catch:
	if(!errno) errno = EILSEQ; /* Will never be true on POSIX. */
finally:
	if(fp) fclose(fp);
	return success;
}
/* A string set with a pointer to dict map. It duplicates data from `key` and
 `value->word`, but table is complicated enough as it is. It has to be a
 pointer because a table is not stable, (not guaranteed stability even looking
 at it.) */
#define TABLE_NAME dict
#define TABLE_KEY char *
#define TABLE_VALUE struct dict *
#define TABLE_HASH &djb2_hash
#define TABLE_IS_EQUAL &string_is_equal
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
#define TABLE_DEFAULT 0
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
#define TABLE_TO_STRING &string_to_string
#include "../src/table.h"
/* `re2c` is very useful for file input. */
#include "lex.h"
/** Manual test. */
static void linked_dict(void) {
	struct char_array english = ARRAY_IDLE;
	const char *const inglesi_fn = "test/Tutte_le_parole_inglesi.txt";
	struct dict_table words = TABLE_IDLE;
	struct dict_pool backing = POOL_IDLE;
	struct dict_list order;
	struct lex_state state = { 0, 0, 0 }; /* Defined in <lex.h>. */
	printf("Testing linked-dictionary.\n");
	if(!append_file(&english, inglesi_fn)) goto catch;
	dict_list_clear(&order);
	state.cursor = english.data, state.line = 1;
	while(lex_dict(&state)) { /* Cut off the string in the `char` array. */
		struct dict **ptr, *d;
		switch(dict_table_compute(&words, state.word, &ptr)) {
		case TABLE_ERROR: goto catch;
		case TABLE_REPLACE: assert(0); break; /* This can never happen. */
		case TABLE_YIELD: printf("Next line %lu: duplicate \"%s\"; ignoring.\n",
			(unsigned long)state.line, state.word); continue;
		case TABLE_UNIQUE:
			if(!(d = *ptr = dict_pool_new(&backing))) goto catch;
			d->word = state.word;
			dict_list_push(&order, &d->link);
		}
	}
	/* Natural merge sort, `O(n)` when it's (almost?) in order. (They are
	 presumably already sorted.) */
	dict_list_sort(&order);
	printf("Dictionary: %s.\n", dict_table_to_string(&words));
	/* Print all.
	for(i = dict_list_head(&order); i; i = dict_list_next(i))
		printf("%s\n", link_upcast(i)->word); */
	{
		char *rando[] = { "HIPPOPOTAMI", "EMU", "ZYGON", "LIZARDS", "APE" },
			*befs[] = { "HIPPOLOGY", "EMS", "ZYGOMORPHY", "LIZARD", "APAYS" },
			*afts[] = { "HIPPOPOTAMUS","EMULATE","ZYGOPHYTE","LLAMA","APEAK" },
			**r, **r_end;
		(void)befs, (void)afts;
		for(r = rando, r_end = r + sizeof rando / sizeof *rando;
			r < r_end; r++) {
			struct dict *found;
			char *look = *r, *before, *after;
			if(!(found = dict_table_get(&words, look))) { printf(
				"That's weird; \"%s\" wasn't found in the dictionary.\n", look);
				assert(0); continue; }
			before = found->link.prev
				? link_upcast(found->link.prev)->word : "start";
			after = found->link.next
				? link_upcast(found->link.next)->word : "end";
			printf("Found \"%s\" between …%s, %s, %s…\n",
				look, before, found->word, after);
			assert(!strcmp(look, found->word)
				&& !strcmp(befs[r - rando], before)
				&& !strcmp(afts[r - rando], after));
		}
	}
	goto finally;
catch:
	perror("dict"), assert(0);
finally:
	dict_table_(&words);
	dict_pool_(&backing);
	char_array_(&english);
	printf("\n");
}


/* Int set that is a subclass of a larger parent using a pointer. */
struct year;
typedef void (*year_to_string_fn)(const struct year *, char (*)[12]);
struct year_vt { year_to_string_fn to_string; };
struct year { int year, unused; struct year_vt vt; };
/** @implements <year>hash_fn */
static unsigned year_hash(const int *const year) { return int_hash(*year); }
/** @implements <year>is_equal_fn */
static int year_is_equal(const int *const a, const int *const b)
	{ return *a == *b; }
/** `container_of` integer `year`, which is `struct year`. */
static const struct year *year_upcast(const int *const year)
	{ return (const struct year *)(const void *)
	((const char *)year - offsetof(struct year, year)); }
/** @implements <thing>to_string_fn */
static void year_to_string(const int *const year, char (*const a)[12])
	{ year_upcast(year)->vt.to_string(year_upcast(year), a); }
#define TABLE_NAME year
#define TABLE_KEY int *
#define TABLE_UINT unsigned
#define TABLE_HASH &year_hash
/* Can not use `TABLE_INVERSE` because it's not a bijection. */
#define TABLE_IS_EQUAL &year_is_equal
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
#define TABLE_TO_STRING &year_to_string
#include "../src/table.h"
#define COLOUR /* Max 11 letters. */ \
	X(White), X(Silver), X(Gray), X(Black), X(Red), X(Maroon), X(Bisque), \
	X(Wheat), X(Tan), X(Sienna), X(Brown), X(Yellow), X(Khaki), X(Gold), \
	X(Olive), X(Lime), X(Green), X(Aqua), X(Cyan), X(Teal), X(Salmon), \
	X(Orange), X(Powder), X(Sky), X(Steel), X(Royal), X(Blue), X(Navy), \
	X(Fuchsia), X(Pink), X(Purple), X(MaxColour)
#define X(n) n
struct colour { struct year year; enum col { COLOUR } colour, unused; };
#undef X
#define X(n) #n
static const char *colours[] = { COLOUR };
#undef X
struct snake { struct year year; unsigned chill, unused; };
struct letter { struct year year; char letter, unused[7]; };
/* (It's just a coincidence that the types are all the same size. They could
 not be.) */
/** @implements year_to_string_fn */
static void colour_to_string(const struct year *const year,
	char (*const z)[12])
	{ sprintf(*z, "%d:%.6s",
	year->year, colours[((const struct colour *)year)->colour]); }
static void snake_to_string(const struct year *const year,
	char (*const z)[12])
	{ sprintf(*z, "%d:Snake%.1u",
	year->year, ((const struct snake *)year)->chill); }
static void letter_to_string(const struct year *const year,
	char (*const z)[12])
	{ sprintf(*z, "%d:'%c'",
	year->year, ((const struct letter *)year)->letter); }
static const struct year_vt colour_vt = { &colour_to_string },
	snake_vt = { &snake_to_string }, letter_vt = { &letter_to_string };
static void fill_year(struct year *const year)
	{ year->year = rand() / (RAND_MAX / 3000 + 1) - 500; }
static void fill_colour(struct colour *const colour) {
	fill_year(&colour->year);
	colour->year.vt = colour_vt;
	colour->colour = (enum col)(rand() / (RAND_MAX / MaxColour + 1));
}
static void fill_snake(struct snake *const snake) {
	fill_year(&snake->year);
	snake->year.vt = snake_vt;
	snake->chill = (unsigned)(rand() / (RAND_MAX / 10 + 1));
}
static void fill_letter(struct letter *const letter) {
	fill_year(&letter->year);
	letter->year.vt = letter_vt;
	letter->letter = rand() / (RAND_MAX / 26 + 1) + 'A';
}
#define POOL_NAME colour
#define POOL_TYPE struct colour
#include "pool.h"
#define POOL_NAME snake
#define POOL_TYPE struct snake
#include "pool.h"
#define POOL_NAME letter
#define POOL_TYPE struct letter
#include "pool.h"
/** Example of a set with a pointer key. */
static void year_of(void) {
	struct year_table year = TABLE_IDLE;
	struct colour_pool colour_pool = POOL_IDLE;
	struct snake_pool snake_pool = POOL_IDLE;
	struct letter_pool letter_pool = POOL_IDLE;
	size_t i;
	printf("Testing table pointer set key.\n");
	for(i = 0; i < 500; i++) {
		struct year *y = 0;
		int *eject;
		switch(rand() / (RAND_MAX / 3 + 1)) {
		case 0: { struct colour *c;
			if(!(c = colour_pool_new(&colour_pool))) goto catch;
			fill_colour(c), y = &c->year; } break;
		case 1: { struct snake *s;
			if(!(s = snake_pool_new(&snake_pool))) goto catch;
			fill_snake(s), y = &s->year; } break;
		case 2: { struct letter *a;
			if(!(a = letter_pool_new(&letter_pool))) goto catch;
			fill_letter(a), y = &a->year; } break;
		}
		switch(year_table_replace(&year, &y->year, &eject)) {
		case TABLE_ERROR: case TABLE_YIELD: goto catch;
		case TABLE_REPLACE: { char e[12], z[12];
			year_to_string(eject, &e);
			year_to_string(&y->year, &z);
			printf("Replaced %s with %s.\n", e, z);
			/* We could have a `delete` in the virtual table which frees
			 unneeded memory. This is just an example, we'll allow it all to be
			 deleted later; it just sits there unused, now. */ } break;
		case TABLE_UNIQUE: break;
		}
	}
	printf("Year of %s.\n", year_table_to_string(&year));
	goto finally;
catch:
	perror("year"), assert(0);
finally:
	year_table_(&year);
	colour_pool_(&colour_pool); /* This is where it's deleted. */
	snake_pool_(&snake_pool);
	letter_pool_(&letter_pool);
	printf("\n");
}


/* A histogram of lengths' defined as a map with the pointers to the keys
 recorded as a linked-list. */
struct nato_list { const char *alpha; struct nato_list *next; };
struct nato_value { size_t occurrences; struct nato_list *head; };
/** Symmetric bijection. @implements <nato>hash_fn, <nato>inverse_hash_fn */
static size_t nato_hash(const size_t n) { return n; }
#define TABLE_NAME nato
#define TABLE_KEY size_t
#define TABLE_VALUE struct nato_value
#define TABLE_INVERSE &nato_hash
#define TABLE_HASH &nato_hash
#include "../src/table.h" /* (Manual testing.) */
/** Counts code-points except non-alnums of `s`, being careful.
 (You are working in UTF-8, right?) <https://stackoverflow.com/a/32936928> */
static size_t utf_alnum_count(const char *s) {
	size_t c = 0;
	while(*s != '\0')
		c += *s & 0x80 ? (*s++ & 0xC0) != 0x80 : !!isalnum(*s), s++;
	return c;
}
static void nato(void) {
	const char *const alphabet[] = { "Alpha", "Bravo", "Charlie", "Delta",
		"Echo", "Foxtrot", "Golf", "Hotel", "India", "Juliet", "Kilo", "Lima",
		"Mike", "November", "Oscar", "Papa", "Québec", "Romeo", "Sierra",
		"Tango", "Uniform", "Victor", "Whisky", "X-ray", "Yankee", "Zulu" };
	struct nato_list list[sizeof alphabet / sizeof *alphabet];
	struct nato_table nato = TABLE_IDLE;
	struct nato_table_iterator it;
	struct nato_table_entry entry;
	size_t i;
	for(i = 0; i < sizeof alphabet / sizeof *alphabet; i++) {
		size_t length = utf_alnum_count(alphabet[i]);
		struct nato_value *value = 0;
		struct nato_list *item = list + i;
		switch(nato_table_compute(&nato, length, &value)) {
		case TABLE_ERROR: goto catch;
			/* PutIfAbsent */
		case TABLE_UNIQUE: value->occurrences = 1, value->head = 0; break;
			/* PutIfPresent */
		case TABLE_YIELD: value->occurrences++; break;
		case TABLE_REPLACE: assert(0); /* Impossible, <fn:<N>table_compute>. */
		}
		item->alpha = alphabet[i];
		item->next = value->head, value->head = item; /* Linked-list append. */
	}
	printf("NATO phonetic alphabet letter count histogram\n"
		"length\tcount\twords\n");
	for(nato_table_begin(&it, &nato); nato_table_next(&it, &entry); ) {
		struct nato_list *const head = entry.value.head, *w = head;
		printf("%lu\t%lu\t{", (unsigned long)entry.key,
			(unsigned long)entry.value.occurrences);
		do printf("%s%s", head == w ? "" : ",", w->alpha); while(w = w->next);
		printf("}\n");
	}
	{ /* Check. */
		struct nato_table_entry e;
		int success;
		success = nato_table_query(&nato, 0, &e);
		assert(!success);
		success = nato_table_query(&nato, 3, &e);
		assert(!success);
		success = nato_table_query(&nato, 4, &e);
		assert(success && e.value.occurrences == 8);
		success = nato_table_query(&nato, 5, &e);
		assert(success && e.value.occurrences == 8);
		success = nato_table_query(&nato, 6, &e);
		assert(success && e.value.occurrences == 6);
		success = nato_table_query(&nato, 7, &e);
		assert(success && e.value.occurrences == 3);
		success = nato_table_query(&nato, 8, &e);
		assert(success && e.value.occurrences == 1);
	}
	goto finally;
catch:
	perror("nato"), assert(0);
finally:
	nato_table_(&nato);
	printf("\n");
}


/** Contrived example. */
static void stars(void) {
	struct star_table stars = TABLE_IDLE;
	const size_t s_array[] = { 0, 1, 8, 9, 11, 16, 17, 20, 22, 25, 49 };
	size_t i;
	for(i = 0; i < sizeof s_array / sizeof *s_array; i++) {
		struct star_table_entry e;
		size_t s = s_array[i];
		e.key = star_names[s], e.value = star_distances[s];
		printf("%lu: %s -> %f\n", (unsigned long)s, e.key, e.value);
		if(star_table_try(&stars, e) != TABLE_UNIQUE)
			goto catch;
	}
	printf("%s.\n", star_table_to_string(&stars));
	table_star_graph(&stars, "web/table-precursor/star-raw.gv");
	goto finally;
catch:
	assert(0);
finally:
	star_table_(&stars);
	printf("\n");
}


#if 0 /* <!-- timing */

/* Set up a closed hash table for comparison. With optimizations, I get that
 the run-time is very close to the same. Performance-wise, the simplicity of
 the closed hash is the winner. However, open tables are a big improvement in
 usability over the inscrutable closed set. (Like why is there a linked-list?
 No one has time to read or understand the documentation.)

 This relies on the closed set branch being in a certain directory outside the
 project, which it isn't in general, (_ie_, one would have to make changes.) I
 expect strings are the basis of most use cases. */

/** This was before we solved pointers-pointers. */
static void pstring_to_string(const char *const*const ps,
	char (*const a)[12])
	{ strncpy(*a, *ps, sizeof(*a) - 1), (*a)[sizeof(*a) - 1] = '\0'; }
/** Bogus `a`. */
static void closed_fill(const char **const a) { assert(a && !a); }
#define SET_NAME closed
#define SET_TYPE char *
#define SET_UINT size_t
#define SET_HASH &djb2_hash
#define SET_IS_EQUAL &string_is_equal
#define SET_TEST &closed_fill
#define SET_EXPECT_TRAIT
#include "../../../set/src/set.h" /* Set `#if 0` above; it is only for me. */
#define SET_TO_STRING &pstring_to_string
#include "../../../set/src/set.h"
#define ARRAY_NAME closed
#define ARRAY_TYPE struct closed_setlink
#include "array.h"
/** Pair of elements stuck together for passing to tests. */
struct backing { struct str16_pool str16s; struct closed_array closed; };

#include <time.h>
/** Returns a time difference in microseconds from `then`. */
static double diff_us(clock_t then)
	{ return 1000000.0 / CLOCKS_PER_SEC * (clock() - then); }
/** On-line numerically stable first-order statistics, <Welford, 1962, Note>. */
struct measure { size_t count; double mean, ssdm; };
static void m_reset(struct measure *const m)
	{ m->count = 0, m->mean = m->ssdm = 0; }
static void m_add(struct measure *const m, const double replica) {
	const size_t n = ++m->count;
	const double delta = replica - m->mean;
	m->mean += delta / n;
	m->ssdm += delta * (replica - m->mean);
}
static double m_mean(const struct measure *const m)
	{ return m->count ? m->mean : (double)NAN; }
static double m_sample_variance(const struct measure *const m)
	{ return m->count > 1 ? m->ssdm / (m->count - 1) : (double)NAN; }
static double m_stddev(const struct measure *const m)
	{ return sqrt(m_sample_variance(m)); }

#define EXPS(X) X(Closed), X(Open)

static void timing_comparison(void) {
	FILE *gnu = 0;
	const char *name = "timing";
	size_t i, n = 1, e, replicas = 5;
#define X(n) n
	enum { EXPS(X) };
#undef X
#define X(n) { #n, 0, { 0, 0.0, 0.0 } }
	struct { const char *name; FILE *fp; struct measure m; }
		exp[] = { EXPS(X) };
	const size_t exp_size = sizeof exp / sizeof *exp;
#undef X
	struct closed_set closed = SET_IDLE;
	struct string_table open = TABLE_IDLE;
	struct backing backing = { POOL_IDLE, ARRAY_IDLE };
	closed_set(&closed);
	/* Open all graphs for writing. */
	for(e = 0; e < exp_size; e++) {
		char fn[64];
		if(sprintf(fn, "graph/%s.tsv", exp[e].name) < 0
			|| !(exp[e].fp = fopen(fn, "w"))) goto catch;
		fprintf(exp[e].fp, "# %s\n"
			"# <items>\t<t (ms)>\t<sample error on t with %lu replicas>\n",
			exp[e].name, (unsigned long)replicas);
	}
	/* Do experiment. */
	for(n = 1; n < 10000000; n <<= 1) {
		clock_t t_total;
		size_t r;
		for(e = 0; e < exp_size; e++) m_reset(&exp[e].m);
		for(r = 0; r < replicas; r++) {
			clock_t t;
			struct str16 *s16;
			struct closed_setlink *link;
			t_total = clock();
			printf("Replica %lu/%lu.\n", r + 1, replicas);

			/* It crashes if I don't have this, but no idea why. */
			closed_set(&closed);
			string_table(&open);

			/* Sorted array; pre-allocate for fair test. Don't worry about
			 unused references. */
			str16_pool_clear(&backing.str16s);
			closed_array_clear(&backing.closed);
			for(i = 0; i < n; i++) {
				if(!(s16 = str16_pool_new(&backing.str16s))
				   || !(link = closed_array_new(&backing.closed))) goto catch;
				orcish(s16->str, sizeof s16->str);
				link->key = s16->str;
				/*printf("Word pool %s\n", link->key); Spaammm. */
			}

			/* Set, (closed hash set.) (Don't put I/O in the test.) */
			t = clock();
			for(i = 0; i < n; i++) {
				link = backing.closed.data + i;
				if(closed_set_policy_put(&closed, link, 0))
					/*printf("Closed %s already.\n", link->key)*/;
			}
			m_add(&exp[Closed].m, diff_us(t));
			printf("Closed size %lu: %s.\n", (unsigned long)closed.size,
				closed_set_to_string(&closed));

			/* Table, (open hash set.) */
			t = clock();
			for(i = 0; i < n; i++) {
				char *const word = backing.closed.data[i].key;
				string_table_try(&open, word);
				/*switch(string_table_try(&open, word)) {
				case TABLE_ERROR: case TABLE_REPLACE: goto catch;
				case TABLE_YIELD: printf("Open %s already.\n", word); break;
				case TABLE_UNIQUE: printf("Open %s.\n", word); break;
				}*/
			}
			m_add(&exp[Open].m, diff_us(t));
			printf("Open size %lu: %s.\n", (unsigned long)open.size,
				string_table_to_string(&open));

			/* Took took much time; decrease the replicas for next time. */
			if(replicas != 1
				&& 10.0 * (clock() - t_total) / CLOCKS_PER_SEC > 1.0 * replicas)
				replicas--;

			/* Cut a slice to see if it's actually working. */
			if(n == 1024) {
				char fn[64];
				sprintf(fn, "graph/%s.gv", exp[0].name);
				set_closed_graph(&closed, fn);
				sprintf(fn, "graph/%s.gv", exp[1].name);
				table_string_graph(&open, fn);
			}
			/*closed_set_clear(&closed);
			string_table_clear(&open);*/
			closed_set_(&closed);
			string_table_(&open);
		}
		for(e = 0; e < exp_size; e++) {
			double stddev = m_stddev(&exp[e].m);
			if(stddev != stddev) stddev = 0; /* Is nan; happens. */
			fprintf(exp[e].fp, "%lu\t%f\t%f\n",
				(unsigned long)n, m_mean(&exp[e].m), stddev);
		}
	}
	goto finally;
catch:
	perror("timing"), assert(0);
finally:
	for(e = 0; e < exp_size; e++)
		if(exp[e].fp && fclose(exp[e].fp)) perror(exp[e].name);

	/* Output a `gnuplot` script. */
	{
		char fn[64];
		if(sprintf(fn, "graph/%s.gnu", name) < 0
			|| !(gnu = fopen(fn, "w"))) goto catch2;
		fprintf(gnu,
			"set style line 1 lt 5 lw 2 lc rgb '#0072bd'\n"
			"set style line 2 lt 5 lw 2 lc rgb '#ff0000'\n"
			"set style line 3 lt 5 lw 2 lc rgb '#00ac33'\n"
			"set style line 4 lt 5 lw 2 lc rgb '#19d3f5'\n");
		fprintf(gnu, "set term postscript eps enhanced color\n"
			/*"set encoding utf8\n" Doesn't work at all; {/Symbol m}. */
			"set output \"graph/%s.eps\"\n"
			"set grid\n"
			"set xlabel \"elements\"\n"
			"set ylabel \"time per element, t (ns)\"\n"
			"set yrange [0:]\n"
			"set log x\n"
			"plot", name);
		for(e = 0; e < exp_size; e++) fprintf(gnu,
			"%s \\\n\"graph/%s.tsv\" using 1:($2/$1*1000):($3/$1*1000) "
			"with errorlines title \"%s\" ls %d", e ? "," : "",
			exp[e].name, exp[e].name, (int)e + 1);
		fprintf(gnu, "\n");
	}
	if(gnu && fclose(gnu)) goto catch2; gnu = 0;
	{
		int result;
		char cmd[64];
		fprintf(stderr, "Running Gnuplot to get a graph of, \"%s,\" "
			"(http://www.gnuplot.info/.)\n", name);
		if((result = system("/usr/local/bin/gnuplot --version")) == -1)
			goto catch2;
		else if(result != EXIT_SUCCESS) { errno = EDOM; goto catch2; }
		if(sprintf(cmd, "/usr/local/bin/gnuplot graph/%s.gnu", name) < 0
			|| (result = system(cmd)) == -1) goto catch2;
		else if(result != EXIT_SUCCESS) { errno = EDOM; goto catch2; }
		fprintf(stderr, "Running open.\n");
		if(sprintf(cmd, "open graph/%s.eps", name) < 0
		   || (result = system(cmd)) == -1) goto catch2;
		else if(result != EXIT_SUCCESS) { errno = EDOM; goto catch2; }
	}
	goto finally2;
catch2:
	perror(name);
finally2:
	if(gnu && fclose(gnu)) perror(name);
	printf("\n");
}

#else /* timing --><!-- !timing */
static void timing_comparison(void)
	{ printf("%s:%lu: \"#if 0\" to do timing comparison.\n\n",
	__FILE__, (unsigned long)__LINE__); }
#endif /* timing --> */


/*#include <time.h>*/

int main(void) {
	struct str16_pool strings = POOL_IDLE;
	struct vec4_pool vec4s = POOL_IDLE;
	/*unsigned seed = (unsigned)clock();*/
	zodiac_table_test(&fill_zodiac, 0); /* Don't require any space. */
	string_table_test(&str16_from_void, &strings), str16_pool_(&strings);
	uint_table_test(&uint_from_void, 0);
	int_table_test(&int_from_void, 0);
	vec4_table_test(&vec4_from_void, &vec4s), vec4_pool_(&vec4s);
	star_table_test(&fill_star, 0);
	test_default();
	test_it();
	boat_club();
	linked_dict();
	year_of();
	nato();
	stars();
	timing_comparison();
	return EXIT_SUCCESS;
}

#if 0

/** <https://github.com/skeeto/hash-prospector>; it doesn't say the inverse. */
uint16_t hash16_xm2(uint16_t x) {
	x ^= x >> 8; x *= 0x88b5u;
	x ^= x >> 7; x *= 0xdb2du;
	x ^= x >> 9;
	return x;
}

/* We never did use this hash, but it's cool. Also:
 <https://github.com/aappleby/smhasher/>. */
#if 0x8000 * 2 == 0
#error 16-bit max length is not supported in this test.
#elif 0x80000000 * 2 == 0
/** We support 32 and 64-bit `size_t` but only if `size_t` is used as the
 maximum for integer constants, (not guaranteed.)
 <http://www.isthe.com/chongo/tech/comp/fnv/>
 <https://github.com/sindresorhus/fnv1a> */
static size_t fnv_32a_str(const char *const str) {
	const unsigned char *s = (const unsigned char *)str;
	unsigned hval = 0x811c9dc5;
	while(*s) hval ^= *s++, hval *= 0x01000193;
	return hval;
}
static size_t fnv_a_str(const char *const str) { return fnv_32a_str(str); }
#else /* 0x8000000000000000 * 2 == 0? doesn't matter; it's max 64bit. */
/** <http://www.isthe.com/chongo/tech/comp/fnv/> */
static size_t fnv_64a_str(const char *const str) {
	const unsigned char *s = (const unsigned char *)str;
	size_t hval = 0xcbf29ce484222325;
	while(*s) hval ^= *s++, hval *= 0x100000001b3;
	return hval;
}
static size_t fnv_a_str(const char *const str) { return fnv_64a_str(str); }
#endif

#endif

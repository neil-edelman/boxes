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


/* Zodiac is a bounded set of `enum`, set. */

/* An X-macro allows printing. This is preferable to `TABLE_KEY const char *`,
 (which leads to duplicate `const`.) */
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
static unsigned char zodiac_hash(const enum zodiac z)
	{ return (unsigned char)z; }
/** This is a discrete set with a simple homomorphism between keys and hash
 values, therefore it's simpler to work in hash space. This saves us from
 having to define <typedef:<PN>is_equal_fn> and saves the key from even being
 stored. (Still, this uses 16 bits to store 1 bit of information, it would be
 better as a bit-vector.) @implements <zodiac>unhash_fn */
static enum zodiac zodiac_unhash(const unsigned char z) { return z; }
/** This is not necessary except for testing, because that is how we see what
 we're testing. @implements <zodiac>to_string_fn */
static void zodiac_to_string(const enum zodiac z, char (*const a)[12])
	{ strcpy(*a, zodiac[z]); /* strlen z < 12 */ }
/* For testing; there is no extra memory required to generate random `enum`.
 @implements <zodiac>action_fn */
static void zodiac_filler(void *const zero, enum zodiac *const z) {
	(void)zero, assert(!zero);
	*z = (enum zodiac)(rand() / (RAND_MAX / ZodiacCount + 1));
}
#define TABLE_NAME zodiac
#define TABLE_KEY enum zodiac
#define TABLE_UNHASH /* If you can, inverse is less space and simpler. */
#define TABLE_UINT unsigned char /*<-small unsigned*/ /* `size_t` overkill. */
#define TABLE_TEST /* Testing requires to string. */
#define TABLE_TO_STRING /* Requires <../src/to_string.h>. */
#include "../src/table.h"


/* String set. */

/* A pool is convenient for testing because it allows deletion at random. */
struct str16 { char str[16]; };
#define POOL_NAME str16
#define POOL_TYPE struct str16
#include "../src/pool.h"
/** For testing: `s16s` is a pool of `str16`. */
static char *str16_from_pool(struct str16_pool *const s16s) {
	struct str16 *s16 = str16_pool_new(s16s);
	assert(s16);
	orcish(s16->str, sizeof s16->str);
	return s16->str;
}
/** @implements <string>test_new_fn */
static void string_filler(void *const s16s, char **const string)
	{ *string = str16_from_pool(s16s); }
/** One must supply the hash: djb2 <http://www.cse.yorku.ca/~oz/hash.html> is
 a simple one that is mostly `size_t`-length agnostic. It's not the greatest,
 but it doesn't need to be. @implements <string>hash_fn */
static size_t djb2(const char *s) {
	const unsigned char *str = (const unsigned char *)s;
	size_t hash = 5381, c;
	while(c = *str++) hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	return hash;
}
static size_t string_hash(const char *s) { return djb2(s); }
/** @implements <string>is_equal_fn */
static int string_is_equal(const char *const a, const char *const b)
	{ return !strcmp(a, b); }
/** @implements <string>to_string_fn */
static void string_to_string(const char *const s, char (*const a)[12])
	{ strncpy(*a, s, sizeof(*a) - 1), (*a)[sizeof(*a) - 1] = '\0'; }
#define TABLE_NAME string
#define TABLE_KEY char *
#define TABLE_DEFAULT 0 /* Enables `string_get`, do not assume 0 meaning. */
#define TABLE_TO_STRING
#define TABLE_TEST
#include "../src/table.h"


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
/** @implements <int>unhash_fn */
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
/** @implements <int>unhash_fn */
static unsigned lowbias32_r(unsigned x) { return x; }
#endif /* < 32 bits */
static unsigned uint_hash(const unsigned x) { return lowbias32(x); }
static unsigned uint_unhash(const unsigned x) { return lowbias32_r(x); }
/** @implements <int>to_string_fn */
static void uint_to_string(const unsigned x, char (*const a)[12])
	{ sprintf(*a, "%u", x); }
static void uint_filler(void *const zero, unsigned *const u) {
	(void)zero, assert(!zero && RAND_MAX <= 99999999999l); /* For printing. */
	*u = (unsigned)rand();
}
#define TABLE_NAME uint
#define TABLE_KEY unsigned /* Parameter of <fn:lowbias32>. */
#define TABLE_UINT unsigned /* Return key of <fn:lowbias32>. */
#define TABLE_UNHASH /* Invertible means no key storage at all. */
#define TABLE_TEST
#define TABLE_TO_STRING
#include "../src/table.h"


/* Check to see that the prototypes are correct by making a signed integer.
 Also testing `TABLE_DEFAULT`. */
static unsigned int_hash(const int d)
	{ return lowbias32((unsigned)d - (unsigned)INT_MIN); }
static int int_unhash(unsigned u)
	{ return (int)(lowbias32_r(u) + (unsigned)INT_MIN); }
static void int_to_string(const int d, char (*const a)[12])
	{ sprintf(*a, "%d", d); }
static void int_filler(void *const zero, int *const s) {
	(void)zero, assert(!zero && RAND_MAX <= 9999999999l); /* With '-'. */
	*s = rand() - RAND_MAX / 2;
}
#define TABLE_NAME int
#define TABLE_KEY int
#define TABLE_UINT unsigned
#define TABLE_UNHASH
#define TABLE_TO_STRING
#define TABLE_DEFAULT 0
#define TABLE_TEST
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
#define TABLE_TRAIT 42
#define TABLE_DEFAULT 42
#include "../src/table.h"


/** Vector hash implemented as a pointer. This is kind of a silly example
 because it's easily homomorphic to a set of integers, but pretend we had a big
 problem space, (such an example would be difficult to describe succinctly.) */
struct vec4 { char a[2]; int n[2]; };
/* Testing. */
#define POOL_NAME vec4
#define POOL_TYPE struct vec4
#include "../src/pool.h"
static struct vec4 *vec4_from_pool(struct vec4_pool *const v4s) {
	struct vec4 *v4 = vec4_pool_new(v4s);
	if(!v4) return 0;
	v4->a[0] = 'A' + (char)(rand() / (RAND_MAX / 26 + 1));
	v4->a[1] = 'a' + (char)(rand() / (RAND_MAX / 26 + 1));
	v4->n[0] = rand() / (RAND_MAX / 9 + 1);
	v4->n[1] = rand() / (RAND_MAX / 9 + 1);
	return v4;
}
static void vec4_filler(void *const vec4s, struct vec4 **const v)
	{ assert(vec4s), *v = vec4_from_pool(vec4s), assert(*v); }
/* Hash table. */
static unsigned vec4_hash(const struct vec4 *const v4) {
	return (unsigned)(1 * v4->n[0] + 10 * v4->n[1]
		+ 100 * (v4->a[0] - 'A') + 26000 * (v4->a[1] - 'a'));
}
static int vec4_is_equal(const struct vec4 *a, const struct vec4 *const b) {
	return a->a[0] == b->a[0] && a->a[1] == b->a[1]
		&& a->n[0] == b->n[0] && a->n[1] == b->n[1];
}
static void vec4_to_string(const struct vec4 *const v4, char (*const a)[12]) {
	sprintf(*a, "%c%d%c%d", v4->a[0], v4->n[0] % 100, v4->a[1], v4->n[1] % 100);
}
#define TABLE_NAME vec4
#define TABLE_KEY struct vec4 *
#define TABLE_IS_EQUAL
/* Because of alignment, doesn't buy anything in terms of space savings. */
#define TABLE_UINT unsigned
#define TABLE_TEST
#define TABLE_TO_STRING
#include "../src/table.h"


/** Too lazy to do separate tests. */
static void test_default(void) {
	struct int_table t = int_table();
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
	struct zodiac_table z = zodiac_table();
	struct zodiac_table_iterator zit;
	struct int_table t = int_table();
	struct int_table_iterator it;
	const int no_till = 1000, no_till2 = no_till * 2;
	int n;

	printf("Testing zodiac remove iterator.\n");
	if(!zodiac_table_try(&z, Sagittarius) || !zodiac_table_try(&z, Capricorn)
		|| !zodiac_table_try(&z, Gemini) || !zodiac_table_try(&z, Aries)
		|| !zodiac_table_try(&z, Virgo) || !zodiac_table_try(&z, Libra)
		|| !zodiac_table_try(&z, Taurus)) goto catch;
	table_zodiac_graph(&z, "graph/it-z0.gv");
	printf("Remove all zodiac one at a time.\n");
	zit = zodiac_table_iterator(&z), n = 0;
	while(zodiac_table_next(&zit)) {
		char fn[64];
		printf("On %s.\n", zodiac[zodiac_table_key(&zit)]);
		if(!zodiac_table_iterator_remove(&zit)) printf("(that's weird?)\n");
		sprintf(fn, "graph/it-z%d.gv", ++n);
		table_zodiac_graph(&z, fn);
	}
	assert(!z.size);
	zodiac_table_(&z);

	printf("Testing iteration with elements [0, %d).\n", no_till2);
	for(n = 0; n < no_till2; n++) if(!int_table_try(&t, n)) goto catch;
	table_int_graph(&t, "graph/it0.gv");
	assert(t.size == no_till2);
	/* Even ones get deleted. */
	printf("Remove: ");
	it = int_table_iterator(&t);
	while(int_table_next(&it)) if(!(int_table_key(&it) & 1)
		&& !int_table_iterator_remove(&it)) printf("(that's weird?)");
	printf("done.\n");
	table_int_graph(&t, "graph/it1.gv");
	assert(t.size == no_till);
	it = int_table_iterator(&t);
	while(int_table_next(&it)) assert(int_table_key(&it) & 1);
	goto finally;
catch:
	perror("it"), assert(0);
finally:
	zodiac_table_(&z);
	int_table_(&t);
	printf("\n");
}


/* <https://stackoverflow.com/q/59091226/2472827>. */
struct boat_record { int best_time, points; };
static unsigned boat_hash(const int x) { return int_hash(x); }
static int boat_unhash(const unsigned h) { return int_unhash(h); }
#define TABLE_NAME boat
#define TABLE_KEY int
#define TABLE_UINT unsigned
#define TABLE_VALUE struct boat_record
#define TABLE_UNHASH
#include "../src/table.h"
static void boat_club(void) {
	struct boat_table boats = boat_table();
	struct boat_table_iterator it;
	size_t i;
	printf("Boat club races:\n");
	for(i = 0; i < 1000; i++) {
		/* Pigeon-hole principle ensures collisions. */
		const int id = rand() / (RAND_MAX / /*89<-spam*/29 + 1) + 10,
			time = rand() / (RAND_MAX / 100 + 1) + 50,
			points = 151 - time;
		struct boat_record *record;
		/*printf("Boat #%d had a time of %d, giving them %d points.\n",
			id, time, points);*/
		switch(boat_table_assign(&boats, id, &record)) {
		case TABLE_ERROR: goto catch;
		case TABLE_ABSENT: /* First time for boat with this id. */
			record->best_time = time; record->points = points; break;
		case TABLE_PRESENT: /* Returning time. */
			if(time < record->best_time) {
				printf("#%d best time %d -> %d.\n", id, record->best_time, time);
				record->best_time = time;
			}
			/*printf("#%d points %d -> %d.\n",
				id, record->points, record->points + points);*/
			record->points += points;
			break;
		}
	}
	printf("Final score:\n"
		"id\tbest\tpoints\n");
	it = boat_table_iterator(&boats);
	while(boat_table_next(&it))
		printf("%d\t%d\t%d\n", boat_table_key(&it),
		boat_table_value(&it)->best_time, boat_table_value(&it)->points);
	goto finally;
catch:
	perror("boats"), assert(0);
finally:
	boat_table_(&boats);
	printf("\n");
}


/* <https://en.wikipedia.org/wiki/List_of_brightest_stars> and light-years from
 Sol. As a real example, this is silly; it would be much better suited to
 `gperf` because the data is known beforehand. Also, see <fn:hash_zodiac>.
 (It's a good example for the article.) */
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
	/* We don't sanitize GraphViz, and this messes with graph.
	X(2.30 (2.29–2.34var), 550), X(2.30 (2.29–2.31var), 380),*/ \
	X(Dschubba, 400), X(Larawag, 65), /*X(2.35 (2.30–2.41var), 310),*/ \
	X(Merak, 79), X(Ankaa, 77), X(Girtab, 460), X(Enif, 670), X(Scheat, 200), \
	X(Sabik, 88), X(Phecda, 84), X(Aludra, 2000), X(Markeb, 540), \
	X(Navi, 610), X(Markab, 140), X(Aljanah, 72), X(Acrab, 404)
#define X(n, m) #n
/* const is not supported; duplicate; fixme */
static char *star_names[] = { STARS };
#undef X
static const size_t stars_size = sizeof star_names / sizeof *star_names;
#define X(n, m) m
static const double star_distances[] = { STARS };
#undef X
struct star_table_entry;
static void star_filler(void *const zero, struct star_table_entry *const star);
/** Big numbers are hard to understand and useless to explain in an article.
 @implements <star>hash */
static unsigned char star_hash(const char *const s)
	{ return (unsigned char)djb2(s); }
static int star_is_equal(const char *const a, const char *const b) {
	return string_is_equal(a, b);
}
static void star_to_string(const char *const s, const double d,
	char (*const a)[12]) { string_to_string(s, a); (void)d; }
#define TABLE_NAME star
#define TABLE_KEY char *
#define TABLE_VALUE double
#define TABLE_UINT unsigned char
#define TABLE_DEFAULT 0
#define TABLE_TEST
#define TABLE_TO_STRING
#include "../src/table.h"
static void star_filler(void *const zero, struct star_table_entry *const star) {
	size_t r = (size_t)rand() / (RAND_MAX / stars_size + 1);
	(void)zero, assert(!zero);
	star->key = star_names[r];
	star->value = star_distances[r];
}


/* Linked dictionary: linked-list with a comparison order indexed by a map.
 This is just the thing we said in `TABLE_VALUE` to avoid, and probably is the
 worst-case in terms of design of the table. It's associated, so we don't
 really have a nice choice; were we to aggregate it, then it would be a fixed
 size or face alignment issues. */
struct word_listlink;
static int word_compare(const struct word_listlink *,
	const struct word_listlink *);
#define LIST_NAME word
#define LIST_COMPARE
#include "../src/list.h"
/** Associated with the word and a link to other words. */
struct dict { struct word_listlink link; char *word; };
/** `container_of` `link`; `offsetof`, in this case, is zero; we could have
 used a cast. */
static const struct dict *dictlink_upcast(const struct word_listlink *const
	link) { return (const struct dict *)(const void *)
	((const char *)link - offsetof(struct dict, link)); }
/** Compare `a` and `b`. */
static int word_compare(const struct word_listlink *const a,
	const struct word_listlink *const b)
	{ return strcmp(dictlink_upcast(a)->word, dictlink_upcast(b)->word); }
/* We take them on-line as they come, so we need a stable pool. */
#define POOL_NAME dict
#define POOL_TYPE struct dict
#include "../src/pool.h"
/* Words which we read all at once from a file, modify them in-place. */
#define ARRAY_NAME char
#define ARRAY_TYPE char
#include "../src/array.h"
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
static size_t dict_hash(const char *const d) { return djb2(d); }
static void dict_to_string(const char *const key,
	const struct dict *const defn, char (*const a)[12])
	{ string_to_string(key, a); (void)defn; }
static int dict_is_equal(const char *const a, const char *const b)
	{ return !strcmp(a, b); }
/* A string set with a pointer to dict map. It duplicates data from `key` and
 `value->word`, but table is complicated enough as it is. It has to be a
 pointer because a table is not stable, (not guaranteed stability even looking
 at it.) It was easier to make a linked-dictionary in separate-chaining,
 because it's all independent, but it was such a pain to understand, I couldn't
 see anyone using it. Tries are better anyway. */
#define TABLE_NAME dict
#define TABLE_KEY char *
#define TABLE_VALUE struct dict *
#define TABLE_IS_EQUAL &string_is_equal
#define TABLE_DEFAULT 0
#define TABLE_TO_STRING &string_to_string
#include "../src/table.h"
/* `re2c` is very useful for file input. */
#include "lex_dict.h"
/** Manual test. */
static void linked_dict(void) {
	struct char_array english = char_array();
	const char *const inglesi_fn = "test/Tutte_le_parole_inglesi.txt";
	struct dict_table dict = dict_table();
	struct dict_pool pool = dict_pool();
	struct word_list order;
	struct lex_state state = { 0, 0, 0 }; /* Defined in <lex.h>. */
	printf("Testing linked-dictionary.\n");
	if(!append_file(&english, inglesi_fn)) goto catch;
	word_list_clear(&order);
	state.cursor = english.data, state.line = 1;
	while(lex_dict(&state)) { /* Cut off the string in the `char` array. */
		struct dict **ptr, *d;
		switch(dict_table_assign(&dict, state.word, &ptr)) {
		case TABLE_ERROR: goto catch;
		case TABLE_PRESENT: printf("Next line %lu: duplicate \"%s\"; ignoring.\n",
			(unsigned long)state.line, state.word); continue;
		case TABLE_ABSENT:
			if(!(d = *ptr = dict_pool_new(&pool))) goto catch;
			d->word = state.word;
			word_list_push(&order, &d->link);
		}
	}
	/* Natural merge sort, `O(n)` when it's (almost?) in order. (They are
	 presumably already sorted.) */
	word_list_sort(&order);
	printf("Dictionary: %s.\n", dict_table_to_string(&dict));
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
			if(!(found = dict_table_get(&dict, look))) { printf(
				"That's weird; \"%s\" wasn't found in the dictionary.\n", look);
				assert(0); continue; }
			before = found->link.prev
				? dictlink_upcast(found->link.prev)->word : "start";
			after = found->link.next
				? dictlink_upcast(found->link.next)->word : "end";
			printf("Found \"%s\" between …%s, %s, %s…\n",
				look, before, found->word, after);
			assert(!strcmp(look, found->word)
				&& !strcmp(befs[r - rando], before)
				&& !strcmp(afts[r - rando], after));
		}
	}
	goto finally;
catch:
	perror(inglesi_fn), assert(0);
finally:
	dict_table_(&dict);
	dict_pool_(&pool);
	char_array_(&english);
	printf("\n");
}


/* Int set that is a subclass of a larger parent using a pointer. */
struct year;
typedef void (*year_to_string_fn)(const struct year *, char (*)[12]);
struct year_vt { year_to_string_fn to_string; };
struct year { int year, unused; struct year_vt vt; };
static unsigned year_hash(const int *const year) { return int_hash(*year); }
static int year_is_equal(const int *const a, const int *const b)
	{ return *a == *b; }
/** `container_of` integer `year`, which is `struct year`. */
static const struct year *year_upcast(const int *const year)
	{ return (const struct year *)(const void *)
	((const char *)year - offsetof(struct year, year)); }
static void year_to_string(const int *const year, char (*const a)[12])
	{ year_upcast(year)->vt.to_string(year_upcast(year), a); }
/* Can not use `TABLE_UNHASH` because it's not a bijection; the child is also
 part of the data. Arguably, `TABLE_DO_NOT_CACHE` would be useful in this
 situation, but I took it out. Too many options. */
#define TABLE_NAME year
#define TABLE_KEY int *
#define TABLE_UINT unsigned
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
#include "../src/pool.h"
#define POOL_NAME snake
#define POOL_TYPE struct snake
#include "../src/pool.h"
#define POOL_NAME letter
#define POOL_TYPE struct letter
#include "../src/pool.h"
/** Example of a set with a pointer key. */
static void year_of(void) {
	struct year_table year = year_table();
	struct colour_pool colourpool = colour_pool();
	struct snake_pool snakepool = snake_pool();
	struct letter_pool letterpool = letter_pool();
	size_t i;
	printf("Testing table pointer set key.\n");
	for(i = 0; i < 500; i++) {
		struct year *y = 0;
		int *eject;
		switch(rand() / (RAND_MAX / 3 + 1)) {
		case 0: { struct colour *c;
			if(!(c = colour_pool_new(&colourpool))) goto catch;
			fill_colour(c), y = &c->year; } break;
		case 1: { struct snake *s;
			if(!(s = snake_pool_new(&snakepool))) goto catch;
			fill_snake(s), y = &s->year; } break;
		case 2: { struct letter *a;
			if(!(a = letter_pool_new(&letterpool))) goto catch;
			fill_letter(a), y = &a->year; } break;
		}
		switch(year_table_update(&year, &y->year, &eject)) {
		case TABLE_ERROR: goto catch;
		case TABLE_PRESENT: { char e[12], z[12];
			year_to_string(eject, &e);
			year_to_string(&y->year, &z);
			printf("Replaced %s with %s.\n", e, z);
			/* We could have a `delete` in the virtual table which frees
			 unneeded memory. This is just an example, we'll allow it all to be
			 deleted later; it just sits there unused, now. */ } break;
		case TABLE_ABSENT: break;
		}
	}
	printf("Year of %s.\n", year_table_to_string(&year));
	goto finally;
catch:
	perror("year"), assert(0);
finally:
	year_table_(&year);
	colour_pool_(&colourpool); /* This is where it's deleted. */
	snake_pool_(&snakepool);
	letter_pool_(&letterpool);
	printf("\n");
}


/* A histogram of lengths' defined as a map with the pointers to the keys
 recorded as a linked-list. */
struct nato_node { const char *alpha; struct nato_node *next; };
struct nato_value { size_t occurrences; struct nato_node *head; };
/** Symmetric bijection. @implements <nato>hash_fn, <nato>unhash_fn */
static size_t nato_hash(const size_t n) { return n; }
static size_t nato_unhash(const size_t h) { return h; }
#define TABLE_NAME nato
#define TABLE_KEY size_t /* Number of letters. */
#define TABLE_VALUE struct nato_value /* Count and letters. */
#define TABLE_UNHASH
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
	/* Pre-allocate one node for each letter. */
	struct nato_node list[sizeof alphabet / sizeof *alphabet];
	struct nato_table nato = nato_table();
	struct nato_table_iterator it;
	size_t length_of_word;
	struct nato_value *value;
	size_t i;
	for(i = 0; i < sizeof alphabet / sizeof *alphabet; i++) {
		struct nato_node *const item = list + i;
		length_of_word = utf_alnum_count(alphabet[i]);
		switch(nato_table_assign(&nato, length_of_word, &value)) {
		case TABLE_ERROR: goto catch;
		case TABLE_ABSENT: value->occurrences = 1, value->head = 0; break;
		case TABLE_PRESENT: value->occurrences++; break;
		}
		item->alpha = alphabet[i];
		item->next = value->head, value->head = item; /* Linked append. */
	}
	printf("NATO phonetic alphabet letter count histogram\n"
		"length\tcount\twords\n");
	it = nato_table_iterator(&nato);
	while(nato_table_next(&it)) {
		struct nato_node *w;
		length_of_word = nato_table_key(&it);
		value = nato_table_value(&it);
		printf("%lu\t%lu\t{", (unsigned long)length_of_word,
			(unsigned long)value->occurrences);
		w = value->head;
		do printf("%s%s", value->head == w ? "" : ",", w->alpha);
		while(w = w->next);
		printf("}\n");
	}
	{ /* Check. */
		size_t count;
		int success;
		const struct nato_value zero = { 0, 0 };
		struct nato_value v;
		success = nato_table_query(&nato, 0, &count, &v);
		assert(!success);
		success = nato_table_query(&nato, 3, &count, &v);
		assert(!success);
		success = nato_table_query(&nato, 4, &count, &v);
		assert(success && count == 4 && v.occurrences == 8);
		success = nato_table_query(&nato, 5, &count, &v);
		assert(success && count == 5 && v.occurrences == 8);
		success = nato_table_query(&nato, 6, &count, &v);
		assert(success && count == 6 && v.occurrences == 6);
		success = nato_table_query(&nato, 7, &count, &v);
		assert(success && count == 7 && v.occurrences == 3);
		success = nato_table_query(&nato, 8, &count, &v);
		assert(success && count == 8 && v.occurrences == 1);
		v.occurrences =  0;
		v = nato_table_get_or(&nato, 0, zero), assert(v.occurrences == 0);
		v = nato_table_get_or(&nato, 4, zero), assert(v.occurrences == 8);
		v = nato_table_get_or(&nato, 5, zero), assert(v.occurrences == 8);
		v = nato_table_get_or(&nato, 6, zero), assert(v.occurrences == 6);
		v = nato_table_get_or(&nato, 7, zero), assert(v.occurrences == 3);
		v = nato_table_get_or(&nato, 8, zero), assert(v.occurrences == 1);
	}
	goto finally;
catch:
	perror("nato"), assert(0);
finally:
	nato_table_(&nato);
	printf("\n");
}


/** Contrived example for paper. */
static void stars(void) {
	struct star_table stars = star_table();
	const size_t s_array[] = { 0, 1, 8, 9, 11, 16, 17, 20, 22, 25, 49 };
	size_t i;
	for(i = 0; i < sizeof s_array / sizeof *s_array; i++) {
		size_t s = s_array[i];
		char *key = star_names[s];
		double distance = star_distances[s], *content;
		printf("%lu: %s -> %f\n", (unsigned long)s, key, distance);
		if(star_table_assign(&stars, key, &content) != TABLE_ABSENT)
			goto catch; /* Unique names. */
		*content = distance;
	}
	printf("%s.\n", star_table_to_string(&stars));
	table_star_graph(&stars, "doc/table-precursor/star-raw.gv");
	goto finally;
catch:
	assert(0);
finally:
	star_table_(&stars);
	printf("\n");
}


/* Simulated going in header. */
#define TABLE_NAME public
#define TABLE_KEY enum zodiac
#define TABLE_UNHASH
#define TABLE_UINT unsigned char
#define TABLE_HEAD
#include "../src/table.h"
static unsigned char public_hash(const enum zodiac z) { return zodiac_hash(z); }
static enum zodiac public_unhash(const unsigned char z)
	{ return zodiac_unhash(z); }
static void public_to_string(const enum zodiac z, char (*const a)[12])
	{ zodiac_to_string(z, a); }
static void public_filler(void *const zero, enum zodiac *const z)
	{ zodiac_filler(zero, z); }
#define TABLE_NAME public
#define TABLE_KEY enum zodiac
#define TABLE_UNHASH
#define TABLE_UINT unsigned char
#define TABLE_TEST
#define TABLE_TO_STRING
#define TABLE_BODY
#include "../src/table.h"


int main(void) {
	struct str16_pool strings = str16_pool();
	struct vec4_pool vec4s = vec4_pool();
	zodiac_table_test(0); /* Don't require any space. */
	public_table_test(0); /* Export public functions. */
	string_table_test(&strings), str16_pool_(&strings);
	uint_table_test(0);
	int_table_test(0);
	vec4_table_test(&vec4s), vec4_pool_(&vec4s);
	star_table_test(0);
	test_default();
	test_it();
	stars();
	boat_club();
	linked_dict();
	year_of();
	nato();
	return EXIT_SUCCESS;
}

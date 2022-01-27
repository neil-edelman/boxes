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
/** This is a discrete set with a homomorphism between keys and hash values,
 therefore it's simpler to work in hash space. This saves us from having to
 define <typedef:<PN>is_equal_fn> and saves the key from even being stored.
 @implements <zodiac>inverse_hash_fn */
static enum zodiac hash_inv_zodiac(const unsigned z) { return z; }
/** This is not necessary except for testing.
@implements <zodiac>to_string_fn */
static void zodiac_to_string(const enum zodiac z, char (*const a)[12])
	{ strcpy(*a, zodiac[z]); /* strlen z < 12 */ }
#define TABLE_NAME zodiac
#define TABLE_KEY enum zodiac
#define TABLE_HASH &hash_zodiac
/* Generally, if you can, inverse is less space and simpler then equals. */
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
	assert(!zero);
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
	assert(!zero && RAND_MAX <= 99999999999l); /* For printing. */
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
	assert(!zero && RAND_MAX <= 9999999999l); /* For printing with '-'. */
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
}


/** Too lazy to do separate tests. */
static void test_default(void) {
	printf("Testing get.\n");
	struct int_table t = TABLE_IDLE;
	int one, two, def;
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
}


struct boat_record { int best_time, points; };
struct boat_table_entry;
static void boat_to_string(struct boat_table_entry, char (*)[12]);
#define TABLE_NAME boat
#define TABLE_KEY int
#define TABLE_UINT unsigned
#define TABLE_VALUE struct boat_record
#define TABLE_HASH &int_hash
#define TABLE_INVERSE &int_inv_hash
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
#define TABLE_TO_STRING &boat_to_string
#include "../src/table.h"
/** @implements <boat>to_string_fn */
static void boat_to_string(const struct boat_table_entry e, char (*const a)[12]) {
	/* Should be more careful about overflow? */
	/*sprintf(*a, "#%d(%d)", b->id.key, b->points);*/
	
	sprintf(*a, "#%d(%d)", e.key, e.value.points);
}
/** <https://stackoverflow.com/q/59091226/2472827>. */
static void boat_club(void) {
	/* fixme: TABLE_IDLE does not have enough info; how does this not give
	 a warning? */
	struct boat_table boats = TABLE_IDLE;
	size_t i;
	int success = 0;
	printf("Boat club races:\n");
	for(i = 0; i < 10; i++) {
		/* Pigeon-hole principle ensures collisions. */
		const int id = rand() / (RAND_MAX / 89 + 1) + 10,
			time = rand() / (RAND_MAX / 100 + 1) + 50,
			points = 151 - time;
		struct boat_record *record;
		printf("Boat #%d had a time of %d, giving them %d points.\n",
			id, time, points);
		switch(boat_table_compute(&boats, id, &record)) {
		case TABLE_UNIQUE:
			record->best_time = time; record->points = points; break;
		case TABLE_YIELD:
			if(time < record->best_time) record->best_time = time;
			record->points += points;
			break;
		case TABLE_ERROR: case TABLE_REPLACE: goto catch;
		}
	}
	printf("Final score: %s\n", boat_table_to_string(&boats));
	{ success = 1; goto finally; }
catch:
	perror("boats"), assert(0);
finally:
	boat_table_(&boats);
}


int main(void) {
	struct str16_pool strings = POOL_IDLE;
	struct vec4_pool vec4s = POOL_IDLE;
	zodiac_table_test(&fill_zodiac, 0); /* Don't require any space. */
	string_table_test(&str16_from_void, &strings), str16_pool_(&strings);
	uint_table_test(&uint_from_void, 0);
	int_table_test(&int_from_void, 0);
	vec4_table_test(&vec4_from_void, &vec4s);
	test_default();
	nato();
	boat_club();

	return EXIT_SUCCESS;











#if 0
	{ /* Linked dictionary. */
		struct entry_pool buckets = POOL_IDLE;
		const size_t limit = (size_t)500000/*0<-This takes a while to hash up.*/;
		struct dict_entry *e = 0, *sp_es[20], **sp_e, **sp_e_end = sp_es,
			*const*const sp_e_lim = sp_es + sizeof sp_es / sizeof *sp_es;
		struct key_hash khash = TABLE_IDLE;
		struct key_list klist;
		struct key_hashlink *elem;
		struct dict_entry *found;
		size_t line, unique = 0;
		int is_used = 1;
		key_list_clear(&klist);
		for(line = 0; line < limit; line++) {
			if(is_used && !(e = entry_pool_new(&buckets)))
				{ perror("Memory error"); assert(0); return EXIT_FAILURE; }
			entry_fill(e);
			if(!key_hash_reserve(&khash, 1))
				{ perror("Memory error"); assert(0); return EXIT_FAILURE; }
			/* Don't go replacing elements; `elem` is hash on collision. */
			elem = key_hash_policy_put(&khash, &e->elem, 0);
			if(elem) { assert(elem == &e->elem), is_used = 0; continue; }
			is_used = 1;
			unique++;
			/* Keep the list up to date with the hashmap. */
			key_list_push(&klist, &e->node);
			/* Test if we can find `sp_e_lim` of them; presumably they will be
			 randomised by sorting, but accessable to the `KeySet` on `O(1)`. */
			if(sp_e_end >= sp_e_lim) continue;
			printf("Looking for %s.\n", e->key);
			*(sp_e_end++) = e;
		}
		printf("Sorting %lu elements.\n", (unsigned long)unique);
		key_list_sort(&klist);
		for(sp_e = sp_es; sp_e < sp_e_end; sp_e++) {
			const struct dict_entry *prev, *next;
			elem = key_hash_get(&khash, (*sp_e)->key);
			assert(elem);
			found = elem_upcast(elem);
			prev = entry_prev(found);
			next = entry_next(found);
			printf("Found %s between …%s, %s, %s…\n", (*sp_e)->key,
				prev ? prev->key : "start", found->key,
				next ? next->key : "end");
			assert(found == *sp_e);
		}
		key_hash_(&khash);
		entry_pool_(&buckets);
	}
	{ /* Fill it with the actual dictionary. */
		const char *const english = "test/Tutte_le_parole_inglesi.txt";
		FILE *fp = fopen(english, "r");
		struct entry_pool buckets = POOL_IDLE;
		struct dict_entry *e, *sp_es[20], **sp_e;
		size_t sp_es_size = sizeof sp_es / sizeof *sp_es,
			sp_e_to_go = sp_es_size;
		struct key_hash khash = TABLE_IDLE;
		struct key_list klist;
		struct key_hashlink *elem;
		struct dict_entry *found;
		size_t line = 1, words_to_go = 216555, key_len;
		assert(RAND_MAX >= words_to_go);
		key_list_clear(&klist);
		if(!fp) goto catch;
		/* Read file. */
		for( ; ; line++) {
			char *key_end;
			if(!(e = entry_pool_new(&buckets))) goto catch;
			e->elem.key = e->key;
			if(!fgets(e->key, sizeof e->key, fp)) {
				/* Doesn't really do anything. */
				entry_pool_remove(&buckets, e);
				if(ferror(fp)) { if(!errno) errno = EILSEQ; goto catch; }
				break;
			}
			key_len = strlen(e->key);
			assert(key_len);
			key_end = e->key + key_len - 1;
			/* Words > sizeof e->key - 2, (I guess electroencapholographist is
			 not on there.) */
			if(*key_end != '\n') { errno = ERANGE; goto catch; }
			*key_end = '\0';
			/* Never mind about the value; it is not used. */
			elem = key_hash_policy_put(&khash, &e->elem, 0);
			if(elem) {
				fprintf(stderr, "%lu: ignoring %s already in word list.\n",
					(unsigned long)line, e->key);
				entry_pool_remove(&buckets, e);
				words_to_go--;
				continue;
			}
			key_list_push(&klist, &e->node);
			/* In order already: must do the probability thing. */
			if(!words_to_go) {
				fprintf(stderr, "%lu: count inaccurate.\n",
					(unsigned long)line);
				continue;
			}
			if((unsigned)rand() / (RAND_MAX / (unsigned)words_to_go-- + 1)
				>= sp_e_to_go) continue;
			printf("Looking for %s.\n", e->key);
			sp_es[sp_es_size - sp_e_to_go--] = e;
		}
		printf("Sorting %lu lines, (they are presumably already sorted.)\n",
			(unsigned long)line - 1);
		key_list_sort(&klist);
		for(sp_e = sp_es; sp_e < sp_es + sp_es_size; sp_e++) {
			const struct dict_entry *prev, *next;
			elem = key_hash_get(&khash, (*sp_e)->key);
			assert(elem);
			found = elem_upcast(elem);
			prev = entry_prev(found);
			next = entry_next(found);
			printf("Found %s between …%s, %s, %s…\n", (*sp_e)->key,
				prev ? prev->key : "start", found->key,
				next ? next->key : "end");
			assert(found == *sp_e);
		}
		goto finally;
catch:
		fprintf(stderr, "%lu:", (unsigned long)line);
		perror(english);
finally:
		if(fp) fclose(fp);
		key_hash(&khash);
		entry_pool_(&buckets);
	}
#endif
}





#if 0

/* <https://github.com/aappleby/smhasher/> */

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

/** <https://github.com/skeeto/hash-prospector> */
uint16_t hash16_xm2(uint16_t x) {
	x ^= x >> 8; x *= 0x88b5u;
	x ^= x >> 7; x *= 0xdb2du;
	x ^= x >> 9;
	return x;
}

#endif

#if 0







/* Linked dictionary. */

static int key_is_equal(const char *const a, const char *const b) {
	return !strcmp(a, b);
}
static void key_to_string(const char *const*const ps, char (*const a)[12]) {
	strncpy(*a, *ps, sizeof(*a) - 1);
	(*a)[sizeof(*a) - 1] = '\0';
}
#define TABLE_NAME key
#define TABLE_KEY const char *
#define TABLE_HASH &fnv_32a_str
#define TABLE_IS_EQUAL &key_is_equal
#define TABLE_EXPECT_TRAIT
#include "../src/table.h"
#define TABLE_TO_STRING &key_to_string
#include "../src/table.h"

struct key_listlink;
static int key_compare(const struct key_listlink *,
	const struct key_listlink *);
#define LIST_NAME key
#define LIST_COMPARE &key_compare
#include "list.h"

struct dict_entry {
	struct key_hashlink elem;
	struct key_listlink node;
	char key[24];
	char value[32];
};

/* Container of `elem`. */
static struct dict_entry *elem_upcast(struct key_hashlink *const elem) {
	return (struct dict_entry *)(void *)((char *)elem
		- offhashof(struct dict_entry, elem));
}
/* `const` container of `node`. */
static const struct dict_entry
	*node_upcast_c(const struct key_listlink *const node)
	{ return (const struct dict_entry *)(const void *)((const char *)node
	- offhashof(struct dict_entry, node)); }
/** @implements <key_list_node>compare */
static int key_compare(const struct key_listlink *const a,
	const struct key_listlink *const b) {
	return strcmp(node_upcast_c(a)->elem.key, node_upcast_c(b)->elem.key);
}
static void entry_fill(struct dict_entry *const e) {
	assert(e);
	e->elem.key = e->key;
	orcish(e->key, sizeof e->key);
	orcish(e->value, sizeof e->value);
}
static const struct dict_entry *entry_prev(struct dict_entry *const e) {
	const struct key_listlink *const prev = key_list_previous(&e->node);
	assert(e);
	return prev ? node_upcast_c(prev) : 0;
}
static const struct dict_entry *entry_next(struct dict_entry *const e) {
	const struct key_listlink *const next = key_list_next(&e->node);
	assert(e);
	return next ? node_upcast_c(next) : 0;
}

#define POOL_NAME bucket
#define POOL_TYPE struct dict_entry
#include "pool.h"

#endif /* only one */


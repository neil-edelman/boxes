/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT). */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include "orcish.h"


/* Zodiac is a bounded set of `enum`. An X-macro allows printing. This is
 preferable to `SET_KEY const char *`. */
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
 define <typedef:<PS>is_equal_fn> and saves the key from even being stored.
 @implements <zodiac>inverse_hash_fn */
static enum zodiac hash_inv_zodiac(const unsigned z) { return z; }
/** This is not necessary except for testing.
@implements <zodiac>to_string_fn */
static void zodiac_to_string(const enum zodiac z, char (*const a)[12])
	{ strcpy(*a, zodiac[z]); /* strlen z < 12 */ }
#define SET_NAME zodiac
#define SET_KEY enum zodiac
#define SET_HASH &hash_zodiac
#define SET_INVERSE &hash_inv_zodiac
/* <tag:<PS>bucket>: <typedef:<PS>uint> `next`, `hash`. `enum` values really
 are overkill to represent with a `size_t`. There are less then 128 keys, so
 a byte would suffice (/2). Lengthening them as they come out is probably
 slower. */
#define SET_UINT unsigned /*char*/
#define SET_TEST /* Testing requires to string. */
#define SET_EXPECT_TRAIT
#include "../src/set.h"
#define SET_TO_STRING &zodiac_to_string
#include "../src/set.h"
/* For testing; there is no extra memory required to generate random `enum`.
 @implements <zodiac>fill_fn */
static int fill_zodiac(void *const zero, enum zodiac *const z) {
	(void)zero;
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
#define SET_NAME string
#define SET_KEY char * /* Parameter of <fn:djb2_hash> (without `const`.) */
#define SET_HASH &djb2_hash /* Default returns `size_t`. */
#define SET_IS_EQUAL &string_is_equal
#define SET_TEST /* Testing requires to string. */
#define SET_EXPECT_TRAIT
#include "../src/set.h"
#define SET_TO_STRING &string_to_string
#include "../src/set.h"
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
/** Uniform values don't need a hash, and I'm lazy.
 @implements <int>hash_fn */
static unsigned lowbias32(unsigned x) { return x; }
/** @implements <int>inverse_hash_fn */
static unsigned lowbias32_r(unsigned x) { return x; }
#endif /* < 32 bits */
/** @implements <int>to_string_fn */
static void int_to_string(const unsigned x, char (*const a)[12])
	{ sprintf(*a, "%u", x); }
#define SET_NAME int
#define SET_KEY unsigned /* Parameter of <fn:lowbias32>. */
#define SET_UINT unsigned /* Return key of <fn:lowbias32>. */
#define SET_HASH &lowbias32
#define SET_INVERSE &lowbias32_r /* Invertible means no key storage at all. */
#define SET_TEST
#define SET_EXPECT_TRAIT
#include "../src/set.h"
#define SET_TO_STRING &int_to_string
#include "../src/set.h"
/** @implements <int>test_new_fn */
static int int_from_void(void *const zero, unsigned *const u) {
	assert(!zero && RAND_MAX <= 99999999999l); /* For printing. */
	*u = (unsigned)rand();
	return 1;
}


/* Check to see that the prototypes are correct by making a signed integer. */
/** @implements <sint>hash_fn */
static unsigned sint_hash(int d) { return lowbias32((unsigned)(d - INT_MIN)); }
/** @implements <sint>inverse_hash_fn */
static int sint_inv_hash(unsigned u) { return (int)lowbias32_r(u) + INT_MIN; }
/** @implements <sint>to_string_fn */
static void sint_to_string(const int d, char (*const a)[12])
	{ sprintf(*a, "%d", d); }
#define SET_NAME sint
#define SET_KEY int
#define SET_UINT unsigned
#define SET_HASH &sint_hash
#define SET_INVERSE &sint_inv_hash
#define SET_TEST
#define SET_EXPECT_TRAIT
#include "../src/set.h"
#define SET_TO_STRING &sint_to_string
#include "../src/set.h"
/** @implements <int>test_new_fn */
static int sint_from_void(void *const zero, int *const s) {
	assert(!zero && RAND_MAX <= 9999999999l); /* For printing with '-'. */
	*s = rand() - RAND_MAX / 2;
	return 1;
}


#if 0
/* A histogram of lengths' defined as a map with the pointers to the keys
 recorded as a linked-list. */
#define NATO(X) X(Alpha), X(Bravo), X(Charlie), X(Delta), X(Echo), X(Foxtrot), \
	X(Golf), X(Hotel), X(India), X(Juliet), X(Kilo), X(Lima), X(Mike), \
	X(November), X(Oscar), X(Papa), X(Québec) /* `strlen` will report 7 */, \
	X(Romeo), X(Sierra), X(Tango), X(Uniform), X(Victor), X(Whisky), X(X-ray), \
	X(Yankee), X(Zulu)
struct nato_list { const char *alpha; struct nato_list *next; };
struct nato_value { size_t occurrences; struct nato_list *head; };
/** Being a bijection, it is common to implement both.
 @implements <nato>hash_fn, <nato>inverse_hash_fn */
static size_t nato_hash(const size_t n) { return n; }
/** @implements <nato>to_string_fn */
static void nato_to_string(const size_t s, char (*const a)[12])
	{ sprintf(*a, "%lu", (unsigned long)s); }
#define SET_NAME nato
#define SET_KEY size_t
#define SET_VALUE struct nato_value
#define SET_INVERSE &nato_hash
#define SET_HASH &nato_hash
#include "../src/set.h"
#define SET_TO_STRING &nato_to_string
#include "../src/set.h"
/** @implements <nato>compute_fn */
static void nato_count(const size_t size, const int existing,
	size_t *const count) {
	(void)size;
	if(!existing) *count = 1;
	else (*count)++;
}
static void nato(void) {
#define X(a) #a
	const char *const alphabet[] = { NATO(X) };
#undef X
	const struct nato_list list[sizeof alphabet / sizeof *alphabet];

	struct nato_set nato = SET_IDLE;
	struct nato_set_iterator it;
	size_t i;
	for(i = 0; i < sizeof alphabet / sizeof *alphabet; i++)
		nato_set_policy_put(&nato, alphabet[i].word, 0, &nato_collide);
	printf("NATO phonetic alphabet byte count histogram (~word length)\n"
		"length\tcount\twords\n");
	for(nato_set_begin(&it, &nato); nato_set_has_next(&it); ) {
		struct nato_node
			*const word = nato_upcast(nato_set_next_key(&it)), *w = word;
		printf("%lu\t%lu\t{", (unsigned long)strlen(word->word),
			(unsigned long)word->collisions + 1);
		do printf("%s%s", word == w ? "" : ",", w->word); while(w = w->next);
		printf("}\n");
	}
	nato_set_(&nato);
}

#endif






int main(void) {
	struct str16_pool strings = POOL_IDLE;
	zodiac_set_test(&fill_zodiac, 0); /* Don't require any space. */
	string_set_test(&str16_from_void, &strings), str16_pool_(&strings);
	int_set_test(&int_from_void, 0);
	sint_set_test(&sint_from_void, 0);
	//nato();














#if 0
	{ /* Automated tests. The ones that have no pool are self-contained hashs,
	 and we just test them on the stack. The ones that do require more memory
	 from a parent node, which the internals to `Set` don't know of. */
		struct string_pool strings = POOL_IDLE;
		struct boat_pool boats = POOL_IDLE;
		int_hash_test(0, 0);
		byteint_hash_test(0, 0);
		string_hash_test(&string_from_pool, &strings), string_pool_(&strings);
		vec4_hash_test(0, 0);
		id_hash_test(&id_from_pool, &boats), boat_pool_(&boats);
	}
	{ /* Boats. */
		struct boat bs[60000]; /* <- Non-trivial stack requirement. Please? */
		size_t bs_size = sizeof bs / sizeof *bs;
		struct id_hash ids = SET_IDLE;
		each_boat(bs, bs_size, &fill_boat);
		printf("Boat club races individually: ");
		print_boats(bs, bs_size);
		printf("Now adding up:\n");
		each_hash_boat(&ids, bs, bs_size, &put_in_hash);
		/*printf("Final score: %s.\n", id_hash_to_string(&ids));*/
		id_hash_(&ids);
	}
	{ /* Linked dictionary. */
		struct entry_pool buckets = POOL_IDLE;
		const size_t limit = (size_t)500000/*0<-This takes a while to hash up.*/;
		struct dict_entry *e = 0, *sp_es[20], **sp_e, **sp_e_end = sp_es,
			*const*const sp_e_lim = sp_es + sizeof sp_es / sizeof *sp_es;
		struct key_hash khash = SET_IDLE;
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
		struct key_hash khash = SET_IDLE;
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
	return EXIT_SUCCESS;
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

/* Used to test `SET_UINT`; normally `unsigned int`, here `unsigned char`.
 Useful if you want to use a specific hash length, _eg_, `C99`'s `uint32_t` or
 `uint64_t`. */

/** Fast hash function. */
static unsigned char byteint_hash(unsigned x) { return (unsigned char)x; }
/* All the same functions as above, otherwise. */
#define SET_NAME byteint
#define SET_KEY unsigned
#define SET_UINT unsigned char
#define SET_HASH &byteint_hash
#define SET_IS_EQUAL &int_is_equal
#define SET_TEST &int_fill
#define SET_EXPECT_TRAIT
#include "../src/set.h"
#define SET_TO_STRING &int_to_string
#include "../src/set.h"


/* Vector; test of `SET_POINTER`. */
struct vec4 {
	char a[2], unused[2];
	int n[2];
};
/* If we cheat a little, knowing that the numbers are 0-9, we can get a much
 more evenly distributed hash value. */
static unsigned vec4_hash(const struct vec4 *const v4) {
	return (unsigned)(1 * v4->n[0] + 10 * v4->n[1]
		+ 100 * (v4->a[0] - 'A') + 26000 * (v4->a[1] - 'a'));
}
static int vec4_is_equal(const struct vec4 *a, const struct vec4 *const b) {
	return a->a[0] == b->a[0] && a->a[1] == b->a[1]
		&& a->n[0] == b->n[0] && a->n[1] == b->n[1];
}
static void vec4_to_string(const struct vec4 *const v4, char (*const a)[12]) {
	sprintf(*a, "(%c,%c,%d,%d)",
		v4->a[0], v4->a[1], v4->n[0] % 100, v4->n[1] % 100);
}
static void vec4_filler(struct vec4 *const v4) {
	v4->a[0] = 'A' + (char)(rand() / (RAND_MAX / 26 + 1));
	v4->a[1] = 'a' + (char)(rand() / (RAND_MAX / 26 + 1));
	v4->n[0] = rand() / (RAND_MAX / 9 + 1);
	v4->n[1] = rand() / (RAND_MAX / 9 + 1);
}
#define SET_NAME vec4
#define SET_KEY struct vec4
/* <fn:vec4_hash> and <fn:vec4_is_equal> have an extra level of indirection.
 This means that we also have to get an object and fill it to use
 <fn:<S>hash_get>; not very convenient. */
#define SET_POINTER
#define SET_HASH &vec4_hash
#define SET_IS_EQUAL &vec4_is_equal
#define SET_TEST &vec4_filler
#define SET_EXPECT_TRAIT
#include "../src/set.h"
#define SET_TO_STRING &vec4_to_string
#include "../src/set.h"


/* I wrote Set to solve
 [this problem](https://stackoverflow.com/q/59091226/2472827). In general, one
 has to declare before defining if we want a hash map because the
 `<S>hashlink` is not defined until after. */

static unsigned boat_id_hash(const int id) { return (unsigned)id; }
static int boat_id_is_equal(const int a, const int b) { return a == b; }
static void boat_id_to_string(const int *const id, char (*const a)[12]);
static void fill_boat_id(int *const id);
/* Code generation for `id_hash`. */
#define SET_NAME id
#define SET_KEY int
/* Don't need two `int id; unsigned hash = id;` per datum. */
#define SET_NO_CACHE
#define SET_HASH &boat_id_hash
#define SET_IS_EQUAL &boat_id_is_equal
#define SET_TEST &fill_boat_id
#define SET_EXPECT_TRAIT
#include "../src/set.h"
#define SET_TO_STRING &boat_id_to_string
#include "../src/set.h"
struct boat {
	struct id_hashlink id;
	int best_time;
	int points;
};
/* Container of `id`. */
static struct boat *id_upcast(int *const id) {
	/* The `offhashof` are both (now) zero, see <tag:Boat>, so this could be
	 written more succinctly. */
	return (struct boat *)(void *)((char *)id - offhashof(struct boat, id)
		- offhashof(struct id_hashlink, key));
}
/* `const` container of `id`. */
static const struct boat *id_upcast_c(const int *const id) {
	/* As this: (or cast) */
	return (const struct boat *)(const void *)((const char *)id);
}
static void boat_to_string(const struct boat *const b, char (*const a)[12]) {
	/* Should be more careful about overflow? */
	sprintf(*a, "#%d(%d)", b->id.key, b->points);
}
static void boat_id_to_string(const int *const id, char (*const a)[12]) {
	boat_to_string(id_upcast_c(id), a);
}
/** <http://c-faq.com/lib/randrange.html>. Pigeon-hole principle ensures
 collisions > 89; this is good because we want `b` to be involved in several
 races. */
static void fill_boat(struct boat *const b) {
	assert(b);
	b->id.key = rand() / (RAND_MAX / 89 + 1) + 10;
	b->best_time = rand() / (RAND_MAX / 100 + 1) + 50;
	b->points = 151 - b->best_time;
}
static void fill_boat_id(int *const id) { fill_boat(id_upcast(id)); }
/* Individual races. */
static void print_boats(const struct boat *const bs, const size_t bs_size) {
	const size_t bs_eff_size = bs_size > 1000 ? 1000 : bs_size;
	char a[12];
	size_t b;
	assert(bs);
	printf("[ ");
	for(b = 0; b < bs_eff_size; b++)
		boat_to_string(bs + b, &a),
		printf("%s%s", b ? ", " : "", a);
	printf("%s]\n", bs_size > bs_eff_size ? ",…" : " ");
}
/** @implements <id>bi_predicate */
static int add_up_score(int *const original, int *const replace) {
	struct boat *const o = id_upcast(original), *const r = id_upcast(replace);
	char a[12];
	boat_to_string(o, &a);
	/*printf("Adding %d to %s.\n", r->points, a); Takes too long to print. */
	o->points += r->points;
	r->points = 0;
	if(r->best_time < o->best_time) o->best_time = r->best_time;
	return 0; /* Always false because we've sucked the points from `replace`. */
}
static void put_in_hash(struct id_hash *const hash, struct boat *const b) {
	/* Should always reserve memory first if we may be expanding the buffer for
	 error detection; otherwise it's awkward to tell. */
	if(!id_hash_reserve(hash, 1)) { perror("put_in_hash"); return; }
	id_hash_policy_put(hash, &b->id, &add_up_score);
}
static void each_boat(struct boat *const bs, const size_t bs_size,
	void (*const action)(struct boat *)) {
	size_t b;
	assert(bs);
	for(b = 0; b < bs_size; b++) action(bs + b);
}
static void each_hash_boat(struct id_hash *const ids, struct boat *const bs,
	const size_t bs_size,
	void (*const action)(struct id_hash *const, struct boat *)) {
	size_t b;
	assert(bs);
	for(b = 0; b < bs_size; b++) action(ids, bs + b);
}
/* Dynamic memory pool for storing boats, `boat_pool`. */
#define POOL_NAME boat
#define POOL_TYPE struct boat
#include "pool.h"
/** Parent-key for testing. */
static struct id_hashlink *id_from_pool(void *const vboats) {
	struct boat_pool *const boats = vboats;
	struct boat *b = boat_pool_new(boats);
	assert(boats);
	return b ? &b->id : 0;
}


/* Linked dictionary. */

static int key_is_equal(const char *const a, const char *const b) {
	return !strcmp(a, b);
}
static void key_to_string(const char *const*const ps, char (*const a)[12]) {
	strncpy(*a, *ps, sizeof(*a) - 1);
	(*a)[sizeof(*a) - 1] = '\0';
}
#define SET_NAME key
#define SET_KEY const char *
#define SET_HASH &fnv_32a_str
#define SET_IS_EQUAL &key_is_equal
#define SET_EXPECT_TRAIT
#include "../src/set.h"
#define SET_TO_STRING &key_to_string
#include "../src/set.h"

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


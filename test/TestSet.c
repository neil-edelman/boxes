/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 Testing and example of `Set`.

 @std C89/90 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h> /* strncpy */
#include "Orcish.h"



/* Simple integer set. */

/** <https://stackoverflow.com/a/12996028> */
static unsigned int int_hash(unsigned x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}
/** `a` == `b`. */
static int int_is_equal(const unsigned a, const unsigned b) { return a == b; }
/** Outputs `x` to `a`. */
static void int_to_string(const unsigned *const x, char (*const a)[12]) {
	/*snprintf(*a, sizeof *a, "%u", *x); Gah.*/
	sprintf(*a, "%u", *x); /* Assumes <= 32-bit. */
}
/** Fills `x` with random. */
static void int_fill(unsigned *const x) { *x = rand(); }
/** This defines `struct IntSet` and `struct IntSetElement`. */
#define SET_NAME Int
#define SET_TYPE unsigned
#define SET_HASH &int_hash
#define SET_IS_EQUAL &int_is_equal
#define SET_TO_STRING &int_to_string
#define SET_TEST &int_fill
#include "../src/Set.h"



/* Used to test `SET_HASH_TYPE`. */

/** This is probably not the greatest hash function. */
static unsigned char charint_hash(unsigned x) { return x; }
/** This defines `struct IntSet` and `struct IntSetElement`. */
#define SET_NAME CharInt
#define SET_TYPE unsigned
#define SET_HASH_TYPE unsigned char
#define SET_HASH &charint_hash
#define SET_IS_EQUAL &int_is_equal
#define SET_TO_STRING &int_to_string
#define SET_TEST &int_fill
#include "../src/Set.h"



/* String set (with support in dynamically generated pool.) */

/** Perform a 32 bit
 [Fowler/Noll/Vo FNV-1a](http://www.isthe.com/chongo/tech/comp/fnv/) hash on a
 string, modified to `unsigned`. */
static unsigned fnv_32a_str(char *const str) {
	const unsigned char *s = (const unsigned char *)str;
	/* 32 bit FNV-1 and FNV-1a non-zero initial basis, `FNV1_32A_INIT`. */
	unsigned hval = 0x811c9dc5;
	/* FNV magic prime `FNV_32_PRIME 0x01000193`. */
	while(*s) {
		hval ^= *s++;
		hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
	}
	return hval;
}
/** `a` == `b` */
static int string_is_equal(char *const a, char *const b) {
	return !strcmp(a, b);
}
/** Copies `s` to `a`. */
static void string_to_string(char *const*const s, char (*const a)[12]) {
	strncpy(*a, *s, sizeof(*a) - 1);
	(*a)[sizeof(*a) - 1] = '\0';
}
#define STRBUF 12
/** Automatic random naming for test. */
static void string_fill(char **const pstr) {
	Orcish(*pstr, STRBUF);
}
#define SET_NAME String
#define SET_TYPE char *
#define SET_HASH &fnv_32a_str
#define SET_IS_EQUAL &string_is_equal
#define SET_TO_STRING &string_to_string
#define SET_TEST &string_fill
#include "../src/Set.h"
/* Auto-naming details. */
struct StringElement {
	struct StringSetElement sse;
	char buffer[STRBUF];
};
#define POOL_NAME StringElement
#define POOL_TYPE struct StringElement
#include "Pool.h"
/** This is to store the strings. */
static struct StringSetElement *sse_from_pool(void *const vses) {
	struct StringElementPool *const ses = vses;
	struct StringElement *const se = StringElementPoolNew(ses);
	assert(ses);
	if(!se) return 0;
	/* This is `MAX_STRING` buffer; <fn:string_fill> will read this value. */
	se->sse.data = se->buffer;
	return &se->sse;
}



/* Vector; used to test `SET_PASS_POINTER`. */

struct Vec4 {
	char a[2];
	int n[2];
};
static unsigned vec4_hash(const struct Vec4 *const v4) {
	/* Cheat a little knowing that the numbers are 0-9. */
	return 1 * v4->n[0] + 10 * v4->n[1]
		+ 100 * (v4->a[0] - 'A') + 26000 * (v4->a[1] - 'a');
}
static int vec4_is_equal(const struct Vec4 *a, const struct Vec4 *const b) {
	return a->a[0] == b->a[0] && a->a[1] == b->a[1]
		&& a->n[0] == b->n[0] && a->n[1] == b->n[1];
}
static void vec4_to_string(const struct Vec4 *const v4, char (*const a)[12]) {
	sprintf(*a, "(%c,%c,%d,%d)",
		v4->a[0], v4->a[1], v4->n[0] % 100, v4->n[1] % 100);
}
static void vec4_filler(struct Vec4 *const v4) {
	v4->a[0] = rand() / (RAND_MAX / 26 + 1) + 'A';
	v4->a[1] = rand() / (RAND_MAX / 26 + 1) + 'a';
	v4->n[0] = rand() / (RAND_MAX / 9 + 1);
	v4->n[1] = rand() / (RAND_MAX / 9 + 1);
}
#define SET_NAME Vec4
#define SET_TYPE struct Vec4
#define SET_PASS_POINTER
#define SET_HASH &vec4_hash
#define SET_IS_EQUAL &vec4_is_equal
#define SET_TO_STRING &vec4_to_string
#define SET_TEST &vec4_filler
#include "../src/Set.h"



/* Boats; example of a hash map to solve
 [this problem](https://stackoverflow.com/q/59091226/2472827). In general, one
 has to declare before defining if one wants a hash map because the
 `<E>SetElement` is not defined until after, (or one could cast the first
 element.) */

static unsigned boat_id_hash(const int id) { return id; }
static int boat_id_is_equal(const int a, const int b) { return a == b; }
static void boat_id_to_string(const int *const id, char (*const a)[12]);
static void boat_id_filler(int *const id);
/* Code generation for `IdSet`;
 we are responsible for storing `IdSetElement`. */
#define SET_NAME Id
#define SET_TYPE int
#define SET_HASH &boat_id_hash
/* Don't need two `int id; unsigned hash = id;` per datum. */
#define SET_NO_CACHE
#define SET_IS_EQUAL &boat_id_is_equal
#define SET_TO_STRING &boat_id_to_string
#define SET_TEST &boat_id_filler
#include "../src/Set.h"
/* Here is where we store it. */
struct Boat {
	struct IdSetElement id;
	int best_time;
	int points;
};
/* `container_of(id.data)`. */
static struct Boat *id_upcast(int *const id) {
	return (struct Boat *)(void *)((char *)id - offsetof(struct Boat, id)
		- offsetof(struct IdSetElement, data));
}
/* `const container_of(id.data)`. */
static const struct Boat *id_const_upcast(const int *const id) {
	return (const struct Boat *)(const void *)
		((const char *)id - offsetof(struct Boat, id)
		- offsetof(struct IdSetElement, data));
}
static void boat_to_string(const struct Boat *const b, char (*const a)[12]) {
	sprintf(*a, "#%d(%d)", b->id.data, b->points);
}
static void boat_id_to_string(const int *const id, char (*const a)[12]) {
	boat_to_string(id_const_upcast(id), a);
}
/** <http://c-faq.com/lib/randrange.html>. Pigeon-hole principle ensures
 collisions > 90; this is good because we want them to be involved in
 several races. */
static void fill_boat(struct Boat *const b) {
	assert(b);
	b->id.data = rand() / (RAND_MAX / 89 + 1) + 10;
    b->best_time = rand() / (RAND_MAX / 100 + 1) + 50;
    b->points = 151 - b->best_time;
}
static void boat_id_filler(int *const id) {
	fill_boat(id_upcast(id));
}
/* Individual races. */
static void print_boats(const struct Boat *const bs,
	const size_t bs_size) {
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
/** @implements <Id>Replace */
static int add_up_score(int *const original, int *const replace) {
	struct Boat *const o = id_upcast(original), *const r = id_upcast(replace);
	char a[12];
	boat_to_string(o, &a);
	/*printf("Adding %d to %s.\n", r->points, a); Takes too long to print. */
	o->points += r->points;
	r->points = 0;
	if(r->best_time < o->best_time) o->best_time = r->best_time;
	return 0; /* Always false because we've sucked the points from `replace`. */
}
static void put_in_set(struct IdSet *const set, struct Boat *const b) {
	if(!IdSetReserve(set, 1)) { perror("put_in_set"); return; }
	IdSetPolicyPut(set, &b->id, &add_up_score);
}
static void each_boat(struct Boat *const bs, const size_t bs_size,
	void (*const action)(struct Boat *)) {
	size_t b;
	assert(bs);
	for(b = 0; b < bs_size; b++) action(bs + b);
}
static void each_set_boat(struct IdSet *const ids, struct Boat *const bs,
	const size_t bs_size,
	void (*const action)(struct IdSet *const, struct Boat *)) {
	size_t b;
	assert(bs);
	for(b = 0; b < bs_size; b++) action(ids, bs + b);
}
/* Dynamic memory pool for storing boats, `BoatPool`. */
#define POOL_NAME Boat
#define POOL_TYPE struct Boat
#include "Pool.h"
/** Parent-type for testing. */
static struct IdSetElement *id_from_pool(void *const vboats) {
	struct BoatPool *const boats = vboats;
	struct Boat *b = BoatPoolNew(boats);
	assert(boats);
	return b ? &b->id : 0;
}



/* Linked dictionary. */

#define SET_NAME Word
#define SET_TYPE char *
#define SET_HASH &fnv_32a_str
#define SET_IS_EQUAL &string_is_equal
#define SET_TO_STRING &string_to_string
#include "../src/Set.h"

struct WordListNode;
static int word_compare(const struct WordListNode *,
	const struct WordListNode *);
#define LIST_NAME Word
#define LIST_COMPARE &word_compare
#include "List.h"

struct Entry {
	struct WordSetElement set;
	struct WordListNode list;
	char key[24];
	char value[32];
};

/* `const` container of `list`. */
static const struct Entry *list_upcast_c(const struct WordListNode *const list)
{
	return (const struct Entry *)(const void *)((const char *)list
		- offsetof(struct Entry, list));
}
/* `const` container of `set`. */
static struct Entry *set_upcast(struct WordSetElement *const elem) {
	return (struct Entry *)(void *)((char *)elem - offsetof(struct Entry, set));
}
/** @implements <WordListNode>Compare */
static int word_compare(const struct WordListNode *const a,
	const struct WordListNode *const b) {
	return strcmp(list_upcast_c(a)->key, list_upcast_c(b)->key);
}
static void entry_fill(struct Entry *const e) {
	assert(e);
	e->set.data = e->key;
	Orcish(e->key, sizeof e->key);
	Orcish(e->value, sizeof e->value);
}
static const struct Entry *entry_prev(struct Entry *const e) {
	const struct WordListNode *const prev = WordListPrevious(&e->list);
	assert(e);
	return prev ? list_upcast_c(prev) : 0;
}
static const struct Entry *entry_next(struct Entry *const e) {
	const struct WordListNode *const next = WordListNext(&e->list);
	assert(e);
	return next ? list_upcast_c(next) : 0;
}

#define POOL_NAME Entry
#define POOL_TYPE struct Entry
#include "Pool.h"

int main(void) {
	{ /* Automated tests. */
		struct BoatPool boats;
		struct StringElementPool ses;

		IntSetTest(0, 0);
		CharIntSetTest(0, 0);
		StringElementPool(&ses), StringSetTest(&sse_from_pool, &ses),
			StringElementPool_(&ses);
		Vec4SetTest(0, 0);
		BoatPool(&boats), IdSetTest(&id_from_pool, &boats), BoatPool_(&boats);
	}
	{ /* Boats. */
		struct Boat bs[60000]; /* <- Non-trivial stack requirement. Please? */
		size_t bs_size = sizeof bs / sizeof *bs;
		struct IdSet ids = SET_ZERO;
		each_boat(bs, bs_size, &fill_boat);
		printf("Boat club races individually: ");
		print_boats(bs, bs_size);
		printf("Now adding up:\n");
		each_set_boat(&ids, bs, bs_size, &put_in_set);
		printf("Final score: %s.\n", IdSetToString(&ids));
		IdSet_(&ids);
	}
	{ /* Linked dictionary. */
		struct EntryPool entries = POOL_IDLE;
		const size_t limit = (size_t)500000/*0*/;
		struct Entry *e, *sp_es[10], **sp_e, **sp_e_end = sp_es,
			*const*const sp_e_lim = sp_es + sizeof sp_es / sizeof *sp_es;
		struct WordSet word_set = SET_ZERO;
		struct WordList word_list;
		struct WordSetElement *elem;
		struct Entry *found;
		size_t i, unique = 0;
		int is_used = 1;
		WordListClear(&word_list);
		for(i = 0; i < limit; i++) {
			if(is_used && !(e = EntryPoolNew(&entries)))
				{ perror("Memory error"); assert(0); return EXIT_FAILURE; }
			entry_fill(e);
			if(WordSetGet(&word_set, e->key))
				{ printf("Already %s.\n", e->key); is_used = 0; continue; }
			is_used = 1;
			unique++;
			if(!WordSetReserve(&word_set, 1))
				{ perror("Memory error"); assert(0); return EXIT_FAILURE; }
			elem = WordSetPut(&word_set, &e->set), assert(!elem);
			WordListPush(&word_list, &e->list);
			if(sp_e_end >= sp_e_lim) continue;
			printf("Looking for %s.\n", e->key);
			*(sp_e_end++) = e;
		}
		printf("Sorting %lu elements.\n", (unsigned long)unique);
		WordListSort(&word_list);
		for(sp_e = sp_es; sp_e < sp_e_end; sp_e++) {
			const struct Entry *prev, *next;
			elem = WordSetGet(&word_set, (*sp_e)->key);
			assert(elem);
			found = set_upcast(elem);
			prev = entry_prev(found);
			next = entry_next(found);
			printf("The found element was …%s, %s, %s…\n",
				prev ? prev->key : "start", found->key,
				next ? next->key : "end");
			assert(found == *sp_e);
		}
		WordSet_(&word_set);
	}

	return EXIT_SUCCESS;
}

/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 Testing and example of `Set` based on a `Stackoverflow` question.

 @std C89/90 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h> /* strncpy */
#include "Orcish.h"

/** <https://stackoverflow.com/a/12996028> */
static unsigned int hash_int(unsigned int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}
/** `a` == `b`. */
static int equal_ints(const unsigned a, const unsigned b) { return a == b; }
/** Outputs `x` to `a`. */
static void int_to_string(const unsigned *const x, char (*const a)[12]) {
	snprintf(*a, sizeof *a, "%u", *x);
}
/** Fills `x` with random. */
static void fill_int(unsigned *const x) { *x = rand(); }
/** This defines `struct IntSet` and `struct IntSetElement`. */
#define SET_NAME Int
#define SET_TYPE unsigned
#define SET_HASH &hash_int
#define SET_IS_EQUAL &equal_ints
#define SET_TO_STRING &int_to_string
#define SET_TEST &fill_int
#include "../src/Set.h"

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
static void string_to_string(char *const*const s, char (*const a)[12]) {
	strncpy(*a, *s, sizeof(*a) - 1);
	(*a)[sizeof(*a) - 1] = '\0';
}
static int string_is_equal(char *const a, char *const b) {
	return !strcmp(a, b);
}
#define MAX_STRING 12
static void string_fill(char **const pstr) {
	Orcish(*pstr, MAX_STRING);
}
#define SET_NAME String
#define SET_TYPE char *
#define SET_HASH &fnv_32a_str
#define SET_IS_EQUAL &string_is_equal
#define SET_TO_STRING &string_to_string
#define SET_TEST &string_fill
#include "../src/Set.h"
struct StringElement {
	struct StringSetElement sse;
	char buffer[MAX_STRING];
};
#define POOL_NAME StringElement
#define POOL_TYPE struct StringElement
#include "Pool.h"
/** This is to store the strings. We could just have string constants, but we
 want to automate testing. */
static struct StringSetElement *sse_from_pool(void *const vses) {
	struct StringElementPool *const ses = vses;
	struct StringElement *const se = StringElementPoolNew(ses);
	assert(ses);
	if(!se) return 0;
	/* This is `MAX_STRING` buffer; <fn:string_fill> will read this value. */
	se->sse.data = se->buffer;
	return &se->sse;
}

struct Vec4 {
	char a[2];
	int n[2];
};
static unsigned vec4_hash(const struct Vec4 *const v4) {
	/* Cheat a little knowing that the numbers are 0-9. */
	return 1 * v4->n[0] + 10 * v4->n[1]
		+ 100 * (v4->a[0] - 'A') + 26000 * (v4->a[1] - 'a');
}
static int vec4_is_equal(const struct Vec4 *a, const struct Vec4 *const b)
{
	return a->a[0] == b->a[0] && a->a[1] == b->a[1]
		&& a->n[0] == b->n[0] && a->n[1] == b->n[1];
}
static void vec4_to_string(const struct Vec4 *const v4, char (*const a)[12]) {
	snprintf(*a, sizeof *a, "(%c,%c,%d,%d)",
		v4->a[0], v4->a[1], v4->n[0], v4->n[1]);
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

static unsigned boat_id_hash(const int id) { return id; }
static int boat_id_is_equal(const int a, const int b) { return a == b; }
static void boat_id_to_string(const int *const id, char (*const a)[12]);
static void boat_id_filler(int *const id);
/* Code generation for `IdSet`;
 we are responsible for storing `IdSetElement`. */
#define SET_NAME Id
#define SET_TYPE int
#define SET_HASH &boat_id_hash
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
static void fill_boat(struct Boat *const b) {
	assert(b);
	/* <http://c-faq.com/lib/randrange.html>. PHP ensures collisions > 900. */
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

int main(void) {
	{ /* Automated tests. */
		struct BoatPool boats;
		struct StringElementPool ses;

		IntSetTest(0, 0);
		StringElementPool(&ses), StringSetTest(&sse_from_pool, &ses),
			StringElementPool_(&ses);
		Vec4SetTest(0, 0);
		BoatPool(&boats), IdSetTest(&id_from_pool, &boats), BoatPool_(&boats);
	}
	{ /* Not as automated tests. */
		struct Boat bs[60000]; /* <- Non-trivial stack requirement. */
		size_t bs_size = sizeof bs / sizeof *bs;
		struct IdSet ids = SET_ZERO;

		each_boat(bs, bs_size, &fill_boat);
		printf("Boat club races individually: ");
		print_boats(bs, bs_size);
		printf("Now adding up:\n");
		each_set_boat(&ids, bs, bs_size, &put_in_set);
		printf("Final score: %s.\n", IdSetToString(&ids));
	}

	return EXIT_SUCCESS;
}

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
#define SET_EQUAL &equal_ints
#define SET_TO_STRING &int_to_string
#define SET_TEST &fill_int
#include "../src/Set.h"

struct String { char s64[64]; };
/** Perform a 32 bit
 [Fowler/Noll/Vo FNV-1a](http://www.isthe.com/chongo/tech/comp/fnv/) hash on a
 string, modified to `unsigned`. */
static unsigned fnv_32a_str(const char *str) {
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
static void string_to_string(const char *const str, char (*const a)[12]) {
	strncpy(*a, str, sizeof(*a));
}
#define ARRAY_NAME String
#define ARRAY_TYPE struct String
#include "Array.h"
static struct StringArray strings;
static void string_fill(const char *str) {
	struct String *s64;
	if(!(s64 = StringArrayNew(&strings)))
		{ perror("string64"); exit(EXIT_FAILURE); return; }
	Orcish(s64->s64, sizeof s64->s64);
	str = s64->s64;
}
#define SET_NAME String
#define SET_TYPE const char *
#define SET_HASH &fnv_32a_str
#define SET_EQUAL &strcmp
#define SET_TO_STRING &string_to_string
#define SET_TEST &string_fill
#include "../src/Set.h"


#if 0
static unsigned id_hash(const int id) { return id; }
static int id_is_equal(const int a, const int b) { return a == b; }

/* Prototype these for <fn:id_to_string>. */
struct Boat;
static struct Boat *id_upcast(int *);
static const struct Boat *id_constupcast(const int *);
static void fill_boat(struct Boat *);
static void boat_to_string(const struct Boat *, char (*)[12]);

static void id_filler(int *const id) { fill_boat(id_upcast(id)); }
static void id_to_string(const int *const id, char (*const a)[12]) {
	boat_to_string(id_constupcast(id), a);
}

/* Code generation for `IdSet`;
 we are responsible for storing `IdSetKey`. */
#define SET_NAME Id
#define SET_TYPE int
#define SET_HASH &id_hash
#define SET_NO_CACHE
#define SET_IS_EQUAL &id_is_equal
#define SET_TO_STRING &id_to_string
#define SET_TEST &id_filler
#include "../src/Set.h"

/* Here is where we store it. */
struct Boat {
	struct IdSetKey id;
	int best_time;
	int points;
};

/* `container_of(id)`. */
static struct Boat *id_upcast(int *const id) {
	return (struct Boat *)(void *)((char *)id - offsetof(struct Boat, id));
}
/* `container_of(id)`. */
static const struct Boat *id_constupcast(const int *const id) {
	return (const struct Boat *)(const void *)
		((const char *)id - offsetof(struct Boat, id));
}

static void boat_to_string(const struct Boat *const b, char (*const a)[12]) {
	sprintf(*a, "#%d(%d)", b->id.data, b->points);
}

static void fill_boat(struct Boat *const b) {
	assert(b);
	/* <http://c-faq.com/lib/randrange.html>. PHP ensures collisions > 30. */
    b->id.data = rand() / (RAND_MAX / 30 + 1) + 1;
    b->best_time = rand() / (RAND_MAX / 100 + 1) + 50;
    b->points = 151 - b->best_time;
}

/* Individual races. */
static void print_boats(const struct Boat *const bs,
	const size_t bs_size) {
	char a[12];
	size_t b;
	assert(bs);
	printf("In array: [ ");
	for(b = 0; b < bs_size; b++)
		boat_to_string(bs + b, &a),
		printf("%s%s", b ? ", " : "", a);
	printf(" ]\n");
}

/** @implements <Id>Replace */
static int add_up_score(int *const original, int *const replace) {
	struct Boat *const o = id_upcast(original), *const r = id_upcast(replace);
	o->points += r->points;
	r->points = 0;
	if(r->best_time < o->best_time) o->best_time = r->best_time;
	return 0; /* Always false because we've sucked the points from `r`. */
}

static void put_in_set(struct IdSet *const set, struct Boat *const b) {
	if(!IdSetReserve(set, 1)) { perror("put_in_set"); return; }
	IdSetPutResolve(set, &b->id, &add_up_score);
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
#endif

int main(void) {
#if 0
	struct Boat bs[32];
	size_t bs_size = sizeof bs / sizeof *bs;
	/*struct IdSet set = SET_ZERO;*/
	IdSetTest(bs, bs_size, sizeof *bs, offsetof(struct Boat, id));
	/*each_boat(bs, bs_size, &fill_boat);
	print_boats(bs, bs_size);
	each_set_boat(&set, bs, bs_size, &put_in_set);
	printf("Final score: %s.\n", IdSetToString(&set));*/
#endif
	IntSetTest();
	/*StringSetTest();
	StringArray_(&strings);*/
	return EXIT_SUCCESS;
}

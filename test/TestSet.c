/** @license 2019 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 Testing and example of `Set` based on a `Stackoverflow` question.

 @std C89/90 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

static unsigned id_hash(const int id) { return id; }
static int id_is_equal(const int a, const int b) { return a == b; }

/* Prototype these for <fn:id_to_string>. */
struct Boat;
static const struct Boat *id_const_upcast(const int *const);
static void boat_to_string(const struct Boat *const b, char (*const a)[12]);
static void id_to_string(const int *const id, char (*const a)[12]) {
	boat_to_string(id_const_upcast(id), a);
}

/* Code generation for `IdSet`; we are responsible for storing `IdSetItem`. */
#define SET_NAME Id
#define SET_TYPE int
#define SET_HASH &id_hash
#define SET_IS_EQUAL &id_is_equal
#define SET_TO_STRING &id_to_string
#define SET_NO_CACHE
#include "../src/Set.h"

/* Here is where we store it. */
struct Boat {
	struct IdSetItem id;
	int best_time;
	int points;
};

/* `container_of(id)`. */
static const struct Boat *id_const_upcast(const int *const id) {
	return (const struct Boat *)(const void *)
		((const char *)id - offsetof(struct Boat, id));
}

/* `container_of(id)`. */
static struct Boat *id_upcast(int *const id) {
	return (struct Boat *)(void *)((char *)id - offsetof(struct Boat, id));
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
	printf("In array: { ");
	for(b = 0; b < bs_size; b++)
		boat_to_string(bs + b, &a),
		printf("%s%s", b ? ", " : "", a);
	printf(" }\n");
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

int main(void) {
	struct Boat bs[32];
	size_t bs_size = sizeof bs / sizeof *bs;
	struct IdSet set = SET_ZERO;
	each_boat(bs, bs_size, &fill_boat);
	print_boats(bs, bs_size);
	each_set_boat(&set, bs, bs_size, &put_in_set);
	printf("Final score: %s.\n", IdSetToString(&set));
	return EXIT_SUCCESS;
}

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

static void boat_id_to_string(const int *const id, char (*const a)[12]);

#define SET_NAME Id
#define SET_TYPE int
#define SET_HASH &id_hash
#define SET_IS_EQUAL &id_is_equal
#define SET_TO_STRING &boat_id_to_string
#include "../src/Set.h"

struct Boat {
	struct IdSetItem id;
	int time;
	int points;
};

static struct Boat *boat_id_upcast(int *const id) {
	return (struct Boat *)(void *)((char *)id) - offsetof(struct Boat, id);
}

static const struct Boat *boat_id_const_upcast(const int *const id) {
	return (const struct Boat *)(const void *)
		((const char *)id - offsetof(struct Boat, id));
}

static void boat_id_to_string(const int *const id, char (*const a)[12]) {
	const struct Boat *const b = boat_id_const_upcast(id);
	sprintf(*a, "#%d(%d)", *IdSetItem(&b->id), b->points);
}

static void fill_boat(struct Boat *const b) {
	assert(b);
	/* <http://c-faq.com/lib/randrange.html>. PHP ensures collisions > 30. */
    *IdSetVariableItem(&b->id) = rand() / (RAND_MAX / 30 + 1) + 1;
    b->time = rand() / (RAND_MAX / 100 + 1) + 50;
    b->points = 151 - b->time;
}

static void print_boats(const struct Boat *const bs,
	const size_t bs_size) {
	char a[12];
	size_t b;
	assert(bs);
	printf("{ ");
	for(b = 0; b < bs_size; b++)
		boat_id_to_string(IdSetItem(&bs[b].id), &a), printf("%s%s", b ? ", " : "", a);
	printf(" }\n");
}

static void each_boat(struct Boat *const bs, const size_t bs_size,
	void (*const action)(struct Boat *)) {
	size_t b;
	assert(bs);
	for(b = 0; b < bs_size; b++) action(bs + b);
}

static void put_in_set(struct Boat *const b, struct IdSet *const set) {
	int is_put;
	char a[12];
	assert(b && set);
	IdSetPutIfAbsent(set, &b->id, &is_put);
	if(is_put) return;
	boat_id_to_string(IdSetItem(&b->id), &a);
	printf("There's already a %s in %s.\n", a, IdSetToString(set));
}

static void each_boat_param(struct Boat *const bs, const size_t bs_size,
	struct IdSet *const ids,
	void (*const consumer)(struct Boat *, struct IdSet *const)) {
	size_t b;
	assert(bs);
	for(b = 0; b < bs_size; b++) consumer(bs + b, ids);
}

int main(void) {
	struct Boat bs[32];
	size_t bs_size = sizeof bs / sizeof *bs;
	struct IdSet set = SET_ZERO;
	each_boat(bs, bs_size, &fill_boat);
	print_boats(bs, bs_size);
	each_boat_param(bs, bs_size, &set, &put_in_set);
	printf("Set: %s.\n", IdSetToString(&set));
	return EXIT_SUCCESS;
}

#if 0

int main(void) {
	struct BoatEntry boats[12], *discard;
	const size_t boats_size = sizeof boats / sizeof *boats;
	size_t i;
	struct BoatMap map;
	each_boat(boats, boats_size, &fill);
	print_boats(boats, boats_size);
	BoatMap(&map);
	for(i = 0; i < boats_size; i++) {
		BoatMapPut(&map, boats + i, &discard);
		if(discard) {
			char a[12], b[12];
			boat_to_string(boats + i, &a);
			boat_to_string(discard, &b);
			printf("%s dispaced by %s at %lu.\n", b, a, (unsigned long)i);
		}
	}
	printf("%s\n", BoatMapToString(&map));
	/*print_boats(boats, boats_size);
	 if(!(set = hashset_create())) return perror("set"), EXIT_FAILURE;
	 each_set_boat(set, boats, boats_size, &put);*/
	return EXIT_SUCCESS;
}
#endif

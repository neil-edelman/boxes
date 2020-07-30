/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 Test Heap.

 @std C89/90 */

#include <stdlib.h> /* EXIT malloc free rand */
#include <stdio.h>  /* *printf */
#include "Orcish.h"


struct int_heap_node;
static void int_to_string(const struct int_heap_node *, char (*)[12]);
static void test_int(struct int_heap_node *, void *);

#define HEAP_NAME int
#define HEAP_EXPECT_TRAIT
#include "../src/Heap.h"
#define HEAP_TO_STRING &int_to_string
#define HEAP_TEST &test_int
#include "../src/Heap.h"

static void int_to_string(const struct int_heap_node *const i,
	char (*const a)[12]) {
	sprintf(*a, "%u", i->priority);
}

static void test_int(struct int_heap_node *i, void *const unused) {
	(void)(unused);
	i->priority = rand() / (RAND_MAX / 99 + 1) + 1;
	/* printf("test_int: generated %u\n", i->priority); */
}


struct orc_heap_node;
static void orc_to_string(const struct orc_heap_node *, char (*)[12]);
static void test_orc(struct orc_heap_node *, void *);

#define HEAP_NAME orc
#define HEAP_VALUE struct Orc
#define HEAP_EXPECT_TRAIT
#include "../src/Heap.h"
#define HEAP_TO_STRING &orc_to_string
#define HEAP_TEST &test_orc
#include "../src/Heap.h"

struct Orc { unsigned health; char name[10]; };

static void orc_to_string(const struct orc_heap_node *const node,
	char (*const a)[12]) {
	sprintf(*a, "%u%.9s", node->priority, node->value->name);
}

#define POOL_NAME orc
#define POOL_TYPE struct Orc
#include "Pool.h"

static void test_orc(struct orc_heap_node *node, void *const vpool) {
	struct Orc *orc = orc_pool_new(vpool);
	if(!orc) { assert(0); exit(EXIT_FAILURE); }
	orc->health = rand() / (RAND_MAX / 99 + 1);
	Orcish(orc->name, sizeof orc->name);
	node->priority = orc->health;
	node->value = orc;
}


int main(void) {
	struct orc_pool orcs = POOL_IDLE;
	rand();
	int_heap_test(0);
	orc_heap_test(&orcs), orc_pool_(&orcs);
	return EXIT_SUCCESS;
}

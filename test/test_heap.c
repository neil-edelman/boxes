/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 Test Heap.

 @std C89/90 */

#include <stdlib.h> /* EXIT malloc free rand */
#include <stdio.h>  /* *printf */
#include "orcish.h"


/* Minimal heap. This selects the default `unsigned int`. */
#define HEAP_NAME min
#include "../src/heap.h"


/* Also an `unsigned int`, but with testing. */
static void int_to_string(const unsigned *const i, char (*const z)[12])
	{ sprintf(*z, "%u", *i); }
static void test_int(unsigned *const i, void *const unused) {
	(void)(unused);
	*i = (unsigned)rand() / (RAND_MAX / 99 + 1) + 1;
}
#define HEAP_NAME int
#define HEAP_TEST &test_int
#define HEAP_EXPECT_TRAIT
#include "../src/heap.h"
#define HEAP_TO_STRING &int_to_string
#include "../src/heap.h"


/* A value pointer along with a priority; we have to have some place to put
 them, so we use a pool. */
struct orc { unsigned health; char name[12]; };
static void orc_to_string(const struct orc *const orc, char (*const a)[12])
	{ sprintf(*a, "%u%.9s", orc->health, orc->name); }
/* Testing requires that we forward-declare. */
struct orc_heapnode;
static void test_orcnode(struct orc_heapnode *, void *);
#define HEAP_NAME orc
#define HEAP_VALUE struct orc
#define HEAP_TEST &test_orcnode
#define HEAP_EXPECT_TRAIT
#include "../src/heap.h"
#define HEAP_TO_STRING &orc_to_string
#include "../src/heap.h"
#define POOL_NAME orc
#define POOL_TYPE struct orc
#include "pool.h"
/*static void orc_hn_to_string(const struct orc_heap_node *const node,
	char (*const a)[12]) { orc_to_string(node->value, a); }*/
static void test_orcnode(struct orc_heapnode *node, void *const vpool) {
	struct orc *orc = orc_pool_new(vpool);
	if(!orc) { assert(0); exit(EXIT_FAILURE); }
	orc->health = (unsigned)rand() / (RAND_MAX / 99 + 1);
	orcish(orc->name, sizeof orc->name);
	node->priority = orc->health;
	node->value = orc;
}


static void index_to_string(const size_t *const i, char (*const a)[12]) {
	sprintf(*a, "%lu", (unsigned long)*i);
}
static void test_index(size_t *const i, void *const unused) {
	(void)(unused);
	*i = (unsigned)rand();
}
static int index_compare(const size_t a, const size_t b) { return a < b; }
#define HEAP_NAME index
#define HEAP_TYPE size_t
#define HEAP_COMPARE &index_compare
#define HEAP_TEST &test_index
#define HEAP_EXPECT_TRAIT
#include "../src/heap.h"
#define HEAP_TO_STRING &index_to_string
#include "../src/heap.h"


int main(void) {
	struct orc_pool orcs = POOL_IDLE;
	rand();
	int_heap_test(0);
	orc_heap_test(&orcs), orc_pool_(&orcs);
	index_heap_test(0);
	return EXIT_SUCCESS;
}

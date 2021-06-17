/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 Test Heap.

 @std C89/90 */

#include <stdlib.h> /* EXIT malloc free rand */
#include <stdio.h>  /* *printf */
#include "orcish.h"

static void int_to_string(const int i, char (*const z)[12])
	{ sprintf(*z, "%u", i); }
static void test_int(int *const i, void *const unused) {
	(void)(unused);
	*i = (unsigned)rand() / (RAND_MAX / 99 + 1) + 1;
}
#define HEAP_NAME int
#define HEAP_EXPECT_TRAIT
#include "../src/heap.h"
#define HEAP_TO_STRING &int_to_string
#define HEAP_TEST &test_int
#include "../src/heap.h"

/* ...

struct orc_heap_node;
static void orc_to_string(const struct orc_heap_node *, char (*)[12]);
static void test_orc(struct orc_heap_node *, void *);

#define HEAP_NAME orc
#define HEAP_VALUE struct orc
#define HEAP_EXPECT_TRAIT
#include "../src/heap.h"
#define HEAP_TO_STRING &orc_to_string
#define HEAP_TEST &test_orc
#include "../src/heap.h"

struct orc { unsigned health; char name[10]; };

static void orc_to_string(const struct orc_heap_node *const node,
	char (*const a)[12]) {
	sprintf(*a, "%u%.9s", node->priority, node->value->name);
}

#define POOL_NAME orc
#define POOL_TYPE struct orc
#include "pool.h"

static void test_orc(struct orc_heap_node *node, void *const vpool) {
	struct orc *orc = orc_pool_new(vpool);
	if(!orc) { assert(0); exit(EXIT_FAILURE); }
	orc->health = (unsigned)rand() / (RAND_MAX / 99 + 1);
	orcish(orc->name, sizeof orc->name);
	node->priority = orc->health;
	node->value = orc;
}


struct index_heap_node;
static void index_to_string(const struct index_heap_node *, char (*)[12]);
static void test_index(struct index_heap_node *, void *);
static int index_compare(const size_t a, const size_t b) { return a < b; }

#define HEAP_NAME index
#define HEAP_TYPE size_t
#define HEAP_COMPARE &index_compare
#define HEAP_EXPECT_TRAIT
#include "../src/heap.h"
#define HEAP_TO_STRING &index_to_string
#define HEAP_TEST &test_index
#include "../src/heap.h"

static void index_to_string(const struct index_heap_node *const i,
	char (*const a)[12]) {
	sprintf(*a, "%lu", (unsigned long)i->priority);
}
static void test_index(struct index_heap_node *i, void *const unused) {
	(void)(unused);
	i->priority = (unsigned)rand();
}

*/

int main(void) {
	/*struct orc_pool orcs = POOL_IDLE;*/
	rand();
	int_heap_test(0);
	/*orc_heap_test(&orcs), orc_pool_(&orcs);
	index_heap_test(0);*/
	return EXIT_SUCCESS;
}

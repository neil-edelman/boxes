/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 Test Heap.

 @std C89/90 */

#include <stdlib.h> /* EXIT malloc free rand */
#include <stdio.h>  /* *printf */
/*#include <assert.h>*/ /* assert */
#include <errno.h>  /* errno */
#include "Orcish.h"


struct IntHeapNode;
static void int_to_string(const struct IntHeapNode *, char (*)[12]);
static void test_int(struct IntHeapNode *, void *);

#define HEAP_NAME Int
#define HEAP_TO_STRING &int_to_string
#define HEAP_TEST &test_int
#include "../src/Heap.h"

static void int_to_string(const struct IntHeapNode *const i,
	char (*const a)[12]) {
	sprintf(*a, "%u", i->priority);
}

static void test_int(struct IntHeapNode *i, void *const unused) {
	(void)(unused);
	i->priority = rand() / (RAND_MAX / 99 + 1) + 1;
	/* printf("test_int: generated %u\n", i->priority); */
}


struct OrcHeapNode;
static void orc_to_string(const struct OrcHeapNode *, char (*)[12]);
static void test_orc(struct OrcHeapNode *, void *);

#define HEAP_NAME Orc
#define HEAP_VALUE struct Orc
#define HEAP_TO_STRING &orc_to_string
#define HEAP_TEST &test_orc
#include "../src/Heap.h"

struct Orc { unsigned health; char name[10]; };

static void orc_to_string(const struct OrcHeapNode *const node,
	char (*const a)[12]) {
	sprintf(*a, "%u%.9s", node->priority, node->value->name);
}

#define POOL_NAME Orc
#define POOL_TYPE struct Orc
#include "Pool.h"

static void test_orc(struct OrcHeapNode *node, void *const vpool) {
	struct Orc *orc = OrcPoolNew(vpool);
	if(!orc) { assert(0); exit(EXIT_FAILURE); }
	orc->health = rand() / (RAND_MAX / 99 + 1);
	Orcish(orc->name, sizeof orc->name);
	node->priority = orc->health;
	node->value = orc;
}


int main(void) {
	struct OrcPool orcs = POOL_IDLE;
	rand();
	IntHeapTest(0);
	OrcHeapTest(&orcs), OrcPool_(&orcs);
	return EXIT_SUCCESS;
}

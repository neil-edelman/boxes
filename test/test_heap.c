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
static unsigned int_filler(void) {
	return (unsigned)rand() / (RAND_MAX / 99 + 1) + 1;
}
#define HEAP_NAME int
#define HEAP_TEST
#define HEAP_TO_STRING
#include "../src/heap.h"


/* Value pointer along with a priority. We have to store values somewhere, so
 we use a pool, (which depends on `heap`, it has evolved.) */
struct orc { unsigned health; char name[12]; };
static void orc_to_string(const struct orc *const orc, char (*const a)[12])
	{ sprintf(*a, "%u%.9s", orc->health, orc->name); }
#define POOL_NAME orc
#define POOL_TYPE struct orc
#include "pool.h"
static struct orc_pool orcs;
static unsigned orc_filler(struct orc **const orc_ptr) {
	struct orc *orc = orc_pool_new(&orcs);
	if(!orc) { assert(0); exit(EXIT_FAILURE); }
	orc->health = (unsigned)rand() / (RAND_MAX / 99 + 1);
	orcish(orc->name, sizeof orc->name);
	*orc_ptr = orc;
	return orc->health;
}
#define HEAP_NAME orc
#define HEAP_VALUE struct orc *
#define HEAP_TEST
#define HEAP_TO_STRING
#include "../src/heap.h"

#if 0
/* Maximum heap with a `size_t`. */
static void index_to_string(const size_t *const i, char (*const a)[12]) {
	sprintf(*a, "%lu", (unsigned long)*i);
}
static size_t test_index(void) {
	return (size_t)rand();
}
static int index_compare(const size_t a, const size_t b) { return a < b; }
#define HEAP_NAME index
#define HEAP_TYPE size_t
#define HEAP_COMPARE &index_compare
#define HEAP_TEST &test_index
#define HEAP_TO_STRING
#include "../src/heap.h"
#endif


int main(void) {
	rand();
	int_heap_test();
	orc_heap_test(), orc_pool_(&orcs);
	/*index_heap_test();*/
	return EXIT_SUCCESS;
}

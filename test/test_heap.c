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
static void int_filler(unsigned *const p)
	{ *p = (unsigned)rand() / (RAND_MAX / 99 + 1) + 1; }
#define HEAP_NAME int
#define HEAP_TEST
#define HEAP_TO_STRING
#include "../src/heap.h"


/* Value pointer along with a priority. We have to store values somewhere, so
 we use a pool, (which depends on `heap`, it has evolved.) */
struct orc { char name[12]; };
static void orc_to_string(const unsigned health, const struct orc *const orc,
	char (*const a)[12]) { sprintf(*a, "%u%.9s", health, orc->name); }
#define POOL_NAME orc
#define POOL_TYPE struct orc
#include "../src/pool.h"
static struct orc_pool orcs;
static void orc_filler(unsigned *const health, struct orc **const orc_ptr) {
	struct orc *orc = orc_pool_new(&orcs);
	if(!orc) { assert(0); exit(EXIT_FAILURE); }
	*health = (unsigned)rand() / (RAND_MAX / 99 + 1);
	orcish(orc->name, sizeof orc->name);
	*orc_ptr = orc;
}
#define HEAP_NAME orc
#define HEAP_VALUE struct orc *
#define HEAP_TO_STRING
#define HEAP_TEST
#include "../src/heap.h"


/* Maximum heap with a `size_t`. */
static void index_to_string(const size_t *const i, char (*const a)[12]) {
	sprintf(*a, "%lu", (unsigned long)*i);
}
static void index_filler(size_t *const p) { *p = (size_t)rand(); }
static int index_compare(const size_t a, const size_t b) { return a < b; }
#define HEAP_NAME index
#define HEAP_TYPE size_t
#define HEAP_COMPARE &index_compare
#define HEAP_TEST
#define HEAP_TO_STRING
#include "../src/heap.h"


/* Heap separate header. */
#define HEAP_NAME header
#define HEAP_HEAD
#include "../src/heap.h"
static void header_to_string(const unsigned *const i, char (*const z)[12])
	{ int_to_string(i, z); }
static void header_filler(unsigned *const p) { int_filler(p); }
#define HEAP_NAME header
#define HEAP_TEST
#define HEAP_TO_STRING
#define HEAP_BODY
#include "../src/heap.h"


int main(void) {
	rand();
	int_heap_test();
	orc_heap_test(), orc_pool_(&orcs);
	index_heap_test();
	header_heap_test();
	return EXIT_SUCCESS;
}

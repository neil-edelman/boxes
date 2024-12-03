/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 Test Heap.

 @std C89/90 */

#include <stdlib.h> /* EXIT malloc free rand */
#include <stdio.h>  /* *printf */
#include "orcish.h"


/* Minimal heap. This selects the default `unsigned int`. */
static int min_less(const unsigned a, const unsigned b) { return a < b; }
#define HEAP_NAME min
#include "../src/heap.h"


/* Also an `unsigned int`, but with testing. */
static int int_less(const unsigned a, const unsigned b) { return a < b; }
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
struct orc { char name[12]; }; /* Some complicated structure somewhere else. */
struct npc { unsigned health; struct orc *orc; };
static void npc_to_string(const struct npc *const npc, char (*const a)[12])
	{ sprintf(*a, "%d%.9s", npc->health, npc->orc->name); }
#define POOL_NAME orc
#define POOL_TYPE struct orc
#include "../src/pool.h"
static struct orc_pool orcs; /* Already initialized. */
static void npc_filler(struct npc *const npc) {
	struct orc *orc = orc_pool_new(&orcs);
	if(!orc) { assert(0); exit(EXIT_FAILURE); }
	orcish(orc->name, sizeof orc->name);
	/* Writing directly into the priority. */
	npc->health = (unsigned)rand() / (RAND_MAX / 99 + 1);
	npc->orc = orc;
}
static int npc_less(const struct npc a, const struct npc b)
	{ return /*strcmp(a->name, b->name)*/ a.health < b.health; }
#define HEAP_NAME npc
#define HEAP_TYPE struct npc
#define HEAP_TO_STRING
#define HEAP_TEST
#define TEST_HEAP_ZERO {0, 0}
#include "../src/heap.h"


/* Maximum heap with a `size_t`. */
static void index_to_string(const size_t *const i, char (*const a)[12])
	{ sprintf(*a, "%lu", (unsigned long)*i); }
static void index_filler(size_t *const p) { *p = (size_t)rand(); }
static int index_less(const size_t a, const size_t b) { return a < b; }
#define HEAP_NAME index
#define HEAP_TYPE size_t
#define HEAP_TEST
#define HEAP_TO_STRING
#include "../src/heap.h"


static int static_less(const int a, const int b) { return a > b; }
static void static_to_string(const int *const i, char (*const z)[12])
	{ sprintf(*z, "%d", *i); }
static void static_filler(int *const p)
	{ *p = (int)rand() / (RAND_MAX / 99 + 1) + 1; }
#define HEADER_HEAP_DEFINE
#include "header_heap.h"
struct header_heap header_heap(void)
	{ struct header_heap _; _._ = static_heap(); return _; }
void header_heap_(struct header_heap *const _) { static_heap_(&_->_); }
void header_heap_test(void) { static_heap_test(); }


int main(void) {
	rand();
	int_heap_test();
	npc_heap_test(), orc_pool_(&orcs);
	index_heap_test();
	header_heap_test();
	return EXIT_SUCCESS;
}

/** @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Stable Pool

 ![Example of Pool](../web/pool.png)

 This is the next version of pool. I don't like having two-pointers per data,
 this is stupid. Let's do like a deque, except bit fields to store what is
 valid so we can erase in the middle. Maybe 32/64/128... element blocks with
 a bit field pointed to by a vector and a circular list?

 Instead of a free-list, let's make it a free-heap:
 the list of indices is a queue, the heap is way better at concentrating values
 towards the front; list is O(1), heap O(log n); heap is half the size and
 can be allocated by need instead of all up front.

 @param[POOL_NAME, POOL_TYPE]
 `<T>` that satisfies `C` naming conventions when mangled and a valid tag type
 associated therewith; required. `<PT>` is private, whose names are prefixed in
 a manner to avoid collisions.

 @param[POOL_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[POOL_TO_STRING_NAME, POOL_TO_STRING]
 To string trait contained in <to_string.h>; `<A>` that satisfies `C` naming
 conventions when mangled and function implementing <typedef:<PA>to_string_fn>.
 There can be multiple to string traits, but only one can omit
 `POOL_TO_STRING_NAME`.

 @param[POOL_TEST]
 To string trait contained in <../test/pool_test.h>; optional unit testing
 framework using `assert`. Can only be defined once _per_ pool. Must be defined
 equal to a (random) filler function, satisfying <typedef:<PT>action_fn>.
 Output will be shown with the to string trait in which it's defined; provides
 tests for the base code and all later traits.

 @std C89
 @cf [array](https://github.com/neil-edelman/array)
 @cf [heap](https://github.com/neil-edelman/heap)
 @cf [list](https://github.com/neil-edelman/list)
 @cf [orcish](https://github.com/neil-edelman/orcish)
 @cf [set](https://github.com/neil-edelman/set)
 @cf [trie](https://github.com/neil-edelman/trie) */

#include <stddef.h> /* offsetof */
#include <stdlib.h>	/* malloc free */
#include <assert.h>	/* assert */
#include <errno.h>	/* errno */

#ifndef POOL_H /* <!-- idempotent */
#define POOL_H

/* <list.h> and <array.h> must be in the same directory. */
#define LIST_NAME pool_free
#include "list.h"
#define ARRAY_NAME pool_free
#define ARRAY_TYPE struct pool_free_list_node
#include "array.h"
#endif /* idempotent --> */

/* Check defines. */
#if !defined(POOL_NAME) || !defined(POOL_TYPE)
#error Name POOL_NAME undefined or tag type POOL_TYPE undefined.
#endif
#if defined(POOL_TO_STRING_NAME) || defined(POOL_TO_STRING)
#define POOL_TO_STRING_TRAIT 1
#else
#define POOL_TO_STRING_TRAIT 0
#endif
#define POOL_TRAITS POOL_TO_STRING_TRAIT
#if POOL_TRAITS > 1
#error Only one trait per include is allowed; use POOL_EXPECT_TRAIT.
#endif
#if POOL_TRAITS != 0 && (!defined(T_) || !defined(CAT) || !defined(CAT_))
#error T_ or CAT_? not yet defined; traits must be defined separately?
#endif
#if (POOL_TRAITS == 0) && defined(POOL_TEST)
#error POOL_TEST must be defined in POOL_TO_STRING trait.
#endif
#if defined(POOL_TO_STRING_NAME) && !defined(POOL_TO_STRING)
#error POOL_TO_STRING_NAME requires POOL_TO_STRING.
#endif


#if POOL_TRAITS == 0 /* <!-- base code */


/* <Kernighan and Ritchie, 1988, p. 231>. */
#if defined(X_) || defined(PX_) \
	|| (defined(POOL_SUBTYPE) ^ (defined(CAT) || defined(CAT_)))
#error Unexpected P?X_ or CAT_?; possible stray POOL_EXPECT_TRAIT?
#endif
#ifndef POOL_SUBTYPE /* <!-- !sub-type */
#define CAT_(x, y) x ## _ ## y
#define CAT(x, y) CAT_(x, y)
#endif /* !sub-type --> */
#define X_(thing) CAT(POOL_NAME, thing)
#define PX_(thing) CAT(pool, X_(thing))

/** A valid tag type set by `POOL_TYPE`. */
typedef POOL_TYPE PX_(type);

/** A chunk followed by data of `capacity`. */
struct PX_(chunk) { size_t capacity, size; };

#define ARRAY_NAME PX_(chunk)
#define ARRAY_TYPE struct PX_(chunk) *
#define ARRAY_SUBTYPE
#include "array.h"

/** Consists of a map of several chunks of increasing size and a free-list.
 Zeroed data is a valid state. To instantiate to an idle state, see
 <fn:<X>pool>, `POOL_IDLE`, `{0}` (`C99`,) or being `static`.

 ![States.](../web/states.png) */
struct X_(pool);
struct X_(pool) {
	struct PX_(chunk_array) chunks; /* Pointers to stable cap, size, data. */
	struct pool_free_list free; /* Free-list only in chunks[0]. */
	struct pool_free_array store; /* Backing for free-list. */
};
/* `{0}` is `C99`. */
#ifndef POOL_IDLE /* <!-- !zero */
#define POOL_IDLE { ARRAY_IDLE, LIST_IDLE }
#endif /* !zero --> */

#if 0
/** Flag `node` removed in the largest block of `pool` so that later data can
 overwrite it. */
static void PX_(enqueue_removed)(struct X_(pool) *const pool,
	PX_(type) *const data) {
	assert(pool && pool->largest && node
		&& node >= PT_(block_nodes)(pool->largest)
		&& node < PT_(block_nodes)(pool->largest) + pool->largest->size
		&& !node->x.prev && !node->x.next
		&& !pool->removed.prev == !pool->removed.next);
	node->x.next = &pool->removed;
	if(pool->removed.prev) {
		node->x.prev = pool->removed.prev;
		pool->removed.prev->next = &node->x;
	} else {
		node->x.prev = &pool->removed;
		pool->removed.next = &node->x;
	}
	pool->removed.prev = &node->x;
}

/** @return Dequeues a removed node from `pool`, or if the queue is empty,
 returns null. */
static struct PT_(node) *PT_(dequeue_removed)(struct T_(pool) *const pool) {
	struct PT_(x) *x0, *x1;
	assert(pool && !pool->removed.next == !pool->removed.prev);
	if(!(x0 = pool->removed.next)) return 0; /* No elements. */
	if((x1 = x0->next) == &pool->removed) { /* Last element. */
		pool->removed.prev = pool->removed.next = 0;
	} else { /* > 1 removed. */
		pool->removed.next = x1;
		x1->prev = &pool->removed;
	}
	x0->prev = x0->next = 0;
	return PT_(x_upcast)(x0);
}

/** Gets rid of the removed node at the tail of the list of `pool`.
 @order Amortized \O(1). */
static void PT_(trim_removed)(struct T_(pool) *const pool) {
	struct PT_(node) *node;
	struct PT_(block) *const block = pool->largest;
	struct PT_(node) *const nodes = PT_(block_nodes)(block);
	assert(pool && block);
	while(block->size && (node = nodes + block->size - 1)->x.prev) {
		assert(node->x.next);
		if(node->x.prev == node->x.next) { /* There's only one. */
			pool->removed.prev = pool->removed.next = 0;
		} else {
			node->x.prev->next = node->x.next;
			node->x.next->prev = node->x.prev;
		}
		block->size--;
	}
}
#endif

/** Makes sure there is space for `n` further items in `pool`.
 @return Success. */
static int PX_(buffer)(struct X_(pool) *const pool, const size_t n) {
	struct PX_(chunk) *c0;
	assert(pool && (!pool->chunks.size
		|| pool->chunks.data[0]->size >= pool->chunks.data[0]->capacity));
	if(!pool->chunks.size) { /* It is idle. */
		assert();
	}
		&& (c0 = pool->chunks.data[0], n <= c0->capacity - c0->size))
		return 1;
}

/** Initializes `pool` to idle. @order \Theta(1) @allow */
static void X_(pool)(struct X_(pool) *const pool) {
	assert(pool);
	PX_(chunk_array)(&pool->chunks), PX_(free_list_clear)(&pool->free);
}

/** Destroys `pool` and returns it to idle. @order \O(\log `data`) @allow */
static void X_(pool_)(struct X_(pool) *const pool) {
	struct PX_(chunk) *i, *i_end;
	assert(pool);
	for(i = *pool->chunks.data, i_end = i + pool->chunks.size; i < i_end; i++)
		free(i);
	X_(pool)(pool);
}

#if 0
/** Pre-sizes an _idle_ `pool` to ensure that it can hold at least `min`
 elements. @param[min] If zero, doesn't do anything and returns true.
 @return Success; the pool becomes active with at least `min` elements.
 @throws[EDOM] The pool is active and doesn't allow reserving.
 @throws[ERANGE, malloc] @allow */
static int T_(pool_reserve)(struct T_(pool) *const pool, const size_t min) {
	if(!pool) return 0;
	if(pool->largest) return errno = EDOM, 0;
	return min ? PT_(reserve)(pool, min) : 1;
}
#endif

/** This pointer is constant until it gets removed.
 @return A pointer to a new uninitialized element from `pool`.
 @throws[ERANGE, malloc] @order amortised O(1) @allow */
static PX_(type) *X_(pool_new)(struct X_(pool) *const pool) {
	struct PT_(node) *node;
	size_t size;
	assert(pool);
	if((node = PT_(dequeue_removed)(pool))) return &node->data;
	size = pool->largest ? pool->largest->size : 0;
	if(!PT_(reserve)(pool, size + 1)) return 0;
	assert(pool->largest);
	node = PT_(block_nodes)(pool->largest) + pool->largest->size++;
	node->x.prev = node->x.next = 0;
	return &node->data;
}

/** Deletes `datum` from `pool`. @return Success.
 @throws[EDOM] `data` is not part of `pool`.
 @order Amortised \O(1), if the pool is in steady-state, but
 \O(log `pool.items`) for a small number of deleted items. @allow */
static int T_(pool_remove)(struct T_(pool) *const pool,
	PT_(type) *const datum) {
	struct PT_(node) *node;
	struct PT_(block) *block, **baddr;
	assert(pool && datum);
	node = PT_(data_upcast)(datum);
	/* Removed already or not part of the container. */
	if(node->x.next || !(block = *(baddr = PT_(find_block_addr)(pool, node))))
		return errno = EDOM, 0;
	assert(!node->x.prev && block->size);
	if(block == pool->largest) { /* The largest block has a free list. */
		size_t idx = node - PT_(block_nodes)(block);
		PT_(enqueue_removed)(pool, node);
		if(idx >= block->size - 1) PT_(trim_removed)(pool);
	} else {
		PT_(flag_removed)(node);
		if(!--block->size) { *baddr = block->smaller, free(block); }
	}
	return 1;
}

/** Removes all from `pool`, but keeps it's active state. (Only freeing the
 smaller blocks.) @order \O(`pool.blocks`) @allow */
static void T_(pool_clear)(struct T_(pool) *const pool) {
	struct PT_(block) *block, *next;
	assert(pool);
	if(!pool->largest) return;
	block = pool->largest, next = block->smaller;
	block->size = 0, block->smaller = 0;
	pool->removed.prev = pool->removed.next = 0;
	while(next) block = next, next = next->smaller, free(block);
}

/** Iterates though `pool` and calls `action` on all the elements.
 @order O(`capacity` \times `action`) @allow */
static void T_(pool_for_each)(struct T_(pool) *const pool,
	const PT_(action_fn) action) {
	struct PT_(node) *n, *end;
	struct PT_(block) *block;
	if(!pool || !action) return;
	for(block = pool->largest; block; block = block->smaller)
		for(n = PT_(block_nodes)(block), end = n + PT_(range)(pool, block);
			n < end; n++) if(!n->x.prev) action(&n->data);
}

/** Contains all iteration parameters. */
struct PT_(iterator);
struct PT_(iterator)
	{ const struct T_(pool) *pool; struct PT_(block) *block; size_t i; };

/** Loads `pool` into `it`. @implements begin */
static void PT_(begin)(struct PT_(iterator) *const it,
	const struct T_(pool) *const pool) {
	assert(it && pool);
	it->pool = pool, it->block = pool->largest, it->i = 0;
}

/** Advances `it`. @implements next */
static const PT_(type) *PT_(next)(struct PT_(iterator) *const it) {
	struct PT_(node) *nodes, *n;
	size_t i_end;
	assert(it && it->pool);
	while(it->block) {
		nodes = PT_(block_nodes)(it->block);
		i_end = PT_(range)(it->pool, it->block);
		while(it->i < i_end)
			if(!(n = nodes + it->i++)->x.prev) return &n->data;
		it->block = it->block->smaller;
	}
	return 0;
}
	
#if defined(ITERATE) || defined(ITERATE_BOX) || defined(ITERATE_TYPE) \
	|| defined(ITERATE_BEGIN) || defined(ITERATE_NEXT)
#error Unexpected ITERATE*.
#endif
	
#define ITERATE struct PX_(iterator)
#define ITERATE_BOX struct X_(pool)
#define ITERATE_TYPE PX_(type)
#define ITERATE_BEGIN PX_(begin)
#define ITERATE_NEXT PX_(next)

static void PT_(unused_base_coda)(void);
static void PT_(unused_base)(void) {
	/* Define `POOL_C99` to stop the error on this line. */
	POOL_BUILD_BUG_ON(sizeof(pool32) != 4);
	T_(pool)(0); T_(pool_)(0); T_(pool_reserve)(0, 0); T_(pool_new)(0);
	T_(pool_remove)(0, 0); T_(pool_clear)(0); T_(pool_for_each)(0, 0);
	PT_(begin)(0, 0); PT_(next)(0); PT_(unused_base_coda)();
}
static void PT_(unused_base_coda)(void) { PT_(unused_base)(); }


#elif defined(POOL_TO_STRING) /* base code --><!-- to string trait */


#ifdef POOL_TO_STRING_NAME /* <!-- name */
#define A_(thing) CAT(X_(pool), CAT(POOL_TO_STRING_NAME, thing))
#else /* name --><!-- !name */
#define A_(thing) CAT(X_(pool), thing)
#endif /* !name --> */
#define TO_STRING POOL_TO_STRING
#include "to_string.h" /** \include */

#if !defined(POOL_TEST_BASE) && defined(POOL_TEST) /* <!-- test */
#define POOL_TEST_BASE /* Only one instance of base tests. */
#include "../test/test_pool.h" /** \include */
#endif /* test --> */

#undef A_
#undef POOL_TO_STRING
#ifdef POOL_TO_STRING_NAME
#undef POOL_TO_STRING_NAME
#endif


#endif /* traits --> */


#ifdef POOL_EXPECT_TRAIT /* <!-- trait */
#undef POOL_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifndef POOL_SUBTYPE /* <!-- !sub-type */
#undef CAT
#undef CAT_
#else /* !sub-type --><!-- sub-type */
#undef POOL_SUBTYPE
#endif /* sub-type --> */
#undef X_
#undef PX_
#undef POOL_NAME
#undef POOL_TYPE
#ifdef POOL_TEST
#undef POOL_TEST
#endif
#ifdef POOL_TEST_BASE
#undef POOL_TEST_BASE
#endif
#undef ITERATE
#undef ITERATE_BOX
#undef ITERATE_TYPE
#undef ITERATE_BEGIN
#undef ITERATE_NEXT
#endif /* !trait --> */

#undef POOL_TO_STRING_TRAIT
#undef POOL_TRAITS

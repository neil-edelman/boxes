/** @license 2016 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Stable Pool

 ![Example of Pool](../web/pool.png)

 <tag:<P>pool> stores `<P>` in a memory pool. Pointers to valid items in the
 pool are stable, but not generally in any order or contiguous. It uses
 geometrically increasing size-blocks and when the removal is ongoing and
 uniformly sampled, (specifically, old elements are all eventually removed,)
 and data reaches a steady-state size, the data will settle in one allocated
 region. In this way, manages a fairly contiguous space for items which have
 references.

 @param[POOL_NAME, POOL_TYPE]
 `<P>` that satisfies `C` naming conventions when mangled and a valid tag type
 associated therewith; required. `<PP>` is private, whose names are prefixed in
 a manner to avoid collisions.

 @param[POOL_TEST]
 To string trait contained in <../test/test_pool.h>; optional unit testing
 framework using `assert`. Can only be defined once _per_ pool. Must be defined
 equal to a (random) filler function, satisfying <typedef:<PP>action_fn>.
 Output will be shown with the to string trait in which it's defined; provides
 tests for the base code and all later traits.

 @param[POOL_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[POOL_TO_STRING_NAME, POOL_TO_STRING]
 To string trait contained in <to_string.h>; `<Z>` that satisfies `C` naming
 conventions when mangled and function implementing <typedef:<PZ>to_string_fn>.
 There can be multiple to string traits, but only one can omit
 `POOL_TO_STRING_NAME`.

 @std C89 */

#include <stddef.h> /* offsetof */
#include <stdlib.h>	/* malloc free */
#include <assert.h>	/* assert */
#include <errno.h>	/* errno */


#if !defined(POOL_NAME) || !defined(POOL_TYPE)
#error Name POOL_NAME undefined or tag type POOL_TYPE undefined.
#endif
#if defined(POOL_TO_STRING_NAME) || defined(POOL_TO_STRING) /* <!-- str */
#define POOL_TO_STRING_TRAIT 1
#else /* str --><!-- !str */
#define POOL_TO_STRING_TRAIT 0
#endif /* !str --> */
#define POOL_TRAITS POOL_TO_STRING_TRAIT
#if POOL_TRAITS > 1
#error Only one trait per include is allowed; use ARRAY_EXPECT_TRAIT.
#endif
#if POOL_TRAITS != 0 && (!defined(P_) || !defined(CAT) || !defined(CAT_))
#error Use POOL_EXPECT_TRAIT and include it again.
#endif
#if defined(POOL_TO_STRING_NAME) && !defined(POOL_TO_STRING)
#error POOL_TO_STRING_NAME requires POOL_TO_STRING.
#endif


#if POOL_TRAITS == 0 /* <!-- base code */


/* <Kernighan and Ritchie, 1988, p. 231>. */
#if defined(P_) || defined(PP_) \
	|| (defined(POOL_SUBTYPE) ^ (defined(CAT) || defined(CAT_)))
#error Unexpected P?P_ or CAT_?; possible stray POOL_EXPECT_TRAIT?
#endif
#ifndef POOL_SUBTYPE /* <!-- !sub-type */
#define CAT_(x, y) x ## _ ## y
#define CAT(x, y) CAT_(x, y)
#endif /* !sub-type --> */
#define P_(thing) CAT(POOL_NAME, thing)
#define PP_(thing) CAT(pool, P_(thing))

/** A valid tag type set by `POOL_TYPE`. */
typedef POOL_TYPE PP_(type);

/** Operates by side-effects. */
typedef void (*PP_(action_fn))(PP_(type) *const data);

/* Free-list item. The reason it's doubly-linked is to support popping a link
 from the end. The reason it's <tag:<PP>x> instead of <tag:<PP>node> is it
 greatly simplifies edge cases, at the expense of casting. */
struct PP_(x) { struct PP_(x) *prev, *next; };

/* Nodes containing the data and, in the largest block, the free list item;
 smaller blocks satisfy `x.prev, x.next -> deleted`. */
struct PP_(node) { PP_(type) data; struct PP_(x) x; };

/* Information about each block and will have capacity in an array of
 <tag:<PP>node> at `block + 1`, specified by <fn:<PP>block_nodes>. */
struct PP_(block) { struct PP_(block) *smaller; size_t capacity, size; };

/** Retrieve the array implicitly after `b`. */
static struct PP_(node) *PP_(block_nodes)(struct PP_(block) *const b)
	{ return (struct PP_(node) *)(void *)(b + 1); }

/** Zeroed data is a valid state. To instantiate to an idle state, see
 <fn:<P>pool>, `POOL_IDLE`, `{0}` (`C99`,) or being `static`.

 ![States.](../web/states.png) */
struct P_(pool);
struct P_(pool) {
	/* Ideally, all items to go in here, but there may be smaller blocks. */
	struct PP_(block) *largest;
	/* Fibonacci; largest -> (c0 < c1 || c0 == c1 == max_size). */
	size_t next_capacity;
	/* {0,0} -> no nodes removed from the largest block, otherwise, it's a
	 circular list of items in the largest block. All other states invalid. */
	struct PP_(x) removed;
};
/* `{0}` is `C99`. */
#ifndef POOL_IDLE /* <!-- !zero */
#define POOL_IDLE { 0, 0, { 0, 0 } }
#endif /* !zero --> */

/** Container of `data`. */
static struct PP_(node) *PP_(data_upcast)(PP_(type) *const data)
	{ return (struct PP_(node) *)(void *)
	((char *)data - offsetof(struct PP_(node), data)); }

/** Container of `x`. */
static struct PP_(node) *PP_(x_upcast)(struct PP_(x) *const x)
	{ return (struct PP_(node) *)(void *)
	((char *)x - offsetof(struct PP_(node), x)); }

/** Only for the not-largest, inactive, blocks, `node` becomes a boolean,
 null/not. */
static void PP_(flag_removed)(struct PP_(node) *const node)
	{ assert(node); node->x.prev = node->x.next = &node->x; }

/** Flag `node` removed in the largest block of `pool` so that later data can
 overwrite it. */
static void PP_(enqueue_removed)(struct P_(pool) *const pool,
	struct PP_(node) *const node) {
	assert(pool && pool->largest && node
		&& node >= PP_(block_nodes)(pool->largest)
		&& node < PP_(block_nodes)(pool->largest) + pool->largest->size
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
static struct PP_(node) *PP_(dequeue_removed)(struct P_(pool) *const pool) {
	struct PP_(x) *x0, *x1;
	assert(pool && !pool->removed.next == !pool->removed.prev);
	if(!(x0 = pool->removed.next)) return 0; /* No elements. */
	if((x1 = x0->next) == &pool->removed) { /* Last element. */
		pool->removed.prev = pool->removed.next = 0;
	} else { /* > 1 removed. */
		pool->removed.next = x1;
		x1->prev = &pool->removed;
	}
	x0->prev = x0->next = 0;
	return PP_(x_upcast)(x0);
}

/** Gets rid of the removed node at the tail of the list of `pool`.
 @order Amortized \O(1). */
static void PP_(trim_removed)(struct P_(pool) *const pool) {
	struct PP_(node) *node;
	struct PP_(block) *const block = pool->largest;
	struct PP_(node) *const nodes = PP_(block_nodes)(block);
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

/** How much data is in a `block` in `pool`, skipping items removed. */
static size_t PP_(range)(const struct P_(pool) *const pool,
	const struct PP_(block) *const block) {
	assert(pool && block);
	return block == pool->largest ? block->size : block->capacity;
}

/** @order At worst \O(log `pool.items`) when there's no deletion.
 @return What block `node` is in `pool`. */
static struct PP_(block) **PP_(find_block_addr)(struct P_(pool) *const pool,
	const struct PP_(node) *const node) {
	struct PP_(block) *b, **baddr;
	struct PP_(node) *n;
	assert(pool && node);
	for(baddr = &pool->largest, b = *baddr; b; baddr = &b->smaller, b = *baddr)
		if(n = PP_(block_nodes)(b), node >= n && node < n + b->capacity) break;
	return baddr;
}

/** Ensures `min` capacity of the largest block in `pool` where the free list
 is empty. @param[min] If zero, allocates anyway.
 @return Success; otherwise, `errno` will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `malloc`
 doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/malloc.html).
 @throws[malloc] */
static int PP_(reserve)(struct P_(pool) *const pool, const size_t min) {
	size_t c0, c1;
	struct PP_(block) *block;
	const size_t max_size = ((size_t)-1 - sizeof *block)
		/ sizeof(struct PP_(node));
	assert(pool && !pool->removed.prev && !pool->removed.next && min);
	assert(!pool->largest
		|| (pool->largest->capacity < pool->next_capacity
		&& pool->next_capacity <= max_size)
		|| (pool->largest->capacity == pool->next_capacity) == max_size);
	if(pool->largest && pool->largest->capacity >= min) return 1;
	if(min > max_size) return errno = ERANGE, 0;
	if(!pool->largest) {
		c0 = 8, c1 = 13;
	} else {
		c0 = pool->largest->capacity, c1 = pool->next_capacity;
	}
	while(c0 < min) { /* `min < max_size`; this `c0 ^= c1 ^= c0 ^= c1 += c0`. */
		size_t temp = c0 + c1;
		if(temp > max_size || temp < c1) temp = max_size;
		c0 = c1, c1 = temp;
	}
	if(!(block = malloc(sizeof *block + c0 * sizeof(struct PP_(node)))))
		{ if(!errno) errno = ERANGE; return 0; }
	block->smaller = pool->largest;
	block->capacity = c0;
	block->size = 0;
	pool->largest = block;
	pool->next_capacity = c1;
	pool->removed.prev = pool->removed.next = 0;
	return 1;
}

/** Initialises `pool` to idle. @order \Theta(1) @allow */
static void P_(pool)(struct P_(pool) *const pool)
	{ assert(pool), pool->largest = 0, pool->next_capacity = 0,
	pool->removed.prev = pool->removed.next = 0; }

/** Destroys `pool` and returns it to idle. @order \O(`blocks`) @allow */
static void P_(pool_)(struct P_(pool) *const pool) {
	struct PP_(block) *block, *next;
	assert(pool);
	for(block = pool->largest; block; block = next)
		next = block->smaller, free(block);
	P_(pool)(pool);
}

/** Pre-sizes an _idle_ `pool` to ensure that it can hold at least `min`
 elements. @param[min] If zero, doesn't do anything and returns true.
 @return Success; the pool becomes active with at least `min` elements.
 @throws[EDOM] The pool is active and doesn't allow reserving.
 @throws[ERANGE, malloc] @allow */
static int P_(pool_reserve)(struct P_(pool) *const pool, const size_t min) {
	if(!pool) return 0;
	if(pool->largest) return errno = EDOM, 0;
	return min ? PP_(reserve)(pool, min) : 1;
}

/** @return A new element from `pool`. @throws[ERANGE, malloc]
 @order amortised O(1) @allow */
static PP_(type) *P_(pool_new)(struct P_(pool) *const pool) {
	struct PP_(node) *node;
	size_t size;
	assert(pool);
	if((node = PP_(dequeue_removed)(pool))) return &node->data;
	size = pool->largest ? pool->largest->size : 0;
	if(!PP_(reserve)(pool, size + 1)) return 0;
	assert(pool->largest);
	node = PP_(block_nodes)(pool->largest) + pool->largest->size++;
	node->x.prev = node->x.next = 0;
	return &node->data;
}

/** Deletes `datum` from `pool`. @return Success.
 @throws[EDOM] `data` is not part of `pool`.
 @order Amortised \O(1), if the pool is in steady-state, but
 \O(log `pool.items`) for a small number of deleted items. @allow */
static int P_(pool_remove)(struct P_(pool) *const pool,
	PP_(type) *const datum) {
	struct PP_(node) *node;
	struct PP_(block) *block, **baddr;
	assert(pool && datum);
	node = PP_(data_upcast)(datum);
	/* Removed already or not part of the container. */
	if(node->x.next || !(block = *(baddr = PP_(find_block_addr)(pool, node))))
		return errno = EDOM, 0;
	assert(!node->x.prev && block->size);
	if(block == pool->largest) { /* The largest block has a free list. */
		size_t idx = (size_t)(node - PP_(block_nodes)(block));
		PP_(enqueue_removed)(pool, node);
		if(idx >= block->size - 1) PP_(trim_removed)(pool);
	} else {
		PP_(flag_removed)(node);
		if(!--block->size) { *baddr = block->smaller, free(block); }
	}
	return 1;
}

/** Removes all from `pool`, but keeps it's active state. (Only freeing the
 smaller blocks.) @order \O(`pool.blocks`) @allow */
static void P_(pool_clear)(struct P_(pool) *const pool) {
	struct PP_(block) *block, *next;
	assert(pool);
	if(!pool->largest) return;
	block = pool->largest, next = block->smaller;
	block->size = 0, block->smaller = 0;
	pool->removed.prev = pool->removed.next = 0;
	while(next) block = next, next = next->smaller, free(block);
}

/** Iterates though `pool` and calls `action` on all the elements.
 @order O(`capacity` \times `action`) @allow */
static void P_(pool_for_each)(struct P_(pool) *const pool,
	const PP_(action_fn) action) {
	struct PP_(node) *n, *end;
	struct PP_(block) *block;
	if(!pool || !action) return;
	for(block = pool->largest; block; block = block->smaller)
		for(n = PP_(block_nodes)(block), end = n + PP_(range)(pool, block);
			n < end; n++) if(!n->x.prev) action(&n->data);
}

/* <!-- iterate interface */
#define BOX_ITERATE

/** Contains all iteration parameters. */
struct PP_(iterator);
struct PP_(iterator)
	{ const struct P_(pool) *pool; struct PP_(block) *block; size_t i; };

/** Loads `pool` into `it`. @implements begin */
static void PP_(begin)(struct PP_(iterator) *const it,
	const struct P_(pool) *const pool) {
	assert(it && pool);
	it->pool = pool, it->block = pool->largest, it->i = 0;
}

/** Advances `it`. @implements next */
static const PP_(type) *PP_(next)(struct PP_(iterator) *const it) {
	struct PP_(node) *nodes, *n;
	size_t i_end;
	assert(it && it->pool);
	while(it->block) {
		nodes = PP_(block_nodes)(it->block);
		i_end = PP_(range)(it->pool, it->block);
		while(it->i < i_end)
			if(!(n = nodes + it->i++)->x.prev) return &n->data;
		it->block = it->block->smaller;
	}
	return 0;
}

/* iterate --> */

/* Define these for traits. */
#define BOX_ PP_
#define BOX_CONTAINER struct P_(pool)
#define BOX_CONTENTS PP_(type)

static void PP_(unused_base_coda)(void);
static void PP_(unused_base)(void) {
	P_(pool)(0); P_(pool_)(0); P_(pool_reserve)(0, 0); P_(pool_new)(0);
	P_(pool_remove)(0, 0); P_(pool_clear)(0); P_(pool_for_each)(0, 0);
	PP_(begin)(0, 0); PP_(next)(0);
	PP_(unused_base_coda)();
}
static void PP_(unused_base_coda)(void) { PP_(unused_base)(); }


#elif defined(POOL_TO_STRING) /* base code --><!-- to string trait */


#define TO_STRING POOL_TO_STRING
#define ZI_ POOL_FORWARD_
#ifdef POOL_TO_STRING_NAME /* <!-- name */
#define Z_(n) CAT(P_(pool), CAT(POOL_TO_STRING_NAME, n))
#else /* name --><!-- !name */
#define Z_(n) CAT(P_(pool), n)
#endif /* !name --> */
#include "to_string.h" /** \include */

#if !defined(POOL_TEST_BASE) && defined(POOL_TEST) /* <!-- test */
#define POOL_TEST_BASE /* Only one instance of base tests. */
#include "../test/test_pool.h" /** \include */
#endif /* test --> */

#undef Z_ /* From <to_string.h>. */
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
#ifdef POOL_ITERATE /* <!-- iter */
#undef POOL_FORWARD_
#undef POOL_ITERATE
#endif /* iter --> */
#undef P_
#undef PP_
#undef POOL_NAME
#undef POOL_TYPE
#ifdef POOL_TEST
#undef POOL_TEST
#endif
#ifdef POOL_TEST_BASE
#undef POOL_TEST_BASE
#endif
#endif /* !trait --> */
#undef POOL_TO_STRING_TRAIT
#undef POOL_TRAITS

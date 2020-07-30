/** @license 2016 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Stable Pool

 ![Example of Pool](../web/pool.png)

 <tag:<T>pool> stores unordered `<T>` in a memory pool. Pointers to valid items
 in the pool are stable, but not generally contiguous or in any order. It uses
 geometrically increasing size-blocks and when the removal is ongoing and
 uniformly sampled, (specifically, old elements are all eventually removed,)
 and data reaches a steady-state size, the data will settle in one allocated
 region. In this way, manages a fairly contiguous space for items which have
 references.

 `<T>pool` is not synchronised. Errors are returned with `errno`. The
 parameters are preprocessor macros, and are all undefined at the end of the
 file for convenience. `assert.h` is included in this file; to stop the
 debug assertions, use `#define NDEBUG` before `assert.h`.

 @param[POOL_NAME, POOL_TYPE]
 `<T>` that satisfies `C` naming conventions when mangled and a valid tag type
 associated therewith; required. `<PT>` is private, whose names are prefixed in
 a manner to avoid collisions.

 @param[POOL_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[POOL_TO_STRING_NAME, POOL_TO_STRING]
 To string trait contained in <ToString.h>; `<A>` that satisfies `C` naming
 conventions when mangled and function implementing <typedef:<PT>to_string>.
 There can be multiple to string traits, but only one can omit
 `POOL_TO_STRING_NAME`.

 @param[POOL_TEST]
 To string trait contained in <../test/PoolTest.h>; optional unit testing
 framework using `assert`. Can only be defined once _per_ `Pool`. Must be
 defined equal to a (random) filler function, satisfying <typedef:<PT>action>.
 Output will be shown with the to string trait in which it's defined; provides
 tests for the base code and all later traits.

 @std C89
 @cf [Array](https://github.com/neil-edelman/Array)
 @cf [Heap](https://github.com/neil-edelman/Heap)
 @cf [List](https://github.com/neil-edelman/List)
 @cf [Orcish](https://github.com/neil-edelman/Orcish)
 @cf [Set](https://github.com/neil-edelman/Set)
 @cf [Trie](https://github.com/neil-edelman/Trie) */

#include <stddef.h> /* offsetof */
#include <stdlib.h>	/* malloc free */
#include <assert.h>	/* assert */
#include <errno.h>	/* errno */


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
#if defined(T_) || defined(PT_) \
	|| (defined(POOL_SUBTYPE) ^ (defined(CAT) || defined(CAT_)))
#error Unexpected P?T_ or CAT_?; possible stray POOL_EXPECT_TRAIT?
#endif
#ifndef POOL_SUBTYPE /* <!-- !sub-type */
#define CAT_(x, y) x ## _ ## y
#define CAT(x, y) CAT_(x, y)
#endif /* !sub-type --> */
#define T_(thing) CAT(POOL_NAME, thing)
#define PT_(thing) CAT(pool, T_(thing))

/** A valid tag type set by `POOL_TYPE`. */
typedef POOL_TYPE PT_(type);

/** Operates by side-effects. */
typedef void (*PT_(action))(PT_(type) *const data);

/* Free-list item. The reason it's doubly-linked is to support popping a link
 from the end. The reason it's <tag:<PT>x> instead of <tag:<PT>node> is it
 greatly simplifies edge cases, at the expense of casting. */
struct PT_(x) { struct PT_(x) *prev, *next; };

/* Nodes containing the data and, in the largest block, the free list item;
 smaller blocks satisfy `x.prev, x.next -> deleted`. */
struct PT_(node) { PT_(type) data; struct PT_(x) x; };

/* Information about each block and will have capacity in an array of
 <tag:<PT>node> at `block + 1`, specified by <fn:<PT>block_nodes>. */
struct PT_(block) { struct PT_(block) *smaller; size_t capacity, size; };

/** Retrieve the array implicitly after `b`. */
static struct PT_(node) *PT_(block_nodes)(struct PT_(block) *const b)
	{ return (struct PT_(node) *)(void *)(b + 1); }

/** Zeroed data is a valid state. To instantiate to an idle state, see
 <fn:<T>pool>, `POOL_IDLE`, `{0}` (`C99`,) or being `static`.

 ![States.](../web/states.png) */
struct T_(pool);
struct T_(pool) {
	/* Ideally, all items to go in here, but there may be smaller blocks. */
	struct PT_(block) *largest;
	/* Fibonacci; largest -> (c0 < c1 || c0 == c1 == max_size). */
	size_t next_capacity;
	/* {0,0} -> no nodes removed from the largest block, otherwise, it's a
	 circular list of items in the largest block. All other states invalid. */
	struct PT_(x) removed;
};
/* `{0}` is `C99`. */
#ifndef POOL_IDLE /* <!-- !zero */
#define POOL_IDLE { 0, 0, { 0, 0 } }
#endif /* !zero --> */

/** Container of `data`. */
static struct PT_(node) *PT_(data_upcast)(PT_(type) *const data)
	{ return (struct PT_(node) *)(void *)
	((char *)data - offsetof(struct PT_(node), data)); }

/** Container of `x`. */
static struct PT_(node) *PT_(x_upcast)(struct PT_(x) *const x)
	{ return (struct PT_(node) *)(void *)
	((char *)x - offsetof(struct PT_(node), x)); }

/** Only for the not-largest, inactive, blocks, `node` becomes a boolean,
 null/not. */
static void PT_(flag_removed)(struct PT_(node) *const node)
	{ assert(node); node->x.prev = node->x.next = &node->x; }

/** Flag `node` removed in the largest block of `pool` so that later data can
 overwrite it. */
static void PT_(enqueue_removed)(struct T_(pool) *const pool,
	struct PT_(node) *const node) {
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

/** How much data is in a `block` in `pool`, skipping items removed. */
static size_t PT_(range)(const struct T_(pool) *const pool,
	const struct PT_(block) *const block) {
	assert(pool && block);
	return block == pool->largest ? block->size : block->capacity;
}

/** @order At worst \O(log `pool.items`) when there's no deletion.
 @return What block `node` is in `pool`. */
static struct PT_(block) **PT_(find_block_addr)(struct T_(pool) *const pool,
	const struct PT_(node) *const node) {
	struct PT_(block) *b, **baddr;
	struct PT_(node) *n;
	assert(pool && node);
	for(baddr = &pool->largest, b = *baddr; b; baddr = &b->smaller, b = *baddr)
		if(n = PT_(block_nodes)(b), node >= n && node < n + b->capacity) break;
	return baddr;
}

/** Ensures `min` capacity of the largest block in `pool` where the free list
 is empty. @param[min] If zero, allocates anyway.
 @return Success; otherwise, `errno` will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `malloc`
 doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/malloc.html).
 @throws[malloc] */
static int PT_(reserve)(struct T_(pool) *const pool, const size_t min) {
	size_t c0, c1;
	struct PT_(block) *block;
	const size_t max_size = ((size_t)-1 - sizeof *block)
		/ sizeof(struct PT_(node));
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
	if(!(block = malloc(sizeof *block + c0 * sizeof(struct PT_(node)))))
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
static void T_(pool)(struct T_(pool) *const pool)
	{ assert(pool), pool->largest = 0, pool->next_capacity = 0,
	pool->removed.prev = pool->removed.next = 0; }

/** Destroys `pool` and returns it to idle. @order \O(`blocks`) @allow */
static void T_(pool_)(struct T_(pool) *const pool) {
	struct PT_(block) *block, *next;
	assert(pool);
	for(block = pool->largest; block; block = next)
		next = block->smaller, free(block);
	T_(pool)(pool);
}

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

/** @return A new element from `pool`. @throws[ERANGE, malloc]
 @order amortised O(1) @allow */
static PT_(type) *T_(pool_new)(struct T_(pool) *const pool) {
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
	const PT_(action) action) {
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

/** Loads `a` into `it`. @implements begin */
static void PT_(begin)(struct PT_(iterator) *const it,
	const struct T_(pool) *const pool) {
	assert(it && pool);
	it->pool = pool, it->block = pool->largest, it->i = 0;
}

/** Advances `it`. @implements next */
static PT_(type) *PT_(next)(struct PT_(iterator) *const it) {
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
	
#define ITERATE struct PT_(iterator)
#define ITERATE_BOX struct T_(pool)
#define ITERATE_TYPE PT_(type)
#define ITERATE_BEGIN PT_(begin)
#define ITERATE_NEXT PT_(next)

static void PT_(unused_base_coda)(void);
static void PT_(unused_base)(void) {
	T_(pool)(0); T_(pool_)(0); T_(pool_reserve)(0, 0); T_(pool_new)(0);
	T_(pool_remove)(0, 0); T_(pool_clear)(0); T_(pool_for_each)(0, 0);
	PT_(unused_base_coda)();
}
static void PT_(unused_base_coda)(void) { PT_(unused_base)(); }


#elif defined(POOL_TO_STRING) /* base code --><!-- to string trait */


#ifdef POOL_TO_STRING_NAME /* <!-- name */
#define A_(thing) CAT(T_(pool), CAT(POOL_TO_STRING_NAME, thing))
#else /* name --><!-- !name */
#define A_(thing) CAT(T_(pool), thing)
#endif /* !name --> */
#define TO_STRING POOL_TO_STRING
#include "ToString.h" /** \include */

#if !defined(POOL_TEST_BASE) && defined(POOL_TEST) /* <!-- test */
#define POOL_TEST_BASE /* Only one instance of base tests. */
#include "../test/TestPool.h" /** \include */
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
#undef T_
#undef PT_
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

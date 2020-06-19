/** @license 2016 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Parameterised Stable Pool

 ![Example of Pool](../web/pool.png)

 <tag:<T>Pool> stores unordered `<T>` in a memory pool. Pointers to valid items
 in the pool are stable, but not generally contiguous. It uses geometrically
 increasing size-blocks and when the removal is ongoing and uniformly sampled,
 (specifically, old elements are all eventually removed,) and data reaches a
 steady-state size, the data will settle in one allocated region. In this way,
 provides a fairly contiguous space for items which have references.

 `<T>Pool` is not synchronised. Errors are returned with `errno`. The
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
 conventions when mangled and function implementing <typedef:<PT>ToString>.
 There can be multiple to string traits, but only one can omit
 `POOL_TO_STRING_NAME`.

 @param[POOL_TEST]
 To string trait contained in <../test/PoolTest.h>; optional unit testing
 framework using `assert`. Can only be defined once _per_ `Pool`. Must be
 defined equal to a (random) filler function, satisfying <typedef:<PT>Action>.
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
#ifndef POOL_NAME
#error Name POOL_NAME undefined.
#endif
#ifndef POOL_TYPE
#error Tag type POOL_TYPE undefined.
#endif
#if defined(POOL_TO_STRING_NAME) || defined(POOL_TO_STRING)
#define POOL_TO_STRING_INTERFACE 1
#else
#define POOL_TO_STRING_INTERFACE 0
#endif
#define POOL_INTERFACES POOL_TO_STRING_INTERFACE
#if POOL_INTERFACES > 1
#error Only one trait per include is allowed; use POOL_EXPECT_TRAIT.
#endif
#if (POOL_INTERFACES == 0) && defined(POOL_TEST)
#error POOL_TEST must be defined in POOL_TO_STRING trait.
#endif
#if defined(POOL_TO_STRING_NAME) && !defined(POOL_TO_STRING)
#error POOL_TO_STRING_NAME requires POOL_TO_STRING.
#endif


#if POOL_INTERFACES == 0 /* <!-- base code */


/* <Kernighan and Ritchie, 1988, p. 231>. */
#if defined(T_) || defined(PT_) || (defined(POOL_CHILD) \
	^ (defined(CAT) || defined(CAT_) || defined(PCAT) || defined(PCAT_)))
#error Unexpected P?T_ or P?CAT_?; possible stray POOL_EXPECT_TRAIT?
#endif
#ifndef POOL_CHILD /* <!-- !sub-type */
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#endif /* !sub-type --> */
#define T_(thing) CAT(POOL_NAME, thing)
#define PT_(thing) PCAT(array, PCAT(POOL_NAME, thing))

/** A valid tag type set by `POOL_TYPE`. This becomes `T`. */
typedef POOL_TYPE PT_(Type);
#define T PT_(Type)

/** Operates by side-effects on `data` only. */
typedef void (*PT_(Action))(T *const data);

/** Responsible for turning the first argument into a 12-`char` null-terminated
 output string. Used for `POOL_TO_STRING`. */
typedef void (*PT_(ToString))(const T *, char (*)[12]);

/* Free-list item. The reason it's doubly-linked is to support popping a link
 from the end. fixme: why `<PT>X` when one can have `<PT>Node`? */
struct PT_(X) { struct PT_(X) *prev, *next; };

/* Nodes containing the data and, in the largest block, the free list item;
 smaller blocks satisfy `x.prev, x.next -> deleted`. */
struct PT_(Node) { T data; struct PT_(X) x; };

/* Information about each block. Blocks will have capacity <tag:<PT>Node>'s
 after `block + 1`, specified by <fn:<PT>_block_nodes>. */
struct PT_(Block) { struct PT_(Block) *smaller; size_t capacity, size; };

/** Zeroed data is a valid state. To instantiate to an idle state, see
 <fn:<T>Pool>, `POOL_IDLE`, `{0}` (`C99`,) or being `static`.

 ![States.](../web/states.png) */
struct T_(Pool);
struct T_(Pool) {
	/* Ideally, all items to go in here, but there may be smaller blocks. */
	struct PT_(Block) *largest;
	/* Fibonacci; largest -> (c0 < c1 || c0 == c1 == max_size). */
	size_t next_capacity;
	/* {0,0} -> no nodes removed from the largest block, otherwise, it's a
	 circular list of items in the largest block. All other states invalid. */
	struct PT_(X) removed;
};
/* `{0}` is `C99`. */
#ifndef POOL_IDLE /* <!-- !zero */
#define POOL_IDLE { 0, 0, { 0, 0 } }
#endif /* !zero --> */

/** Contains all iteration parameters in one. */
struct PT_(Iterator);
struct PT_(Iterator)
	{ const struct T_(Pool) *pool; struct PT_(Block) *block; size_t i; };

/** Container of `data`. */
static struct PT_(Node) *PT_(data_upcast)(T *const data)
	{ return (struct PT_(Node) *)(void *)
	((char *)data - offsetof(struct PT_(Node), data)); }

/** Container of `x`. */
static struct PT_(Node) *PT_(x_upcast)(struct PT_(X) *const x)
	{ return (struct PT_(Node) *)(void *)
	((char *)x - offsetof(struct PT_(Node), x)); }

/** `b` to array. */
static struct PT_(Node) *PT_(block_nodes)(struct PT_(Block) *const b)
	{ return (struct PT_(Node) *)(void *)(b + 1); }

/** Initialises `pool` to idle. */
static void PT_(pool)(struct T_(Pool) *const pool)
	{ assert(pool), pool->largest = 0, pool->next_capacity = 0,
	pool->removed.prev = pool->removed.next = 0; }

/** Destroys `pool` and returns it to idle. */
static void PT_(pool_)(struct T_(Pool) *const pool) {
	struct PT_(Block) *block, *next;
	assert(pool);
	for(block = pool->largest; block; block = next)
		next = block->smaller, free(block);
	PT_(pool)(pool);
}

/** Ensures `min` capacity of the largest block in `pool` where the free list
 is empty.
 @param[min] If zero, allocates anyway.
 @return Success; otherwise, `errno` will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `malloc`
 doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/malloc.html).
 @throws[malloc] */
static int PT_(reserve)(struct T_(Pool) *const pool, const size_t min) {
	size_t c0, c1;
	struct PT_(Block) *block;
	const size_t max_size = ((size_t)-1 - sizeof *block)
		/ sizeof(struct PT_(Node));
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
	if(!(block = malloc(sizeof *block + c0 * sizeof(struct PT_(Node)))))
		{ if(!errno) errno = ERANGE; return 0; }
	block->smaller = pool->largest;
	block->capacity = c0;
	block->size = 0;
	pool->largest = block;
	pool->next_capacity = c1;
	pool->removed.prev = pool->removed.next = 0;
	return 1;
}

/** We are very lazy and enqueue `node` in `pool` so that later data can
 overwrite it. */
static void PT_(enqueue_removed)(struct T_(Pool) *const pool,
	struct PT_(Node) *const node) {
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
static struct PT_(Node) *PT_(dequeue_removed)(struct T_(Pool) *const pool) {
	struct PT_(X) *x0, *x1;
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
static void PT_(trim_removed)(struct T_(Pool) *const pool) {
	struct PT_(Node) *node;
	struct PT_(Block) *const block = pool->largest;
	struct PT_(Node) *const nodes = PT_(block_nodes)(block);
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
static size_t PT_(range)(const struct T_(Pool) *const pool,
	const struct PT_(Block) *const block) {
	assert(pool && block);
	return block == pool->largest ? block->size : block->capacity;
}

/** Find what block `node` is in `pool`. Used in <fn:<T>PoolRemove>.
 @order At worst \O(log `items`) when there's no deletetion.
 @return Must return a value. */
static struct PT_(Block) **PT_(find_block_addr)(struct T_(Pool) *const pool,
	const struct PT_(Node) *const node) {
	struct PT_(Block) *b, **baddr;
	struct PT_(Node) *n;
	assert(pool && node);
	for(baddr = &pool->largest, b = *baddr; b; baddr = &b->smaller, b = *baddr)
		if(n = PT_(block_nodes)(b), node >= n && node < n + b->capacity) break;
	return baddr;
}

/** Only for the not-largest, inactive, blocks, `node` becomes a boolean,
 null/not. */
static void PT_(flag_removed)(struct PT_(Node) *const node)
	{ assert(node); node->x.prev = node->x.next = &node->x; }

/** @return New node from `pool`. @throws[malloc] */
static struct PT_(Node) *PT_(new)(struct T_(Pool) *const pool) {
	struct PT_(Node) *node;
	size_t size;
	assert(pool);
	if((node = PT_(dequeue_removed)(pool))) return node;
	size = pool->largest ? pool->largest->size : 0;
	if(!PT_(reserve)(pool, size + 1)) return 0;
	assert(pool->largest);
	node = PT_(block_nodes)(pool->largest) + pool->largest->size++;
	node->x.prev = node->x.next = 0;
	return node;
}

/** Remove `datum` item from `pool`. @return Success. @throws[EDOM] */
static int PT_(remove)(struct T_(Pool) *const pool, T *const datum) {
	struct PT_(Node) *node;
	struct PT_(Block) *block, **baddr;
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

/** Removes all from `pool`. */
static void PT_(clear)(struct T_(Pool) *const pool) {
	struct PT_(Block) *block, *next;
	assert(pool); if(!pool->largest) return;
	block = pool->largest, next = block->smaller;
	block->size = 0, block->smaller = 0;
	pool->removed.prev = pool->removed.next = 0;
	while(next) block = next, next = next->smaller, free(block);
}

#ifndef POOL_CHILD /* <!-- !sub-type */

/** Initialises `pool` to be empty. @order \Theta(1) @allow */
static void T_(Pool)(struct T_(Pool) *const pool)
	{ if(pool) PT_(pool)(pool); }

/** Returns `pool` to the empty state where it takes no dynamic memory.
 @param[pool] If null, does nothing. @order \Theta(`blocks`) @allow */
static void T_(Pool_)(struct T_(Pool) *const pool)
	{ if(pool) PT_(pool_)(pool); }

/** New item from `pool`.
 @param[pool] If is null, returns null.
 @return A new element at the end, or null and `errno` will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `malloc`
 doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/malloc.html).
 @throws[malloc] @order amortised O(1) @allow */
static T *T_(PoolNew)(struct T_(Pool) *const pool) {
	struct PT_(Node) *n;
	n = pool ? PT_(new)(pool) : 0;
	return n ? &n->data : 0;
}

/** Removes `data` from `pool`.
 @param[pool, data] If null, returns false.
 @return Success, otherwise `errno` will be set for valid input.
 @throws[EDOM] `data` is not part of `pool`.
 @order Amortised \O(1), if the pool is in steady-state, but \O(log `items`)
 for a small number of deleted items. @allow */
static int T_(PoolRemove)(struct T_(Pool) *const pool, T *const data)
	{ return pool && data ? PT_(remove)(pool, data) : 0; }

/** Removes all from `pool`. Keeps it's active state, only freeing the smaller
 blocks. Compare <fn:<T>Pool_>.
 @param[pool] If null, does nothing. @order \O(`blocks`) @allow */
static void T_(PoolClear)(struct T_(Pool) *const pool)
	{ if(pool) PT_(clear)(pool); }

/** Pre-sizes an idle pool to ensure that it can hold at least `min` elements.
 @param[pool] If null, returns false.
 @param[min] If zero, doesn't do anything and returns true.
 @return Success; the pool becomes active with at least `min` elements.
 @throws[EDOM] The pool is active and doesn't allow reserving.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `malloc`
 doesn't follow [IEEE Std 1003.1-2001
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/malloc.html).
 @throws[malloc] @allow */
static int T_(PoolReserve)(struct T_(Pool) *const pool, const size_t min) {
	if(!pool) return 0;
	if(pool->largest) return errno = EDOM, 0;
	if(!min) return 1;
	return PT_(reserve)(pool, min);
}

/** Iterates though `pool` and calls `action` on all the elements. There is no
 way to change the iteration order.
 @param[pool, action] If null, does nothing.
 @order O(`capacity` \times `action`) @allow */
static void T_(PoolForEach)(struct T_(Pool) *const pool,
	const PT_(Action) action) {
	struct PT_(Node) *n, *end;
	struct PT_(Block) *block;
	if(!pool || !action) return;
	for(block = pool->largest; block; block = block->smaller)
		for(n = PT_(block_nodes)(block), end = n + PT_(range)(pool, block);
			n < end; n++) if(!n->x.prev) action(&n->data);
}

#endif /* !sub-type --> */

static void PT_(unused_base_coda)(void);
static void PT_(unused_base)(void) {
	PT_(pool)(0); PT_(pool_)(0); PT_(reserve)(0, 0);
	PT_(enqueue_removed)(0, 0); PT_(dequeue_removed)(0); PT_(trim_removed)(0);
	PT_(range)(0, 0); PT_(find_block_addr)(0, 0); PT_(flag_removed)(0);
#ifndef POOL_CHILD /* <!-- !sub-type */
	T_(Pool)(0); T_(Pool_)(0); T_(PoolNew)(0); T_(PoolRemove)(0, 0);
	T_(PoolClear)(0); T_(PoolReserve)(0, 0); T_(PoolForEach)(0, 0);
#endif /* !sub-type --> */
	PT_(unused_base_coda)();
}
static void PT_(unused_base_coda)(void) { PT_(unused_base)(); }


#elif defined(POOL_TO_STRING) /* base code --><!-- to string trait */


#if !defined(T) || !defined(T_) || !defined(PT_) || !defined(CAT) \
	|| !defined(CAT_) || !defined(PCAT) || !defined(PCAT_)
#error P?T_ or P?CAT_? not yet defined; traits must be defined separately?
#endif

#ifdef POOL_TO_STRING_NAME /* <!-- name */
#define PTA_(thing) PCAT(PT_(thing), POOL_TO_STRING_NAME)
#define T_A_(thing1, thing2) CAT(T_(thing1), CAT(POOL_TO_STRING_NAME, thing2))
#else /* name --><!-- !name */
#define PTA_(thing) PCAT(PT_(thing), anonymous)
#define T_A_(thing1, thing2) CAT(T_(thing1), thing2)
#endif /* !name --> */

/* Check that `POOL_TO_STRING` is a function implementing
 <typedef:<PT>ToString>. */
static const PT_(ToString) PTA_(to_str12) = (POOL_TO_STRING);

/** Writes `it` to `str` and advances or returns false.
 @implements <AI>NextToString */
static int PTA_(next_to_str12)(struct PT_(Iterator) *const it,
	char (*const str)[12]) {
	struct PT_(Node) *nodes, *n;
	size_t i_end;
	assert(it && str && it->pool);
	while(it->block) {
		nodes = PT_(block_nodes)(it->block);
		i_end = PT_(range)(it->pool, it->block);
		while(it->i < i_end) {
			if((n = nodes + it->i++)->x.prev) continue;
			/* Found the data. */
			PTA_(to_str12)(&n->data, str);
			return 1;
		}
		it->block = it->block->smaller;
	}
	return 0;
}

/** @return If `it` contains a not-null pool. */
static int PTA_(is_valid)(const struct PT_(Iterator) *const it)
	{ assert(it); return !!it->pool; }

#define AI_ PTA_
#define TO_STRING_ITERATOR struct PT_(Iterator)
#define TO_STRING_NEXT &PTA_(next_to_str12)
#define TO_STRING_IS_VALID &PTA_(is_valid)
#include "ToString.h"

/** @return Prints `pool`. */
static const char *PTA_(to_string)(const struct T_(Pool) *const pool) {
	struct PT_(Iterator) it = { 0, 0, 0 };
	it.pool = pool, it.block = pool ? pool->largest : 0;
	return PTA_(iterator_to_string)(&it, '[', ']'); /* In ToString. */
}

#ifndef POOL_CHILD /* <!-- !sub-type */

/** @return Print the contents of `pool` in a static string buffer with the
 limitations of `ToString.h`. @order \Theta(1) @allow */
static const char *T_A_(Pool, ToString)(const struct T_(Pool) *const pool)
	{ return PTA_(to_string)(pool); /* Can be null. */ }

#endif /* !sub-type --> */

static void PTA_(unused_to_string_coda)(void);
static void PTA_(unused_to_string)(void) {
	PTA_(to_string)(0);
#ifndef POOL_CHILD /* <!-- !sub-type */
	T_A_(Pool, ToString)(0);
#endif /* !sub-type --> */
	PTA_(unused_to_string_coda)();
}
static void PTA_(unused_to_string_coda)(void) { PTA_(unused_to_string)(); }

#if !defined(POOL_TEST_BASE) && defined(POOL_TEST) /* <!-- test */
#define POOL_TEST_BASE /* Only one instance of base tests. */
#include "../test/TestPool.h" /** \include */
#endif /* test --> */

#undef PTA_
#undef T_A_
#undef POOL_TO_STRING
#ifdef POOL_TO_STRING_NAME
#undef POOL_TO_STRING_NAME
#endif


#endif /* traits --> */


#ifdef POOL_EXPECT_TRAIT /* <!-- unfinish */
#undef POOL_EXPECT_TRAIT
#else /* unfinish --><!-- finish */
#ifndef POOL_CHILD /* <!-- !sub-type */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#else /* !sub-type --><!-- sub-type */
#undef POOL_CHILD
#endif /* sub-type --> */
#undef T
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
#endif /* finish --> */

#undef POOL_TO_STRING_INTERFACE
#undef POOL_INTERFACES

/** @license 2016 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Parameterised Stable Pool

 ![Example of Pool][../image/index.png]

 <tag:<T>Pool> stores unordered `<T>` in a memory pool, which must be set using
 `POOL_TYPE`. Pointers to valid items in the pool are stable, and as such,
 contiguity is not possible. However, it uses geometrically increasing
 size-blocks and when the removal is ongoing and uniformly sampled,
 (specifically, old elements are all removed,) and data reaches a
 steady-state size, the data will eventually be in one allocated region.
 In this way, provides a fairly contiguous space for items to which there might
 have permanent references or hierarchical structures with different sizes.

 `<T>Pool` is not synchronised. Errors are returned with `errno`. The
 parameters are preprocessor macros, and are all undefined at the end of the
 file for convenience. `assert.h` is included in this file; to stop the
 debug assertions, use `#define NDEBUG` before `assert.h`.

 ![States.](../web/states.png)

 @param[POOL_NAME, POOL_TYPE]
 `<T>` that satisfies `C` naming conventions when mangled and a valid tag
 (type) associated therewith; required. `<PT>` is private, whose names are
 prefixed in a manner to avoid collisions; any should be re-defined prior to
 use elsewhere.

 @param[POOL_TO_STRING]
 Optional print function implementing <typedef:<PT>ToString>; makes available
 <fn:<T>PoolToString>. Usually this is only used for debugging.

 @param[POOL_TEST]
 Unit testing framework <fn:<T>PoolTest>, included in a separate header,
 <../test/PoolTest.h>. Must be defined equal to a (random) filler function,
 satisfying <typedef:<PT>Action>. Requires `POOL_TO_STRING` and not `NDEBUG`.

 @std C89
 @cf [Array](https://github.com/neil-edelman/Array)
 @cf [Digraph](https://github.com/neil-edelman/Digraph)
 @cf [List](https://github.com/neil-edelman/List)
 @cf [Orcish](https://github.com/neil-edelman/Orcish)
 @cf [Pool](https://github.com/neil-edelman/Pool)
 @cf [Set](https://github.com/neil-edelman/Set) */

#include <stddef.h>	/* offset_of */
#include <stdlib.h>	/* malloc free */
#include <assert.h>	/* assert */
#include <string.h>	/* memcpy (memmove strerror strcpy memcmp in PoolTest.h) */
#include <errno.h>	/* errno */
#ifdef POOL_TO_STRING /* <-- print */
#include <stdio.h>	/* strlen */
#endif /* print --> */

/* Check defines. */
#ifndef POOL_NAME /* <-- error */
#error Generic POOL_NAME undefined.
#endif /* error --> */
#ifndef POOL_TYPE /* <-- error */
#error Generic POOL_TYPE undefined.
#endif /* --> */
#if defined(POOL_TEST) && !defined(POOL_TO_STRING) /* <-- error */
#error POOL_TEST requires POOL_TO_STRING.
#endif /* error --> */

/* Generics using the preprocessor;
 <http://stackoverflow.com/questions/16522341/pseudo-generics-in-c>. */
#ifdef CAT
#undef CAT
#endif
#ifdef CAT_
#undef CAT_
#endif
#ifdef PCAT
#undef PCAT
#endif
#ifdef PCAT_
#undef PCAT_
#endif
#ifdef T
#undef T
#endif
#ifdef T_
#undef T_
#endif
#ifdef PT_
#undef PT_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define T_(thing) CAT(POOL_NAME, thing)
#define PT_(thing) PCAT(pool, PCAT(POOL_NAME, thing))


/* Troubles with this line? check to ensure that `POOL_TYPE` is a valid type,
 whose definition is placed above `#include "Pool.h"`. */
typedef POOL_TYPE PT_(Type);
#define T PT_(Type)

#ifdef POOL_TO_STRING /* <-- string */
/** Responsible for turning `<T>` (the first argument) into a 12 `char`
 null-terminated output string (the second.) Used for `POOL_TO_STRING`. */
typedef void (*PT_(ToString))(const T *, char (*)[12]);
/* Check that `POOL_TO_STRING` is a function implementing
 <typedef:<PT>ToString>, whose definition is placed above
 `#include "Pool.h"`. */
static const PT_(ToString) PT_(to_string) = (POOL_TO_STRING);
#endif /* string --> */

/** Operates by side-effects on `data` only. */
typedef void (*PT_(Action))(T *const data);

/* Free-list item. The reason it's doubly-linked is to support popping a link
 from the end. fixme: why `<PT>X` when one can have `<PT>Node`? */
struct PT_(X) { struct PT_(X) *prev, *next; };

/* Nodes containing the data and, in the largest block, the free list item;
 smaller blocks satisfy `x.prev, x.next -> deleted`. */
struct PT_(Node) { T data; struct PT_(X) x; };

/* Information about each block. Blocks will have capacity <tag:<PT>Node>'s
 after `block + 1`, specified by <fn:<PT>_block_nodes>. */
struct PT_(Block) {
	struct PT_(Block) *smaller;
	size_t capacity, size;
};

/** The pool. Zeroed data is a valid state. To instantiate explicitly, see
 <fn:<T>Pool> or initialise it with `POOL_INIT` or `{0}` (C99.) */
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
#ifndef POOL_ZERO /* <!-- !zero */
#define POOL_ZERO { 0, 0, { 0, 0 } }
#endif /* !zero --> */


/** Private: `container_of` `data`. */
static struct PT_(Node) *PT_(data_upcast)(T *const data) {
	return (struct PT_(Node) *)(void *)
		((char *)data - offsetof(struct PT_(Node), data));
}

/** Private: `container_of` `x`. */
static struct PT_(Node) *PT_(x_upcast)(struct PT_(X) *const x) {
	return (struct PT_(Node) *)(void *)
		((char *)x - offsetof(struct PT_(Node), x));
}

/** Private: `b` to array. */
static struct PT_(Node) *PT_(block_nodes)(struct PT_(Block) *const b) {
	return (struct PT_(Node) *)(void *)(b + 1);
}

/** Ensures `min` (> 0) capacity of the largest block in `pool` where the free
 list is empty.
 @param[min] If zero, allocates anyway.
 @return Success; otherwise, `errno` will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `malloc`
 doesn't follow [IEEE Std 1003.1-2001
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

/** Zeros `pool`. */
static void PT_(pool)(struct T_(Pool) *const pool) {
	assert(pool);
	pool->largest       = 0;
	pool->next_capacity = 0;
	pool->removed.prev  = 0;
	pool->removed.next  = 0;
}

/** Returns `pool` to the empty state where it takes no dynamic memory.
 @param[pool] If null, does nothing.
 @order \Theta(`blocks`)
 @allow */
static void T_(Pool_)(struct T_(Pool) *const pool) {
	struct PT_(Block) *block, *next;
	if(!pool) return;
	for(block = pool->largest; block; block = next)
		next = block->smaller, free(block);
	PT_(pool)(pool);
}

/** Initialises `pool` to be empty.
 @order \Theta(1)
 @allow */
static void T_(Pool)(struct T_(Pool) *const pool) {
	if(!pool) return;
	PT_(pool)(pool);
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
static void PT_(flag_removed)(struct PT_(Node) *const node) {
	assert(node);
	node->x.prev = node->x.next = &node->x;
}

/** Removes `data` from `pool`.
 @param[pool, data] If null, returns false.
 @return Success, otherwise `errno` will be set for valid input.
 @throws[EDOM] `data` is not part of `pool`.
 @order Amortised \O(1), if the pool is in steady-state, but \O(log `items`)
 for a small number of deleted items.
 @allow */
static int T_(PoolRemove)(struct T_(Pool) *const pool, T *const data) {
	struct PT_(Node) *node;
	struct PT_(Block) *block, **baddr;
	if(!pool || !data) return 0;
	node = PT_(data_upcast)(data);
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

/** Removes all from `pool`. Keeps it's active state, only freeing the smaller
 blocks. Compare <fn:<T>Pool_>.
 @param[pool] If null, does nothing.
 @order \O(`blocks`)
 @allow */
static void T_(PoolClear)(struct T_(Pool) *const pool) {
	struct PT_(Block) *block, *next;
	if(!pool || !pool->largest) return;
	block = pool->largest;
	next = block->smaller;
	block->size = 0;
	block->smaller = 0;
	pool->removed.prev = pool->removed.next = 0;
	while(next) block = next, next = next->smaller, free(block);
}

/** Pre-sizes a zeroed pool to ensure that it can hold at least `min` elements.
 Will not work unless the pool is in an empty state.
 @param[pool] If null, returns false.
 @param[min] If zero, doesn't do anything and returns true.
 @return Success; the pool becomes active with at least `min` elements.
 @throws[EDOM] The pool is active and doesn't allow reserving.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `malloc`
 doesn't follow [IEEE Std 1003.1-2001
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/malloc.html).
 @throws[malloc]
 @allow */
static int T_(PoolReserve)(struct T_(Pool) *const pool, const size_t min) {
	if(!pool) return 0;
	if(pool->largest) return errno = EDOM, 0;
	if(!min) return 1;
	return PT_(reserve)(pool, min);
}

/** New item.
 @param[pool] If is null, returns null.
 @return A new element at the end, or null and `errno` will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `malloc`
 doesn't follow [IEEE Std 1003.1-2001
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/malloc.html).
 @throws[malloc]
 @order amortised O(1)
 @allow */
static T *T_(PoolNew)(struct T_(Pool) *const pool) {
	struct PT_(Node) *node;
	size_t size;
	if(!pool) return 0;
	if((node = PT_(dequeue_removed)(pool))) return &node->data;
	size = pool->largest ? pool->largest->size : 0;
	if(!PT_(reserve)(pool, size + 1)) return 0;
	assert(pool->largest);
	node = PT_(block_nodes)(pool->largest) + pool->largest->size++;
	node->x.prev = node->x.next = 0;
	return &node->data;
}

/** Iterates though `pool` and calls `action` on all the elements. There is no
 way to change the iteration order.
 @param[pool, action] If null, does nothing.
 @order O(`capacity` \times `action`)
 @allow */
static void T_(PoolForEach)(struct T_(Pool) *const pool,
	const PT_(Action) action) {
	struct PT_(Node) *n, *end;
	struct PT_(Block) *block;
	if(!pool || !action) return;
	for(block = pool->largest; block; block = block->smaller) {
		for(n = PT_(block_nodes)(block), end = n + PT_(range)(pool, block);
			n < end; n++) {
			if(n->x.prev) continue;
			action(&n->data);
		}
	}
}

#ifdef POOL_TO_STRING /* <-- print */

/** Can print 4 things at once before it overwrites. One must pool
 `POOL_TO_STRING` to a function implementing <typedef:<PT>ToString> to get this
 functionality.
 @return Prints `pool` in a static buffer.
 @order \Theta(1); it has a 255 character limit; every element takes some of it.
 @allow */
static const char *T_(PoolToString)(const struct T_(Pool) *const pool) {
	static char buffers[4][256];
	static size_t buffer_i;
	char *const buffer = buffers[buffer_i++], *b = buffer;
	const size_t buffers_no = sizeof buffers / sizeof *buffers,
		buffer_size = sizeof *buffers / sizeof **buffers;
	struct PT_(Block) *block;
	struct PT_(Node) *n, *n_end;
	const char start = '[', comma = ',', space = ' ', end = ')',
		*const ellipsis_end = ",…]", *const null = "null",
		*const idle = "idle";
	const size_t ellipsis_end_len = strlen(ellipsis_end),
		null_len = strlen(null), idle_len = strlen(idle);
	size_t i;
	int is_first = 1;
	assert(!(buffers_no & (buffers_no - 1)) && ellipsis_end_len >= 1
		&& buffer_size >= 1 + 11 + ellipsis_end_len + 1
		&& buffer_size >= null_len + 1);
	buffer_i &= buffers_no - 1;
	/* Null array. */
	if(!pool)
		{ memcpy(b, null, null_len), b += null_len; goto terminate; }
	if(!pool->largest)
		{ memcpy(b, idle, idle_len), b += idle_len; goto terminate; }
	/* Otherwise. */
	*b++ = start;
	for(block = pool->largest; block; block = block->smaller) {
		for(n = PT_(block_nodes)(block), n_end = n + PT_(range)(pool, block);
			n < n_end; n++) {
			if(n->x.prev) continue;
			if(!is_first) *b++ = comma, *b++ = space;
			else is_first = 0;
			PT_(to_string)(&n->data, (char (*)[12])b);
			for(i = 0; *b != '\0' && i < 12; b++, i++);
			/* No way to tell if it is going to end; this leads to having ...
			 even if there's nothing there. */
			if((size_t)(b - buffer)
				> buffer_size - 2 - 11 - ellipsis_end_len - 1) goto ellipsis;
		}
	}
	*b++ = end;
ellipsis:
	memcpy(b, ellipsis_end, ellipsis_end_len), b += ellipsis_end_len;
terminate:
	*b++ = '\0';
	assert(b <= buffer + buffer_size);
	return buffer;
}

#endif /* print --> */

#ifdef POOL_TEST /* <-- test */
#include "../test/TestPool.h" /** \include */
#endif /* test --> */

/* Prototype. */
static void PT_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation
 <http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code>. */
static void PT_(unused_set)(void) {
	T_(Pool_)(0);
	T_(Pool)(0);
	T_(PoolRemove)(0, 0);
	T_(PoolClear)(0);
	T_(PoolReserve)(0, 0);
	T_(PoolNew)(0);
	T_(PoolForEach)(0, 0);
#ifdef POOL_TO_STRING
	T_(PoolToString)(0);
#endif
	PT_(unused_coda)();
}
/** Some pre-processors are not fooled if you have one function. */
static void PT_(unused_coda)(void) { PT_(unused_set)(); }



/* Un-define all macros. */
#undef POOL_NAME
#undef POOL_TYPE
/* Undocumented; allows nestled inclusion so long as: `CAT_`, _etc_ conform,
 and `T`, _etc_ is not used. */
#ifdef POOL_SUBTYPE /* <-- sub */
#undef POOL_SUBTYPE
#else /* sub --><-- !sub */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#endif /* !sub --> */
#undef T
#undef T_
#undef PT_
#ifdef POOL_TO_STRING
#undef POOL_TO_STRING
#endif
#ifdef POOL_TEST
#undef POOL_TEST
#endif

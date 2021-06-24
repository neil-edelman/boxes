/** @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Stable Pool

 ![Example of Pool](../web/pool.png)

 <tag:<P>pool> stores `<X>` in a memory pool. Pointers to valid items in the
 pool are stable, but not generally in any order or contiguous. It uses
 geometrically increasing size-blocks. When the removal is uniformly sampled,
 and data reaches a steady-state size, it will settle in one allocated region.

 @param[POOL_NAME, POOL_TYPE]
 `<X>` that satisfies `C` naming conventions when mangled and a valid tag type
 associated therewith; required. `<PX>` is private, whose names are prefixed in
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
 equal to a (random) filler function, satisfying <typedef:<PX>action_fn>.
 Output will be shown with the to string trait in which it's defined; provides
 tests for the base code and all later traits.

 @std C89 */

#include <stdlib.h>	/* malloc free */
#include <assert.h>	/* assert */
#include <errno.h>	/* errno */

#ifndef POOL_H /* <!-- idempotent */
#define POOL_H
/* `[2, (SIZE_MAX - sizeof pool_chunk) / sizeof <PX>type]` */
#define POOL_CHUNK_MIN_CAPACITY 8
/** Stable chunk followed by data; explicit naming to avoid confusion. */
struct pool_chunk { size_t size; };
/** A slot is a pointer to a stable chunk. Despite `typedef` ideals, it makes
 the source much more readable to have this instead of a `**chunk` and will
 not compile on error. */
typedef struct pool_chunk *pool_slot;
/* <array.h> and <heap.h> must be in the same directory. */
#define ARRAY_NAME pool_slot
#define ARRAY_TYPE pool_slot
#include "array.h"
/** @return An order on `a`, `b` which specifies a max-heap. */
static int pool_index_compare(const size_t a, const size_t b) { return a < b; }
#define HEAP_NAME pool_free
#define HEAP_TYPE size_t
#define HEAP_COMPARE &pool_index_compare
#include "heap.h"
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
#if POOL_TRAITS != 0 && (!defined(X_) || !defined(CAT) || !defined(CAT_))
#error X_ or CAT_? not yet defined; traits must be defined separately?
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

/** Consists of a map of several chunks of increasing size and a free-list.
 Zeroed data is a valid state. To instantiate to an idle state, see
 <fn:<X>pool>, `POOL_IDLE`, `{0}` (`C99`,) or being `static`.

 ![States.](../web/states.png) */
struct X_(pool);
struct X_(pool) {
	struct pool_slot_array slots; /* Pointers to stable chunks. */
	struct pool_free_heap free0; /* Free-list in chunk-zero. */
	size_t capacity0; /* Capacity of chunk-zero. */
};
/* `{0}` is `C99`. */
#ifndef POOL_IDLE /* <!-- !zero */
#define POOL_IDLE { ARRAY_IDLE, HEAP_IDLE, (size_t)0 }
#endif /* !zero --> */

/** @return Given a pointer to `chunk_size`, return the chunk data. */
static PX_(type) *PX_(data)(struct pool_chunk *const chunk)
	{ return (PX_(type) *)(chunk + 1); }

/** @return Index of sorted slot[1..n] that is higher than `datum` in `pool`.
 The [0] slot is unsorted. @order \O(\log `slots`) */
static size_t PX_(upper)(const struct pool_slot_array *const slots,
	const void *const x) {
	const pool_slot *const base = slots->data;
	size_t n, b0, b1;
	assert(slots && x);
	if(!(n = slots->size)) return 0;
	assert(base);
	if(!--n) return 1;
	/* The last one is a special case: it doesn't have an upper bound. */
	for(b0 = 1, --n; n; n >>= 1) {
		b1 = b0 + (n >> 1);
		if(x < (void *)base[b1]) { continue; }
		else if((void *)base[b1 + 1] <= x) { b0 = b1 + 1; n--; continue; }
		else { return b1 + 1; }
	}
	return b0 + (x < (void *)base[slots->size - 1] ? 0 : 1);
}

/** Which slot is `datum` in `pool`? @order \O(\log \log `items`) */
static size_t PX_(slot)(const struct X_(pool) *const pool,
	const PX_(type) *const datum) {
	pool_slot *const s0 = pool->slots.data;
	PX_(type) *cmp;
	size_t up;
	assert(pool && pool->slots.size && s0 && datum);
	/* One chunk, assume it's in that chunk; first chunk is `capacity0`. */
	if(pool->slots.size <= 1 || (cmp = PX_(data)(s0[0]),
		datum >= cmp && datum < cmp + pool->capacity0))
		return assert(*s0 && (cmp = PX_(data)(s0[0]),
		datum >= cmp && datum < cmp + pool->capacity0)), 0;
	up = PX_(upper)(&pool->slots, datum);
	return assert(up), up - 1;
}

/** Makes sure there are space for `n` further items in `pool`.
 @return Success. */
static int PX_(buffer)(struct X_(pool) *const pool, const size_t n) {
	pool_slot *slot;
	struct pool_chunk *chunk;
	const size_t min_size = POOL_CHUNK_MIN_CAPACITY,
		max_size = ((size_t)-1 - sizeof(struct pool_chunk)) / sizeof(PX_(type));
	size_t c, alloc, insert;
	int is_recycled = 0;
	assert(pool && min_size <= max_size && pool->capacity0 <= max_size &&
		!pool->slots.size && !pool->free0.a.size /* !chunks[0] -> !free0 */
		|| pool->slots.size && pool->slots.data && pool->slots.data[0]
		&& pool->slots.data[0]->size <= pool->capacity0
		&& (!pool->free0.a.size
		|| pool->free0.a.size < pool->slots.data[0]->size
		&& pool->free0.a.data[0] < pool->slots.data[0]->size));

	/* Don't need to do anything. */
	if(!n || pool->slots.size && n <= pool->capacity0
		- pool->slots.data[0]->size + pool->free0.a.size) return 1;
	/* The request is unsatisfiable. */
	if(max_size < n) return errno = ERANGE, 1;
	/* We will make a new slot. */
	if(!pool_slot_array_buffer(&pool->slots, 1)) return 0;

	/* Figure out the size of the next chunk and allocate it. */
	c = pool->capacity0;
	if(pool->slots.size && pool->slots.data[0]->size) { /* ~Golden ratio. */
		size_t c1 = c + (c >> 1) + (c >> 3);
		c = (c1 < c || c1 > max_size) ? max_size : c1;
	}
	if(c < min_size) c = min_size;
	if(c < n) c = n;
	alloc = sizeof chunk + c * sizeof(PX_(type));
	if(pool->slots.size && !pool->slots.data[0]->size)
		is_recycled = 1, chunk = realloc(pool->slots.data[0], alloc);
	else chunk = malloc(alloc);
	if(!chunk) return 0;
	chunk->size = 0;
	pool->capacity0 = c;
	if(is_recycled) return pool->slots.data[0] = chunk, 1;

	/* Add it to the slots, in order. */
	if(!pool->slots.size) insert = 0;
	else insert = PX_(upper)(&pool->slots, pool->slots.data[0]);
	assert(insert <= pool->slots.size);
	slot = pool_slot_array_append_at(&pool->slots, 1, insert);
	assert(slot);
	*slot = pool->slots.data[0], pool->slots.data[0] = chunk;
	return 1;
}

/** Either `data` in `pool` is in a secondary chunk, in which case it
 decrements the size, or it's the zero-chunk, where it gets added to the
 free-heap.
 @return Success. It may fail due to a free-heap memory allocation error.
 @order Amortized \O(\log \log `items`) @throws[realloc] */
static int PX_(remove)(struct X_(pool) *const pool,
	const PX_(type) *const data) {
	size_t s = PX_(slot)(pool, data);
	struct pool_chunk *chunk = pool->slots.data[s];
	assert(pool && pool->slots.size && data);
	if(!s) { /* It's in the zero-slot, we need to deal with the free-heap. */
		const size_t idx = (size_t)(data - PX_(data)(chunk));
		assert(pool->capacity0 && chunk->size <= pool->capacity0
			&& idx < chunk->size);
		if(idx + 1 == chunk->size) { /* It's at the end -- size goes down. */
			while(--chunk->size) {
				const size_t *const free = pool_free_heap_peek(&pool->free0);
				/* Another item on the free-heap is not exposed? */
				if(!free || *free < chunk->size - 1) break;
				assert(*free == chunk->size - 1);
				pool_free_heap_pop(&pool->free0);
			}
		} else if(!pool_free_heap_add(&pool->free0, idx)) return 0;
	} else if(assert(chunk->size), !--chunk->size)
		pool_slot_array_remove(&pool->slots, pool->slots.data + s), free(chunk);
	return 1;
}

/** Initializes `pool` to idle. @order \Theta(1) @allow */
static void X_(pool)(struct X_(pool) *const pool) { assert(pool),
	pool_slot_array(&pool->slots), pool_free_heap(&pool->free0),
	pool->capacity0 = 0; }

/** Destroys `pool` and returns it to idle. @order \O(\log `data`) @allow */
static void X_(pool_)(struct X_(pool) *const pool) {
	pool_slot *i, *i_end;
	assert(pool);
	for(i = pool->slots.data, i_end = i + pool->slots.size; i < i_end; i++)
		assert(*i), free(*i);
	pool_slot_array_(&pool->slots);
	pool_free_heap_(&pool->free0);
	X_(pool)(pool);
}

/** Ensure capacity of at least `n` items in `pool`. Pre-sizing is better for
 contiguous blocks. @return Success. @throws[ERANGE, malloc] @allow */
static int X_(pool_buffer)(struct X_(pool) *const pool, const size_t n) {
	return assert(pool), PX_(buffer)(pool, n);
}

/** This pointer is constant until it gets removed.
 @return A pointer to a new uninitialized element from `pool`.
 @throws[ERANGE, malloc] @order amortised O(1) @allow */
static PX_(type) *X_(pool_new)(struct X_(pool) *const pool) {
	size_t *free;
	struct pool_chunk *chunk0;
	assert(pool);
	if(!PX_(buffer)(pool, 1)) return 0;
	assert(pool->slots.size && (pool->free0.a.size ||
		pool->slots.data[0]->size < pool->capacity0));
	/* Array pop, towards minimum-ish index in the max-free-heap. */
	if(free = heap_pool_free_node_array_pop(&pool->free0.a))
		return assert(free), PX_(data)(pool->slots.data[0]) + *free;
	/* The free-heap is empty; guaranteed by <fn:<PX>buffer>. */
	chunk0 = pool->slots.data[0];
	assert(chunk0->size < pool->capacity0);
	return PX_(data)(chunk0) + chunk0->size++;
}

/** Deletes `datum` from `pool`. Do not remove data that is not in `pool`.
 @return Success. @order \O(\log \log `items`) @allow */
static int X_(pool_remove)(struct X_(pool) *const pool,
	PX_(type) *const datum) { return PX_(remove)(pool, datum); }

/** Removes all from `pool`, but keeps it's active state, only freeing the
 smaller blocks. @order \O(\log `items`) @allow */
static void X_(pool_clear)(struct X_(pool) *const pool) {
	pool_slot *i, *i_end;
	assert(pool);
	if(!pool->slots.size) { assert(!pool->free0.a.size); return; }
	for(i = pool->slots.data + 1, i_end = i-1+pool->slots.size; i < i_end; i++)
		assert(*i), free(*i);
	pool->slots.data[0]->size = 0;
	pool->slots.size = 1;
	pool_free_heap_clear(&pool->free0);
}

/** It's not actually possible to iterate though, given the information that
 we have, but placing it here for testing purposes. Iterates through the
 zero-slot, ignoring the free list. Do not call. */
struct PX_(iterator);
struct PX_(iterator) { struct pool_chunk *chunk0; size_t i; };

/** Loads `pool` into `it`. @implements begin */
static void PX_(begin)(struct PX_(iterator) *const it,
	const struct X_(pool) *const pool) {
	assert(it && pool);
	if(pool->slots.size) it->chunk0 = pool->slots.data[0];
	else it->chunk0 = 0;
	it->i = 0;
}

/** Advances `it`. @implements next */
static const PX_(type) *PX_(next)(struct PX_(iterator) *const it) {
	assert(it);
	return it->chunk0 && it->i < it->chunk0->size
		? PX_(data)(it->chunk0) + it->i++ : 0;
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

static void PX_(unused_base_coda)(void);
static void PX_(unused_base)(void) {
	X_(pool)(0); X_(pool_)(0); X_(pool_buffer)(0, 0); X_(pool_new)(0);
	X_(pool_remove)(0, 0); X_(pool_clear)(0); PX_(begin)(0, 0);
	PX_(next)(0); PX_(unused_base_coda)();
}
static void PX_(unused_base_coda)(void) { PX_(unused_base)(); }


#elif defined(POOL_TO_STRING) /* base code --><!-- to string trait */


#ifdef POOL_TO_STRING_NAME /* <!-- name */
#define Z_(thing) CAT(X_(pool), CAT(POOL_TO_STRING_NAME, thing))
#else /* name --><!-- !name */
#define Z_(thing) CAT(X_(pool), thing)
#endif /* !name --> */
#define TO_STRING POOL_TO_STRING
#include "to_string.h" /** \include */

#if !defined(POOL_TEST_BASE) && defined(POOL_TEST) /* <!-- test */
#define POOL_TEST_BASE /* Only one instance of base tests. */
#include "../test/test_pool.h" /** \include */
#endif /* test --> */

#undef Z_
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

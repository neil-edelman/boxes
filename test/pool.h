/** @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Stable Pool

 ![Example of Pool](../web/pool.png)

 <tag:<P>pool> is a memory pool that stores <typedef:<PP>type>. Pointers to
 valid items in the pool are stable, but not generally in any order or
 contiguous. It uses geometrically increasing size-blocks, so, if data reaches
 a steady-state size, when the removal is uniformly sampled, it will settle in
 one allocated region.

 @param[POOL_NAME, POOL_TYPE]
 `<P>` that satisfies `C` naming conventions when mangled and a valid tag type,
 <typedef:<PP>type>, associated therewith; required. `<PP>` is private, whose
 names are prefixed in a manner to avoid collisions.

 @param[POOL_CHUNK_MIN_CAPACITY]
 Default is 8; optional number in
 `[2, (SIZE_MAX - sizeof pool_chunk) / sizeof <PP>type]` that the capacity can
 not go below.

 @param[POOL_TEST]
 To string trait contained in <../test/pool_test.h>; optional unit testing
 framework using `assert`. Must be defined equal to a (random) filler function,
 satisfying <typedef:<PP>action_fn>. Needs at least one to string trait.

 @param[POOL_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[POOL_TO_STRING_NAME, POOL_TO_STRING]
 To string trait contained in <to_string.h>; `<A>` that satisfies `C` naming
 conventions when mangled and function implementing <typedef:<PSZ>to_string_fn>.
 There can be multiple to string traits, but only one can omit
 `POOL_TO_STRING_NAME`.

 @depend [array](https://github.com/neil-edelman/array)
 @depend [heap](https://github.com/neil-edelman/heap)
 @std C89 */

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
#if defined(POOL_TO_STRING_NAME) && !defined(POOL_TO_STRING)
#error POOL_TO_STRING_NAME requires POOL_TO_STRING.
#endif

#ifndef POOL_H /* <!-- idempotent */
#define POOL_H
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#if defined(POOL_CAT_) || defined(POOL_CAT) || defined(P_) || defined(PP_) \
	|| defined(POOL_IDLE)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define POOL_CAT_(n, m) n ## _ ## m
#define POOL_CAT(n, m) POOL_CAT_(n, m)
#define P_(n) POOL_CAT(POOL_NAME, n)
#define PP_(n) POOL_CAT(pool, P_(n))
#define POOL_IDLE { ARRAY_IDLE, HEAP_IDLE, (size_t)0 }
/** Stable chunk followed by data; explicit naming to avoid confusion. */
struct pool_chunk { size_t size; };
/** A slot is a pointer to a stable chunk. It makes the source much more
 readable to have this instead of a `**chunk`.
 @fixme: This is stupid; have the chunk size in here, then it doesn't have to
 be in the chunk itself, only data. */
typedef struct pool_chunk *poolslot;
/* <array.h> and <heap.h> must be in the same directory. */
#define ARRAY_NAME poolslot
#define ARRAY_TYPE poolslot
#include "array.h"
/** @return An order on `a`, `b` which specifies a max-heap. */
static int pool_index_compare(const size_t a, const size_t b) { return a < b; }
#define HEAP_NAME poolfree
#define HEAP_TYPE size_t
#define HEAP_COMPARE &pool_index_compare
#include "heap.h"
#endif /* idempotent --> */


#if POOL_TRAITS == 0 /* <!-- base code */


#ifndef POOL_CHUNK_MIN_CAPACITY /* <!-- !min */
/* `[2, (SIZE_MAX - sizeof pool_chunk) / sizeof <PP>type]` */
#define POOL_CHUNK_MIN_CAPACITY 8
#endif /* !min --> */

/** A valid tag type set by `POOL_TYPE`. */
typedef POOL_TYPE PP_(type);

/** Consists of a map of several chunks of increasing size and a free-list.
 Zeroed data is a valid state. To instantiate to an idle state, see
 <fn:<P>pool>, `POOL_IDLE`, `{0}` (`C99`,) or being `static`.

 ![States.](../web/states.png) */
struct P_(pool);
struct P_(pool) {
	struct poolslot_array slots; /* Pointers to stable chunks. */
	struct poolfree_heap free0; /* Free-list in chunk-zero. */
	size_t capacity0; /* Capacity of chunk-zero. */
};

/** @return Given a pointer to `chunk`, return the chunk data. */
static PP_(type) *PP_(data)(struct pool_chunk *const chunk)
	{ return (PP_(type) *)(chunk + 1); }

/** @return Index of sorted slot[1..n] that is higher than `x` in `slots`.
 The `[0]` slot is unsorted. @order \O(\log `slots`) */
static size_t PP_(upper)(const struct poolslot_array *const slots,
	const void *const x) {
	const poolslot *const base = slots->data;
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
static size_t PP_(slot)(const struct P_(pool) *const pool,
	const PP_(type) *const datum) {
	poolslot *const s0 = pool->slots.data;
	PP_(type) *cmp;
	size_t up;
	assert(pool && pool->slots.size && s0 && datum);
	/* One chunk, assume it's in that chunk; first chunk is `capacity0`. */
	if(pool->slots.size <= 1 || (cmp = PP_(data)(s0[0]),
		datum >= cmp && datum < cmp + pool->capacity0))
		return assert(*s0 && (cmp = PP_(data)(s0[0]),
		datum >= cmp && datum < cmp + pool->capacity0)), 0;
	up = PP_(upper)(&pool->slots, datum);
	return assert(up), up - 1;
}

/** Makes sure there are space for `n` further items in `pool`.
 @return Success. */
static int PP_(buffer)(struct P_(pool) *const pool, const size_t n) {
	poolslot *slot;
	struct pool_chunk *chunk;
	const size_t min_size = POOL_CHUNK_MIN_CAPACITY,
		max_size = ((size_t)-1 - sizeof(struct pool_chunk)) / sizeof(PP_(type));
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
	if(!poolslot_array_buffer(&pool->slots, 1)) return 0;

	/* Figure out the size of the next chunk and allocate it. */
	c = pool->capacity0;
	if(pool->slots.size && pool->slots.data[0]->size) { /* ~Golden ratio. */
		size_t c1 = c + (c >> 1) + (c >> 3);
		c = (c1 < c || c1 > max_size) ? max_size : c1;
	}
	if(c < min_size) c = min_size;
	if(c < n) c = n;
	alloc = sizeof chunk + c * sizeof(PP_(type));
	if(pool->slots.size && !pool->slots.data[0]->size)
		is_recycled = 1, chunk = realloc(pool->slots.data[0], alloc);
	else chunk = malloc(alloc);
	if(!chunk) return 0;
	chunk->size = 0;
	pool->capacity0 = c;
	if(is_recycled) return pool->slots.data[0] = chunk, 1;

	/* Add it to the slots, in order. */
	if(!pool->slots.size) insert = 0;
	else insert = PP_(upper)(&pool->slots, pool->slots.data[0]);
	assert(insert <= pool->slots.size);
	slot = poolslot_array_append_at(&pool->slots, 1, insert);
	assert(slot);
	*slot = pool->slots.data[0], pool->slots.data[0] = chunk;
	return 1;
}

/** Either `data` in `pool` is in a secondary chunk, in which case it
 decrements the size, or it's the zero-chunk, where it gets added to the
 free-heap.
 @return Success. It may fail due to a free-heap memory allocation error.
 @order Amortized \O(\log \log `items`) @throws[realloc] */
static int PP_(remove)(struct P_(pool) *const pool,
	const PP_(type) *const data) {
	size_t s = PP_(slot)(pool, data);
	struct pool_chunk *chunk = pool->slots.data[s];
	assert(pool && pool->slots.size && data);
	if(!s) { /* It's in the zero-slot, we need to deal with the free-heap. */
		const size_t idx = (size_t)(data - PP_(data)(chunk));
		assert(pool->capacity0 && chunk->size <= pool->capacity0
			&& idx < chunk->size);
		if(idx + 1 == chunk->size) { /* It's at the end -- size goes down. */
			while(--chunk->size) {
				const size_t *const free = poolfree_heap_peek(&pool->free0);
				/* Another item on the free-heap is not exposed? */
				if(!free || *free < chunk->size - 1) break;
				assert(*free == chunk->size - 1);
				poolfree_heap_pop(&pool->free0);
			}
		} else if(!poolfree_heap_add(&pool->free0, idx)) return 0;
	} else if(assert(chunk->size), !--chunk->size)
		poolslot_array_remove(&pool->slots, pool->slots.data + s), free(chunk);
	return 1;
}

/** Initializes `pool` to idle. @order \Theta(1) @allow */
static void P_(pool)(struct P_(pool) *const pool) { assert(pool),
	poolslot_array(&pool->slots), poolfree_heap(&pool->free0),
	pool->capacity0 = 0; }

/** Destroys `pool` and returns it to idle. @order \O(\log `data`) @allow */
static void P_(pool_)(struct P_(pool) *const pool) {
	poolslot *i, *i_end;
	assert(pool);
	for(i = pool->slots.data, i_end = i + pool->slots.size; i < i_end; i++)
		assert(*i), free(*i);
	poolslot_array_(&pool->slots);
	poolfree_heap_(&pool->free0);
	P_(pool)(pool);
}

/** Ensure capacity of at least `n` items in `pool`. Pre-sizing is better for
 contiguous blocks. @return Success. @throws[ERANGE, malloc] @allow */
static int P_(pool_buffer)(struct P_(pool) *const pool, const size_t n) {
	return assert(pool), PP_(buffer)(pool, n);
}

/** This pointer is constant until it gets removed.
 @return A pointer to a new uninitialized element from `pool`.
 @throws[ERANGE, malloc] @order amortised O(1) @allow */
static PP_(type) *P_(pool_new)(struct P_(pool) *const pool) {
	size_t *free;
	struct pool_chunk *chunk0;
	assert(pool);
	if(!PP_(buffer)(pool, 1)) return 0;
	assert(pool->slots.size && (pool->free0.a.size ||
		pool->slots.data[0]->size < pool->capacity0));
	/* Array pop, towards minimum-ish index in the max-free-heap. */
	if(free = heap_poolfree_node_array_pop(&pool->free0.a))
		return assert(free), PP_(data)(pool->slots.data[0]) + *free;
	/* The free-heap is empty; guaranteed by <fn:<PP>buffer>. */
	chunk0 = pool->slots.data[0];
	assert(chunk0->size < pool->capacity0);
	return PP_(data)(chunk0) + chunk0->size++;
}

/** Deletes `datum` from `pool`. Do not remove data that is not in `pool`.
 @return Success. @order \O(\log \log `items`) @allow */
static int P_(pool_remove)(struct P_(pool) *const pool,
	PP_(type) *const datum) { return PP_(remove)(pool, datum); }

/** Removes all from `pool`, but keeps it's active state, only freeing the
 smaller blocks. @order \O(\log `items`) @allow */
static void P_(pool_clear)(struct P_(pool) *const pool) {
	poolslot *i, *i_end;
	assert(pool);
	if(!pool->slots.size) { assert(!pool->free0.a.size); return; }
	for(i = pool->slots.data + 1, i_end = i - 1 + pool->slots.size;
		i < i_end; i++) assert(*i), free(*i);
	pool->slots.data[0]->size = 0;
	pool->slots.size = 1;
	poolfree_heap_clear(&pool->free0);
}

/* <!-- iterate interface: it's not actually possible to iterate though given
 the information that we have, but placing it here for testing purposes.
 Iterates through the zero-slot, ignoring the free list. Do not call. */

struct PP_(iterator);
struct PP_(iterator) { struct pool_chunk *chunk0; size_t i; };

/** Loads `pool` into `it`. @implements begin */
static void PP_(begin)(struct PP_(iterator) *const it,
	const struct P_(pool) *const pool) {
	assert(it && pool);
	if(pool->slots.size) it->chunk0 = pool->slots.data[0];
	else it->chunk0 = 0;
	it->i = 0;
}

/** Advances `it`. @implements next */
static const PP_(type) *PP_(next)(struct PP_(iterator) *const it) {
	assert(it);
	return it->chunk0 && it->i < it->chunk0->size
		? PP_(data)(it->chunk0) + it->i++ : 0;
}

/* iterate --> */

#ifdef POOL_TEST /* <!-- test */
/* Forward-declare. */
static void (*PP_(to_string))(const PP_(type) *, char (*)[12]);
static const char *(*PP_(pool_to_string))(const struct P_(pool) *);
#include "../test/test_pool.h" /** \include */
#endif /* test --> */

/* <!-- box (multiple traits) */
#define BOX_ PP_
#define BOX_CONTAINER struct P_(pool)
#define BOX_CONTENTS PP_(type)

static void PP_(unused_base_coda)(void);
static void PP_(unused_base)(void) {
	P_(pool)(0); P_(pool_)(0); P_(pool_buffer)(0, 0); P_(pool_new)(0);
	P_(pool_remove)(0, 0); P_(pool_clear)(0); PP_(begin)(0, 0);
	PP_(next)(0); PP_(unused_base_coda)();
}
static void PP_(unused_base_coda)(void) { PP_(unused_base)(); }


#elif defined(POOL_TO_STRING) /* base code --><!-- to string trait */


#ifdef POOL_TO_STRING_NAME /* <!-- name */
#define SZ_(n) POOL_CAT(P_(pool), POOL_CAT(POOL_TO_STRING_NAME, n))
#else /* name --><!-- !name */
#define SZ_(n) POOL_CAT(P_(pool), n)
#endif /* !name --> */
#define TO_STRING POOL_TO_STRING
#include "to_string.h" /** \include */
#ifdef POOL_TEST /* <!-- expect: greedy satisfy forward-declared. */
#undef POOL_TEST
static PSZ_(to_string_fn) PP_(to_string) = PSZ_(to_string);
static const char *(*PP_(pool_to_string))(const struct P_(pool) *)
	= &SZ_(to_string);
#endif /* expect --> */
#undef SZ_
#undef POOL_TO_STRING
#ifdef POOL_TO_STRING_NAME
#undef POOL_TO_STRING_NAME
#endif


#endif /* traits --> */


#ifdef POOL_EXPECT_TRAIT /* <!-- trait */
#undef POOL_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifdef POOL_TEST
#error No POOL_TO_STRING traits defined for POOL_TEST.
#endif
#undef POOL_NAME
#undef POOL_TYPE
#undef BOX_
#undef BOX_CONTAINER
#undef BOX_CONTENTS
/* box (multiple traits) --> */
#endif /* !trait --> */
#undef POOL_TO_STRING_TRAIT
#undef POOL_TRAITS

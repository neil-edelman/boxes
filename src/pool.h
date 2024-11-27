/** @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Header <../src/pool.h> depends on <../src/heap.h> and
 <../src/array.h>; examples <../test/test_pool.c>; article
 <../doc/pool/pool.pdf>.

 @subtitle Stable pool

 ![Example of Pool](../doc/pool/pool.png)

 <tag:<P>pool> is a memory pool that stores only one
 size—<typedef:<PP>type>—using
 [slab allocation](https://en.wikipedia.org/wiki/Slab_allocation). As
 <Bonwick, 1994, Slab>, it helps reduce internal fragmentation from repeated
 allocation and destruction by caching contiguous blocks. Pointers to valid
 items in the pool are stable. Instead of freeing memory, a free-heap in the
 active-slab allows lazily reusing the same space. If removal is ongoing and
 uniformly sampled while reaching a steady-state size, it will eventually
 settle in one contiguous region.

 @param[POOL_NAME, POOL_TYPE]
 `<P>` that satisfies `C` naming conventions when mangled and a valid tag type,
 <typedef:<PP>type>, associated therewith; required. `<PP>` is private, whose
 names are prefixed in a manner to avoid collisions.

 @param[POOL_HEAD, POOL_BODY]
 These go together to allow exporting non-static data between compilation units
 by separating the header head from the code body. `POOL_HEAD` needs identical
 `POOL_NAME` and `POOL_TYPE`.

 @depend [array](https://github.com/neil-edelman/array)
 @depend [heap](https://github.com/neil-edelman/heap)
 @std C89. However, when compiling for segmented memory models, C99 with
 `uintptr_t` is recommended because of it's implementation-defined instead of
 undefined-behaviour when comparing pointers from different objects in the
 heap of memory addresses. Still, this is not guaranteed to produce meaningful
 results on all systems. */

/* `POOL_TO_STRING` is undocumented because this container is only iterable in
 the first slab, so this is not very useful except for debugging. */

#if !defined(POOL_NAME) || !defined(POOL_TYPE)
#error Name or tag type undefined.
#endif
#if defined(POOL_TEST) && !defined(POOL_TO_STRING)
#error Test requires to string.
#endif
#if defined POOL_HEAD && defined POOL_BODY
#error Can not be simultaneously defined.
#endif

#ifndef POOL_H /* <!-- idempotent */
#define POOL_H
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#if defined(POOL_CAT_) || defined(POOL_CAT) || defined(P_) || defined(PP_)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define POOL_CAT_(n, m) n ## _ ## m
#define POOL_CAT(n, m) POOL_CAT_(n, m)
#define P_(n) POOL_CAT(POOL_NAME, n)
#define PP_(n) POOL_CAT(pool, P_(n))
#ifndef POOL_HEAD /* <!-- body */
/** @return An order on `a`, `b` which specifies a max-heap. */
static int poolfree_compare(const size_t a, const size_t b) { return a < b; }
#endif /* body --> */
#define BOX_NAME poolfree
#define BOX_TYPE size_t
#define BOX_COMPARE &poolfree_compare
#ifdef POOL_HEAD
#define BOX_DELARE_ONLY
#endif
#include "heap.h"
#if !defined(__STDC__) || !defined(__STDC_VERSION__) \
	|| __STDC_VERSION__ < 199901L /* < C99 */
#define POOL_CAST (const void *)
#else /* < C99 --><!-- >= C99 */
#include <stdint.h>
#define POOL_CAST (const uintptr_t)(const void *)
#endif /* >= C99 --> */
#endif /* idempotent --> */


/* Undocumented: set the initial size. */
#ifndef POOL_SLAB_MIN_CAPACITY /* <!-- !min */
#define POOL_SLAB_MIN_CAPACITY 8
#endif /* !min --> */
#if POOL_SLAB_MIN_CAPACITY < 2
#error Pool slab capacity error.
#endif

#ifndef POOL_BODY /* <!-- head */

/** A valid tag type set by `POOL_TYPE`. */
typedef POOL_TYPE PP_(type);

/* Goes into a slab-sorted array. */
struct PP_(slot) { size_t size; PP_(type) *slab; };
#define ARRAY_NAME PP_(slot)
#define ARRAY_TYPE struct PP_(slot)
#ifdef POOL_HEAD
#define ARRAY_DECLARE_ONLY
#endif
#include "array.h"

/** A zeroed pool is a valid state. To instantiate to an idle state, see
 <fn:<P>pool>, `{0}` (`C99`,) or being `static`.

 ![States.](../doc/pool/states.png) */
struct P_(pool) {
	struct PP_(slot_array) slots;
	struct poolfree_heap free0; /* Free-heap in slab-zero. */
	size_t capacity0; /* Capacity of slab-zero. */
};

/* It is very useful in debugging, is required contract, but only iterates on
 `slot0` and ignores the free-heap. This is a memory-manager, we don't have
 enough information to do otherwise. Only goes one-way. */
struct PP_(iterator) { struct PP_(slot) *slot0; size_t i; };

#endif /* head --> */
#ifndef POOL_HEAD /* <!-- body */

#ifdef POOL_BODY /* <!-- real body: get the array functions, if separate. */
#define ARRAY_NAME PP_(slot)
#define ARRAY_TYPE struct PP_(slot)
#define ARRAY_DEFINE_ONLY
#include "array.h"
#endif /* real body --> */

/** @return Before `p`. @implements `forward` */
static struct PP_(iterator) PP_(iterator)(const struct P_(pool) *const p)
	{ struct PP_(iterator) it; it.slot0 = p && p->slots.data
	? p->slots.data + 0 : 0, it.i = (size_t)~0; return it; }
/** `it` element. */
static PP_(type) *PP_(element)(struct PP_(iterator) *const it)
	{ return it->slot0->slab + it->i; }
/** @return Whether moved to next `it`. @implements `next` */
static int PP_(next)(struct PP_(iterator) *const it) {
	assert(it);
	if(!it->slot0) return 0;
	it->i++;
	return it->i < it->slot0->size;
}

/** @return Index of slot that is higher than `x` in `slots`, but treating zero
 as special. @order \O(\log `slots`) */
static size_t PP_(upper)(const struct PP_(slot_array) *const slots,
	const PP_(type) *const x) {
	const struct PP_(slot) *const base = slots->data;
	size_t n, b0, b1;
	assert(slots && x);
	if(!(n = slots->size)) return 0;
	assert(base);
	if(!--n) return 1;
	/* The last one is a special case: it doesn't have an upper bound. */
	for(b0 = 1, --n; n; n /= 2) {
		b1 = b0 + n / 2;
		/* Cast to `void *` then `uintptr_t` if available. */
		if(POOL_CAST x < POOL_CAST base[b1].slab)
			{ continue; }
		else if(POOL_CAST base[b1 + 1].slab <= POOL_CAST x)
			{ b0 = b1 + 1; n--; continue; }
		else
			{ return b1 + 1; }
	}
	return b0 + ((const void *)x >= (const void *)base[slots->size - 1].slab);
}

/** Which slot contains the slab that has `x` in `pool`?
 @order \O(\log `slots`), \O(\log \log `size`)? */
static size_t PP_(slot_idx)(const struct P_(pool) *const pool,
	const PP_(type) *const x) {
	struct PP_(slot) *const base = pool->slots.data;
	size_t up;
	assert(pool && pool->slots.size && base && x);
	/* There's only one option. */
	if(pool->slots.size <= 1
		|| ((const void *)x >= (const void *)base[0].slab
		&& (const void *)x < (const void *)(base[0].slab + pool->capacity0)))
		return assert(pool->slots.size >= 1
		&& (const void *)x >= (const void *)base[0].slab
		&& (const void *)x < (const void *)(base[0].slab + pool->capacity0)), 0;
	up = PP_(upper)(&pool->slots, x);
	return assert(up), up - 1;
}

/** Makes sure there are space for `n` further items in `pool`.
 @return Success. */
static int PP_(buffer)(struct P_(pool) *const pool, const size_t n) {
	const size_t min_size = POOL_SLAB_MIN_CAPACITY,
		max_size = (size_t)-1 / sizeof(PP_(type));
	struct PP_(slot) *base = pool->slots.data, *slot;
	PP_(type) *slab;
	size_t c, insert;
	int is_recycled = 0;
	assert(pool && min_size <= max_size && pool->capacity0 <= max_size &&
		(!pool->slots.size && !pool->free0.as_array.size
		|| pool->slots.size && base /* !slots[0] -> !free0 */
		&& base[0].size <= pool->capacity0
		&& (!pool->free0.as_array.size
		|| pool->free0.as_array.size < base[0].size
		&& pool->free0.as_array.data[0] < base[0].size)));

	/* Ensure space for new slot. */
	if(!n || pool->slots.size && n <= pool->capacity0
		- base[0].size + pool->free0.as_array.size) return 1; /* Enough. */
	if(max_size < n) return errno = ERANGE, 1; /* Request unsatisfiable. */
	if(!PP_(slot_array_buffer)(&pool->slots, 1)) return 0;
	base = pool->slots.data; /* It may have moved! */

	/* Figure out the capacity of the next slab. */
	c = pool->capacity0;
	if(pool->slots.size && base[0].size) { /* ~Golden ratio. */
		size_t c1 = c + (c >> 1) + (c >> 3);
		c = (c1 < c || c1 > max_size) ? max_size : c1;
	}
	if(c < min_size) c = min_size;
	if(c < n) c = n;

	/* Allocate it; check if the current one is empty. */
	if(pool->slots.size && !base[0].size)
		is_recycled = 1, slab = realloc(base[0].slab, c * sizeof *slab);
	else slab = malloc(c * sizeof *slab);
	if(!slab) { if(!errno) errno = ERANGE; return 0; }
	pool->capacity0 = c; /* We only need to store the capacity of slab 0. */
	if(is_recycled) return base[0].size = 0, base[0].slab = slab, 1;

	/* Evict slot 0. */
	if(!pool->slots.size) insert = 0;
	else insert = PP_(upper)(&pool->slots, base[0].slab);
	assert(insert <= pool->slots.size);
	slot = PP_(slot_array_insert)(&pool->slots, 1, insert);
	assert(slot); /* Made space for it before. */
	slot->slab = base[0].slab, slot->size = base[0].size;
	base[0].slab = slab, base[0].size = 0;
	return 1;
}

/** Either `data` in `pool` is in a secondary slab, in which case it decrements
 the size, or it's the zero-slab, where it gets added to the free-heap.
 @return Success. It may fail due to a free-heap memory allocation error.
 @order Amortized \O(\log \log `items`) @throws[realloc] */
static int PP_(remove)(struct P_(pool) *const pool,
	const PP_(type) *const data) {
	size_t c = PP_(slot_idx)(pool, data);
	struct PP_(slot) *slot = pool->slots.data + c;
	assert(pool && pool->slots.size && data);
	if(!c) { /* It's in the zero-slot, we need to deal with the free-heap. */
		const size_t idx = (size_t)(data - slot->slab);
		assert(pool->capacity0 && slot->size <= pool->capacity0
			&& idx < slot->size);
		if(idx + 1 == slot->size) {
			/* Keep shrinking going while item on the free-heap are exposed. */
			while(--slot->size && poolfree_heap_size(&pool->free0)) {
				const size_t free = *poolfree_heap_peek(&pool->free0);
				if(free < slot->size - 1) break;
				assert(free == slot->size - 1);
				poolfree_heap_pop(&pool->free0);
			}
		} else if(!poolfree_heap_add(&pool->free0, idx)) return 0;
	} else if(assert(slot->size), !--slot->size) {
		PP_(type) *const slab = slot->slab;
		PP_(slot_array_remove)(&pool->slots, pool->slots.data + c);
		free(slab);
	}
	return 1;
}

/** @return An idle pool. @order \Theta(1) @allow */
static struct P_(pool) P_(pool)(void) { struct P_(pool) p;
	p.slots = PP_(slot_array)(), p.free0 = poolfree_heap(), p.capacity0 = 0;
	return p; }

/** Destroys `pool` and returns it to idle. @order \O(\log `data`) @allow */
static void P_(pool_)(struct P_(pool) *const pool) {
	struct PP_(slot) *s, *s_end;
	if(!pool) return;
	for(s = pool->slots.data, s_end = s + pool->slots.size; s < s_end; s++)
		assert(s->slab), free(s->slab);
	PP_(slot_array_)(&pool->slots);
	poolfree_heap_(&pool->free0);
	*pool = P_(pool)();
}

/** Ensure capacity of at least `n` further items in `pool`. Pre-sizing is
 better for contiguous blocks, but takes up that memory.
 @return Success. @throws[ERANGE, malloc] @allow */
static int P_(pool_buffer)(struct P_(pool) *const pool, const size_t n) {
	return assert(pool), PP_(buffer)(pool, n);
}

/** This pointer is constant until it gets <fn:<P>pool_remove>.
 @return A pointer to a new uninitialized element from `pool`.
 @throws[ERANGE, malloc] @order amortised O(1) @allow */
static PP_(type) *P_(pool_new)(struct P_(pool) *const pool) {
	struct PP_(slot) *slot0;
	assert(pool);
	if(!PP_(buffer)(pool, 1)) return 0;
	assert(pool->slots.size && (pool->free0.as_array.size ||
		pool->slots.data[0].size < pool->capacity0));
	if(poolfree_heap_size(&pool->free0)) {
		/* Cheating: we prefer the minimum index from a max-heap, but it
		 doesn't really matter, so take the one off the array used for heap. */
		size_t *free;
		free = heap_poolfree_node_array_pop(&pool->free0.as_array);
		return assert(free), pool->slots.data[0].slab + *free;
	}
	/* The free-heap is empty; guaranteed by <fn:<PP>buffer>. */
	slot0 = pool->slots.data + 0;
	assert(slot0 && slot0->size < pool->capacity0);
	return slot0->slab + slot0->size++;
}

/** Deletes `data` from `pool`. (Do not remove data that is not in `pool`.)
 @return Success. @order \O(\log (`slab0-free-heap` | `slabs`))
 @throws[malloc] Because of lazy deletion, remove can actually demand memory
 when `data` requires adding to the free-heap. @allow */
static int P_(pool_remove)(struct P_(pool) *const pool,
	PP_(type) *const data) { return PP_(remove)(pool, data); }

/** Removes all from `pool`, but keeps it's active state, only freeing the
 smaller blocks. @order \O(\log `items`) @allow */
static void P_(pool_clear)(struct P_(pool) *const pool) {
	struct PP_(slot) *s, *s_end;
	assert(pool);
	if(!pool->slots.size) { assert(!pool->free0.as_array.size); return; }
	for(s = pool->slots.data + 1, s_end = s - 1 + pool->slots.size;
		s < s_end; s++) assert(s->slab && s->size), free(s->slab);
	pool->slots.data[0].size = 0;
	pool->slots.size = 1;
	poolfree_heap_clear(&pool->free0);
}

static void PP_(unused_base_coda)(void);
static void PP_(unused_base)(void) {
	PP_(iterator)(0); PP_(element)(0); PP_(next)(0);
	P_(pool)(); P_(pool_)(0); P_(pool_buffer)(0, 0); P_(pool_new)(0);
	P_(pool_remove)(0, 0); P_(pool_clear)(0); PP_(unused_base_coda)();
}
static void PP_(unused_base_coda)(void) { PP_(unused_base)(); }

/* Box override information. */
#define BOX_TYPE struct P_(pool)
#define BOX_CONTENT PP_(type)
#define BOX_ PP_
#define BOX_MAJOR_NAME pool
#define BOX_NAME POOL_NAME


#ifdef POOL_TO_STRING /* <!-- string */
/** Thunk `p` -> `a`. */
static void PP_(to_string)(const PP_(type) *p, char (*const a)[12])
	{ P_(to_string)((const void *)p, a); }
#define TO_STRING_LEFT '['
#define TO_STRING_RIGHT ']'
#include "to_string.h"
#undef POOL_TO_STRING
#endif


#ifdef POOL_TEST /* <!-- test */
#include "../test/test_pool.h"
#undef POOL_TEST
#endif /* test --> */

#endif /* body --> */

#undef BOX_TYPE
#undef BOX_CONTENT
#undef BOX_
#undef BOX_MAJOR_NAME
#undef BOX_NAME
#undef POOL_NAME
#undef POOL_TYPE
#undef POOL_SLAB_MIN_CAPACITY
#ifdef POOL_BODY
#undef POOL_BODY
#endif
#ifdef POOL_HEAD
#undef POOL_HEAD
#endif

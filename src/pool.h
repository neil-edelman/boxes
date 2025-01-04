/** @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Header <../../src/pool.h> depends on <../../src/heap.h> and
 <../../src/array.h>; examples <../../test/test_pool.c>; article
 <../pool/pool.pdf>.

 @subtitle Stable pool

 ![Example of Pool](../doc/pool/pool.png)

 <tag:<t>pool> is a memory pool that stores only one
 size—<typedef:<pT>type>—using
 [slab allocation](https://en.wikipedia.org/wiki/Slab_allocation). As
 <Bonwick, 1994, Slab>, it helps reduce internal fragmentation from repeated
 allocation and destruction by caching contiguous blocks. Pointers to valid
 items in the pool are stable. Instead of freeing memory, a free-heap in the
 active-slab allows lazily reusing the same space. If removal is ongoing and
 uniformly sampled while reaching a steady-state size, it will eventually
 settle in one contiguous region.

 @param[POOL_NAME, POOL_TYPE]
 `<t>` that satisfies `C` naming conventions when mangled and a valid tag type,
 <typedef:<pT>type>, associated therewith; required.

 @param[POOL_DECLARE_ONLY]
 For headers in different compilation units.

 @depend [array](../../src/array.h)
 @depend [heap](../../src/heap.h)
 @depend [box](../../src/box.h)
 @std C89. However, when compiling for segmented memory models, C99 with
 `uintptr_t` is recommended because of it's implementation-defined instead of
 undefined-behaviour when comparing pointers from different objects in the
 heap of memory addresses. Still, this is not guaranteed to produce meaningful
 results on all systems. */

/* `POOL_TO_STRING` is undocumented because this container is only iterable in
 the first slab, so this is not very useful except for debugging. */

#if !defined POOL_NAME || !defined POOL_TYPE
#	error Name or tag type undefined.
#endif
#if defined POOL_TEST && !defined POOL_TO_STRING
#error Test requires to string.
#endif

#if !defined(POOL_H) && !defined(POOL_DECLARE_ONLY)
#	define POOL_H
/** @return An order on `a`, `b` which specifies a max-heap. */
static int poolfree_less(const size_t a, const size_t b) { return a < b; }
#	define HEAP_NAME poolfree
#	define HEAP_TYPE size_t
#	include "heap.h"
#endif
#if !defined(__STDC__) || !defined(__STDC_VERSION__) \
	|| __STDC_VERSION__ < 199901L /* < C99 */
#	define POOL_CAST (const void *)
#else /* < C99 --><!-- >= C99 */
#	include <stdint.h>
#	define POOL_CAST (const uintptr_t)(const void *)
#endif /* >= C99 --> */

#ifdef POOL_NON_STATIC
#	define BOX_NON_STATIC
#endif
#ifdef POOL_DECLARE_ONLY
#	define BOX_DECLARE_ONLY
#endif
#define BOX_START
#include "box.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#define BOX_MINOR POOL_NAME
#define BOX_MAJOR pool

/* Undocumented: set the initial size. */
#ifndef POOL_SLAB_MIN_CAPACITY
#	define POOL_SLAB_MIN_CAPACITY 8
#endif
#if POOL_SLAB_MIN_CAPACITY < 2
#	error Pool slab capacity error.
#endif

/** A valid tag type set by `POOL_TYPE`. */
typedef POOL_TYPE pT_(type);

/* Goes into a slab-sorted array. */
struct pT_(slot) { size_t size; pT_(type) *slab; };

/* Temporary. Avoid recursion. This must match <box.h>. */
#undef BOX_MINOR
#undef BOX_MAJOR
#define pTpool_(n) BOX_CAT(private, BOX_CAT(POOL_NAME, BOX_CAT(pool, n)))
#define ARRAY_NAME pTpool_(slot)
#define ARRAY_TYPE struct pTpool_(slot)
/* This relies on <array.h> which must be in the same directory. */
#include "array.h"
#undef pTpool_
#define BOX_MINOR POOL_NAME
#define BOX_MAJOR pool

/** A zeroed pool is a valid state. See <fn:<t>pool>.

 ![States.](../doc/pool/states.png) */
struct t_(pool) {
	struct pT_(slot_array) slots;
	struct poolfree_heap free0; /* Free-heap in slab-zero. */
	size_t capacity0; /* Capacity of slab-zero. */
};
typedef struct t_(pool) pT_(box);

/* It is very useful in debugging, is required contract, but only iterates on
 `slot0` and ignores the free-heap. This is a memory-manager, we don't have
 enough information to do otherwise. */
struct T_(cursor) { struct pT_(slot) *slot0; size_t i; };

#ifdef BOX_NON_STATIC
struct T_(cursor) T_(begin)(const struct t_(pool) *);
int T_(exists)(const struct T_(cursor) *);
pT_(type) *T_(entry)(struct T_(cursor) *);
void T_(next)(struct T_(cursor) *);
struct t_(pool) t_(pool)(void);
void t_(pool_)(struct t_(pool) *);
int T_(buffer)(struct t_(pool) *, size_t);
pT_(type) *T_(new)(struct t_(pool) *);
int T_(remove)(struct t_(pool) *, pT_(type) *);
void T_(clear)(struct t_(pool) *);
#endif
#ifndef BOX_DECLARE_ONLY

/** @return Index of slot that is higher than `x` in `slots`, but treating zero
 as special. @order \O(\log `slots`) */
static size_t pT_(upper)(const struct pT_(slot_array) *const slots,
	const pT_(type) *const x) {
	const struct pT_(slot) *const base = slots->data;
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
static size_t pT_(slot_idx)(const struct t_(pool) *const pool,
	const pT_(type) *const x) {
	struct pT_(slot) *const base = pool->slots.data;
	size_t up;
	assert(pool && pool->slots.size && base && x);
	/* There's only one option. */
	if(pool->slots.size <= 1
		|| ((const void *)x >= (const void *)base[0].slab
		&& (const void *)x < (const void *)(base[0].slab + pool->capacity0)))
		return assert(pool->slots.size >= 1
		&& (const void *)x >= (const void *)base[0].slab
		&& (const void *)x < (const void *)(base[0].slab + pool->capacity0)), 0;
	up = pT_(upper)(&pool->slots, x);
	return assert(up), up - 1;
}
/** Makes sure there are space for `n` further items in `pool`.
 @return Success. */
static int pT_(buffer)(struct t_(pool) *const pool, const size_t n) {
	const size_t min_size = POOL_SLAB_MIN_CAPACITY,
		max_size = (size_t)-1 / sizeof(pT_(type));
	struct pT_(slot) *base = pool->slots.data, *slot;
	pT_(type) *slab;
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
	if(!pT_(slot_array_buffer)(&pool->slots, 1)) return 0;
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
	else insert = pT_(upper)(&pool->slots, base[0].slab);
	assert(insert <= pool->slots.size);
	slot = pT_(slot_array_insert)(&pool->slots, 1, insert);
	assert(slot); /* Made space for it before. */
	slot->slab = base[0].slab, slot->size = base[0].size;
	base[0].slab = slab, base[0].size = 0;
	return 1;
}
/** Either `data` in `pool` is in a secondary slab, in which case it decrements
 the size, or it's the zero-slab, where it gets added to the free-heap.
 @return Success. It may fail due to a free-heap memory allocation error.
 @order Amortized \O(\log \log `items`) @throws[realloc] */
static int pT_(remove)(struct t_(pool) *const pool,
	const pT_(type) *const data) {
	size_t c = pT_(slot_idx)(pool, data);
	struct pT_(slot) *slot = pool->slots.data + c;
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
		pT_(type) *const slab = slot->slab;
		pT_(slot_array_remove)(&pool->slots, pool->slots.data + c);
		free(slab);
	}
	return 1;
}

#	define BOX_PUBLIC_OVERRIDE
#	include "box.h"

/** @return A cursor at slot0 of `p` or to nothing. */
static struct T_(cursor) T_(begin)(const struct t_(pool) *const p)
	{ struct T_(cursor) cur; cur.slot0 = p && p->slots.data
	? p->slots.data + 0 : 0, cur.i = 0; return cur; }
/** @return Is `cur` valid? */
static int T_(exists)(const struct T_(cursor) *const cur)
	{ return cur && cur->slot0 && cur->slot0->slab
	&& cur->i < cur->slot0->size; }
/** @return A pointer to a valid `cur`. */
static pT_(type) *T_(entry)(struct T_(cursor) *const cur)
	{ return cur->slot0->slab + cur->i; }
/** Next valid `cur`. */
static void T_(next)(struct T_(cursor) *const cur)
	{ if(cur->i == (size_t)~0) cur->slot0 = 0; else cur->i++; }

/** @return An idle pool is zeroed. @order \Theta(1) @allow */
static struct t_(pool) t_(pool)(void) { struct t_(pool) p;
	p.slots = pT_(slot_array)(), p.free0 = poolfree_heap(), p.capacity0 = 0;
	return p; }

/** Destroys `pool` and returns it to idle. @order \O(\log `data`) @allow */
static void t_(pool_)(struct t_(pool) *const pool) {
	struct pT_(slot) *s, *s_end;
	if(!pool) return;
	for(s = pool->slots.data, s_end = s + pool->slots.size; s < s_end; s++)
		assert(s->slab), free(s->slab);
	pT_(slot_array_)(&pool->slots);
	poolfree_heap_(&pool->free0);
	*pool = t_(pool)();
}

/** Ensure capacity of at least `n` further items in `pool`. Pre-sizing is
 better for contiguous blocks, but takes up that memory.
 @return Success. @throws[ERANGE, malloc] @allow */
static int T_(buffer)(struct t_(pool) *const pool, const size_t n) {
	return assert(pool), pT_(buffer)(pool, n);
}

/** This pointer is constant until it gets <fn:<T>remove>.
 @return A pointer to a new uninitialized element from `pool`.
 @throws[ERANGE, malloc] @order amortised O(1) @allow */
static pT_(type) *T_(new)(struct t_(pool) *const pool) {
	struct pT_(slot) *slot0;
	assert(pool);
	if(!pT_(buffer)(pool, 1)) return 0;
	assert(pool->slots.size && (pool->free0.as_array.size ||
		pool->slots.data[0].size < pool->capacity0));
	if(poolfree_heap_size(&pool->free0)) {
		/* Cheating: we prefer the minimum index from a max-heap, but it
		 doesn't really matter, so take the one off the array used for heap. */
		size_t *free;
		free = private_poolfree_heap_priority_array_pop(&pool->free0.as_array);
		return assert(free), pool->slots.data[0].slab + *free;
	}
	/* The free-heap is empty; guaranteed by <fn:<pT>buffer>. */
	slot0 = pool->slots.data + 0;
	assert(slot0 && slot0->size < pool->capacity0);
	return slot0->slab + slot0->size++;
}

/** Deletes `data` from `pool`. (Do not remove data that is not in `pool`.)
 @return Success. @order \O(\log (`slab0-free-heap` | `slabs`))
 @throws[malloc] Because of lazy deletion, remove can actually demand memory
 when `data` requires adding to the free-heap. @allow */
static int T_(remove)(struct t_(pool) *const pool,
	pT_(type) *const data) { return pT_(remove)(pool, data); }

/** Removes all from `pool`, but keeps it's active state, only freeing the
 smaller blocks. @order \O(\log `items`) @allow */
static void T_(clear)(struct t_(pool) *const pool) {
	struct pT_(slot) *s, *s_end;
	assert(pool);
	if(!pool->slots.size) { assert(!pool->free0.as_array.size); return; }
	for(s = pool->slots.data + 1, s_end = s - 1 + pool->slots.size;
		s < s_end; s++) assert(s->slab && s->size), free(s->slab);
	pool->slots.data[0].size = 0;
	pool->slots.size = 1;
	poolfree_heap_clear(&pool->free0);
}

#	define BOX_PRIVATE_AGAIN
#	include "box.h"

static void pT_(unused_base_coda)(void);
static void pT_(unused_base)(void) {
	T_(begin)(0); T_(exists)(0); T_(entry)(0); T_(next)(0);
	t_(pool)(); t_(pool_)(0); T_(buffer)(0, 0); T_(new)(0);
	T_(remove)(0, 0); T_(clear)(0); pT_(unused_base_coda)();
}
static void pT_(unused_base_coda)(void) { pT_(unused_base)(); }
#endif /* Produce code. */

#ifdef POOL_TO_STRING
#	undef POOL_TO_STRING
#	ifndef POOL_DECLARE_ONLY
/** Type of the required `<tr>to_string`. Responsible for turning the
 read-only argument into a 12-max-`char` output string. */
typedef void (*pT_(to_string_fn))(const pT_(type) *, char (*)[12]);
/** Thunk(`cur`, `a`). One must implement `<tr>to_string`. */
static void pTR_(to_string)(const struct T_(cursor) *const cur,
	char (*const a)[12])
	{ tr_(to_string)(&cur->slot0->slab[cur->i], a); }
#	endif
#	define TO_STRING_LEFT '['
#	define TO_STRING_RIGHT ']'
#	include "to_string.h"
#	define POOL_HAS_TO_STRING /* Warning about tests. */
#endif

#if defined HAS_GRAPH_H && defined POOL_HAS_TO_STRING
#	include "graph.h" /** \include */
#endif

#if defined POOL_TEST && defined POOL_HAS_TO_STRING && defined HAS_GRAPH_H
#	undef POOL_TEST
#	include "../test/test_pool.h"
#endif


#undef BOX_MINOR
#undef BOX_MAJOR
#undef POOL_NAME
#undef POOL_TYPE
#undef POOL_SLAB_MIN_CAPACITY
#ifdef POOL_HAS_TO_STRING
#	undef POOL_HAS_TO_STRING
#endif
#ifdef POOL_DECLARE_ONLY
#	undef POOL_DECLARE_ONLY
#endif
#undef POOL_CAST
#define BOX_END
#include "box.h"

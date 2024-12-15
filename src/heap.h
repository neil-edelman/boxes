/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Header <../../src/heap.h> depends on <../../src/array.h>; examples
 <../../test/test_heap.c>.

 @subtitle Priority-queue

 ![Example of heap.](../doc/heap/heap.png)

 A <tag:<t>heap> is a binary heap, proposed by
 <Williams, 1964, Heapsort, p. 347> using terminology of
 <Knuth, 1973, Sorting>. It is an array implementation of a priority queue. It
 is not stable.

 A function satisfying <typedef:<pT>less_fn> called `<t>less` must be declared.
 For example, a maximum heap, `(a, b) -> a < b`.

 @param[HEAP_NAME, HEAP_TYPE]
 `<t>` that satisfies `C` naming conventions when mangled. `HEAP_NAME` is
 required; `HEAP_TYPE` defaults to `unsigned int`.

 @param[HEAP_TO_STRING]
 To string trait contained in <../../src/to_string.h>. See
 <typedef:<pT>to_string_fn>.

 @param[HEAP_EXPECT_TRAIT, HEAP_TRAIT]
 Named traits are obtained by including `heap.h` multiple times with
 `HEAP_EXPECT_TRAIT` and then subsequently including the name in `HEAP_TRAIT`.

 @param[HEAP_DECLARE_ONLY]
 For headers in different compilation units.

 @depend [array](../../src/array.h)
 @depend [box](../../src/box.h)
 @std C89 */

#ifndef HEAP_NAME
#	error Name undefined.
#endif
#if !defined(BOX_ENTRY1) && (defined(HEAP_TRAIT) ^ defined(BOX_MAJOR))
#	error HEAP_TRAIT name must come after HEAP_EXPECT_TRAIT.
#endif
#if defined(HEAP_TEST) && (!defined(HEAP_TRAIT) && !defined(HEAP_TO_STRING) \
	|| defined(HEAP_TRAIT) && !defined(HEAP_HAS_TO_STRING))
#error Test requires to string.
#endif
#if defined(BOX_TRAIT) && !defined(HEAP_TRAIT)
#	error Unexpected.
#endif

#ifdef HEAP_TRAIT
#	define BOX_TRAIT HEAP_TRAIT /* Ifdef in <box.h>. */
#endif
#define BOX_START
#include "box.h"

#ifndef HEAP_TRAIT /* Base code, necessarily first. */

#	define BOX_MINOR HEAP_NAME
#	define BOX_MAJOR heap

#	ifndef HEAP_TYPE
#		define HEAP_TYPE unsigned
#	endif

/** Valid assignable type used for priority. Defaults to `unsigned int` if not
 set by `HEAP_TYPE`. */
typedef HEAP_TYPE pT_(priority);

/* Temporary. Avoid recursion. This must match <box.h>. */
#	undef BOX_MINOR
#	undef BOX_MAJOR
#	define pTheap_(n) BOX_CAT(private, BOX_CAT(HEAP_NAME, BOX_CAT(heap, n)))
#	define ARRAY_NAME pTheap_(priority)
#	define ARRAY_TYPE pTheap_(priority)
/* This relies on <array.h> which must be in the same directory. */
#	include "array.h"
#	undef pTheap_
#	define BOX_MINOR HEAP_NAME
#	define BOX_MAJOR heap

/** Stores the heap as an array—implicit binary tree in an array called `a`.
 See <fn:<t>heap>.

 ![States.](../doc/heap/states.png) */
struct t_(heap) { struct pT_(priority_array) as_array; };
typedef struct t_(heap) pT_(box);
struct T_(cursor) { struct pT_(priority_array_cursor) _; };

#	ifndef HEAP_DECLARE_ONLY /* <!-- body */

/** Inducing a strict weak order by returning a positive result if `a` is
 out-of-order with respect to `b`. It only needs to divide entries into
 two instead of three categories. Compatible, but less strict then the
 comparators from `bsearch` and `qsort`. For example, `return a > b` or
 `return strcmp(a, b)` would give a minimum-hash. */
typedef int (*pT_(less_fn))(const pT_(priority) a, const pT_(priority) b);

/** @return An iterator at the beginning of `h`. */
static struct T_(cursor) T_(begin)(struct t_(heap) *const h)
	{ struct T_(cursor) it; it._ = pT_(priority_array_begin)(&h->as_array);
	return it; }
/** @return Non-zero if `cur` points to a valid entry. */
static int T_(exists)(struct T_(cursor) *const cur)
	{ return pT_(priority_array_exists)(&cur->_); }
/** @return The <typedef:<pT>priority> pointer that at which a valid `cur`
 points. */
static pT_(priority) *T_(look)(struct T_(cursor) *const cur)
	{ return pT_(priority_array_look)(&cur->_); }
/** Move to the next of a valid `cur`. */
static void T_(next)(struct T_(cursor) *const cur)
	{ pT_(priority_array_next)(&cur->_); }

/** Find the spot in `heap` where `n` goes and put it there.
 @param[heap] At least one entry; the last entry will be replaced by `node`.
 @order \O(log `size`) */
static void pT_(sift_up)(struct t_(heap) *const heap, pT_(priority) n) {
	pT_(priority) *const n0 = heap->as_array.data;
	size_t i = heap->as_array.size - 1, i_up;
	if(i) {
		do { /* Note: don't change the `<=`; it's a queue. */
			i_up = (i - 1) >> 1;
			/* Make sure that `<HEAP_NAME>_less` is defined. */
			if(t_(less)(n0[i_up], n) <= 0) break;
			n0[i] = n0[i_up];
		} while((i = i_up));
	}
	n0[i] = n;
}
/** Pop the head of `heap` and restore the heap by sifting down the last
 element. @param[heap] At least one entry. The head is popped, and the size
 will be one less. */
static void pT_(sift_down)(struct t_(heap) *const heap) {
	const size_t size = (assert(heap && heap->as_array.size),
		--heap->as_array.size), half = size >> 1;
	size_t i = 0, c;
	pT_(priority) *const n0 = heap->as_array.data,
		*const down = n0 + size /* Put it at the top. */, *child;
	while(i < half) {
		c = (i << 1) + 1;
		if(c + 1 < size && t_(less)(n0[c], n0[c + 1]) > 0) c++;
		child = n0 + c;
		if(t_(less)(*down, *child) <= 0) break;
		n0[i] = *child;
		i = c;
	}
	n0[i] = *down;
}
/** Restore the `heap` by permuting the elements so `i` is in the proper place.
 This reads from the an arbitrary leaf-node into a temporary value, so is
 slightly more complex than <fn:<pT>sift_down>, but the same thing.
 @param[heap] At least `i + 1` entries. */
static void pT_(sift_down_i)(struct t_(heap) *const heap, size_t i) {
	const size_t size = (assert(heap && i < heap->as_array.size),
		heap->as_array.size), half = size >> 1;
	size_t c;
	/* Uninitialized variable warning suppression. */
	pT_(priority) *const n0 = heap->as_array.data, *child, temp = *(&temp);
	int temp_valid = 0;
	while(i < half) {
		c = (i << 1) + 1;
		if(c + 1 < size && t_(less)(n0[c], n0[c + 1]) > 0) c++;
		child = n0 + c;
		if(temp_valid) {
			if(t_(less)(temp, *child) <= 0) break;
		} else {
			/* Only happens on the first compare when `i` is in it's original
			 position. */
			if(t_(less)(n0[i], *child) <= 0) break;
			temp = n0[i], temp_valid = 1;
		}
		n0[i] = *child;
		i = c;
	}
	if(temp_valid) n0[i] = temp;
}
/** Create a `heap` from an array. @order \O(`heap.size`) */
static void pT_(heapify)(struct t_(heap) *const heap) {
	size_t i;
	if(heap->as_array.size > 1) for(i = heap->as_array.size / 2 - 1;
		(pT_(sift_down_i)(heap, i), i); i--);
}
/** Removes from `heap`. Must have a non-zero size. */
static pT_(priority) pT_(remove)(struct t_(heap) *const heap) {
	const pT_(priority) result = *heap->as_array.data;
	assert(heap);
	if(heap->as_array.size > 1) {
		pT_(sift_down)(heap);
	} else {
		assert(heap->as_array.size == 1);
		heap->as_array.size = 0;
	}
	return result;
}

/** Zeroed data (not all-bits-zero) is initialised.
 @return An idle heap. @order \Theta(1) @allow */
static struct t_(heap) t_(heap)(void)
	{ struct t_(heap) heap; heap.as_array = pT_(priority_array)(); return heap; }

/** Returns `heap` to the idle state where it takes no dynamic memory.
 @order \Theta(1) @allow */
static void t_(heap_)(struct t_(heap) *const heap)
	{ if(heap) pT_(priority_array_)(&heap->as_array); }

/** Sets `heap` to be empty. That is, the size of `heap` will be zero, but if
 it was previously in an active non-idle state, it continues to be.
 @param[heap] If null, does nothing. @order \Theta(1) @allow */
static void T_(clear)(struct t_(heap) *const heap)
	{ assert(heap), pT_(priority_array_clear)(&heap->as_array); }

/** @return If the `heap` is not null, returns it's size. @allow */
static size_t T_(size)(const struct t_(heap) *const heap)
	{ return heap ? heap->as_array.size : 0; }

/** Copies `node` into `heap`.
 @return Success. @throws[ERANGE, realloc] @order \O(log `heap.size`) @allow */
static int T_(add)(struct t_(heap) *const heap, pT_(priority) node)
	{ return assert(heap), pT_(priority_array_new)(&heap->as_array)
		&& (pT_(sift_up)(heap, node), 1); }

/** @return The value of the lowest element in `heap` or null when the heap is
 empty. @order \O(1) @allow */
static pT_(priority) *T_(peek)(const struct t_(heap) *const heap)
	{ return assert(heap), heap->as_array.size ? heap->as_array.data : 0; }

/** Only defined when <fn:<T>size> returns true. Removes the lowest element.
 @return The value of the lowest element in `heap`.
 @order \O(\log `size`) @allow */
static pT_(priority) T_(pop)(struct t_(heap) *const heap) {
	pT_(priority) n;
	return assert(heap && heap->as_array.size),
		(n = pT_(remove)(heap), n);
}

/** The capacity of `heap` will be increased to at least `n` elements beyond
 the size. Invalidates pointers in `heap`. All the elements
 `heap.as_array.size` <= `index` < `heap.as_array.capacity` can be used to
 construct new elements without immediately making them part of the heap, then
 <fn:<T>append>.
 @return The start of the buffered space. If `a` is idle and `buffer` is zero,
 a null pointer is returned, otherwise null indicates an error.
 @throws[realloc, ERANGE] @allow */
static pT_(priority) *T_(buffer)(struct t_(heap) *const heap,
	const size_t n) { return pT_(priority_array_buffer)(&heap->as_array, n); }

/** Adds and heapifies `n` elements to `heap`. Uses <Floyd, 1964, Treesort> to
 sift-down all the internal nodes of heap. The heap elements must exist, see
 <fn:<T>buffer>.
 @param[n] If zero, returns true without heapifying.
 @return Success. @order \O(`heap.size` + `n`) <Doberkat, 1984, Floyd> @allow */
static void T_(append)(struct t_(heap) *const heap, const size_t n) {
	pT_(priority) *more;
	/* In practice, pushing uninitialized elements onto the heap does not make
	 sense, so we assert that the elements exist first. */
	assert(heap && n <= heap->as_array.capacity - heap->as_array.size);
	more = pT_(priority_array_append)(&heap->as_array, n), assert(more);
	if(n) pT_(heapify)(heap);
}

/** Shallow-copies and heapifies `master` into `heap`.
 @param[master] If null, does nothing. @return Success.
 @order \O(`heap.size` + `copy.size`) @throws[ERANGE, realloc] @allow */
static int T_(affix)(struct t_(heap) *restrict const heap,
	const struct t_(heap) *restrict const master) {
	pT_(priority) *n;
	assert(heap);
	if(!master || !master->as_array.size) return 1;
	assert(master->as_array.data);
	if(!(n = pT_(priority_array_buffer)(&heap->as_array, master->as_array.size)))
		return 0;
	memcpy(n, master->as_array.data, sizeof *n * master->as_array.size);
	n = pT_(priority_array_append)(&heap->as_array, master->as_array.size),
		assert(n);
	pT_(heapify)(heap);
	return 1;
}

static void pT_(unused_base_coda)(void);
static void pT_(unused_base)(void) {
	pT_(priority) unused; memset(&unused, 0, sizeof unused);
	T_(begin)(0); T_(exists)(0); T_(look)(0); T_(next)(0);
	t_(heap)(); t_(heap_)(0); T_(clear)(0); T_(size)(0);
	T_(add)(0, unused); T_(peek)(0); T_(pop)(0);
	T_(buffer)(0, 0); T_(append)(0, 0); T_(affix)(0, 0);
	pT_(unused_base_coda)();
}
static void pT_(unused_base_coda)(void) { pT_(unused_base)(); }

#	endif /* Produce code. */
#endif /* Base code. */

#ifndef HEAP_DECARE_ONLY /* Produce code. */

#	if defined(HEAP_TO_STRING)
#		undef HEAP_TO_STRING
#		ifndef HEAP_TRAIT
/** The type of the required `<tr>to_string`. Responsible for turning the
 read-only argument into a 12-max-`char` output string. */
typedef void (*pT_(to_string_fn))(const pT_(priority) *, char (*)[12]);
#		endif
/** Thunk(`cur`, `a`). One must implement `<tr>to_string`. */
static void pTR_(to_string)(const struct T_(cursor) *const cur,
	char (*const a)[12])
	{ tr_(to_string)((const void *)&cur->_.a->data[cur->_.i], a); }
#		define TO_STRING_LEFT '['
#		define TO_STRING_RIGHT ']'
#		include "to_string.h" /** \include */
#		ifndef HEAP_TRAIT
#			define HEAP_HAS_TO_STRING /* Warning about tests. */
#		endif
#	endif

#	if defined(HEAP_TEST) && !defined(HEAP_TRAIT)
#		include "../test/test_heap.h"
#	endif

#endif /* Produce code. */
#ifdef HEAP_TRAIT
#	undef HEAP_TRAIT
#	undef BOX_TRAIT
#endif


#ifdef HEAP_EXPECT_TRAIT
#	undef HEAP_EXPECT_TRAIT
#else
#	undef BOX_MINOR
#	undef BOX_MAJOR
#	undef HEAP_NAME
#	undef HEAP_TYPE
#	ifdef HEAP_VALUE
#		undef HEAP_VALUE
#	endif
#	ifdef HEAP_HAS_TO_STRING
#		undef HEAP_HAS_TO_STRING
#	endif
#	ifdef HEAP_TEST
#		undef HEAP_TEST
#	endif
#	ifdef HEAP_DECLARE_ONLY
#		undef HEAP_DECLARE_ONLY
#	endif
#endif
#define BOX_END
#include "box.h"

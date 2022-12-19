/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Header <src/heap.h> depends on <src/array.h>; examples
 <test/test_heap.c>; on a compatible workstation, `make` creates the test suite
 of the examples.

 @subtitle Priority-queue

 ![Example of heap.](../doc/heap.png)

 A <tag:<H>heap> is a binary heap, proposed by
 <Williams, 1964, Heapsort, p. 347> using terminology of
 <Knuth, 1973, Sorting>. It can be used as an implementation of a priority
 queue. Internally, it is an array with implicit heap properties on
 <typedef:<PH>priority> and an optional <typedef:<PH>value> that is associated
 with the value.

 @param[HEAP_NAME, HEAP_TYPE]
 `<H>` that satisfies `C` naming conventions when mangled and an assignable
 type <typedef:<PH>priority> associated therewith. `HEAP_NAME` is required;
 `HEAP_TYPE` defaults to `unsigned int`. `<PH>` is private, whose names are
 prefixed in a manner to avoid collisions.

 @param[HEAP_COMPARE]
 A function satisfying <typedef:<PH>compare_fn>. Defaults to minimum-hash.
 Required if `HEAP_TYPE` is changed to an incomparable type. For example, a
 maximum heap, `(a, b) -> a < b`.

 @param[HEAP_VALUE]
 Optional value <typedef:<PH>value>, that, on `HEAP_VALUE`, is stored in
 <tag:<H>heapnode>, which is <typedef:<PH>value>.

 @param[HEAP_TO_STRING]
 To string trait `<STR>` contained in <src/to_string.h>. Require
 `<name>[<trait>]to_string` be declared as <typedef:<PSTR>to_string_fn>.

 @param[HEAP_EXPECT_TRAIT, HEAP_TRAIT]
 Named traits are obtained by including `heap.h` multiple times with
 `HEAP_EXPECT_TRAIT` and then subsequently including the name in
 `HEAP_TRAIT`.

 @depend [array](https://github.com/neil-edelman/array)
 @std C89 */

#ifndef HEAP_NAME
#error Name HEAP_NAME undefined.
#endif
#if defined(HEAP_TRAIT) ^ defined(BOX_TYPE)
#error HEAP_TRAIT name must come after HEAP_EXPECT_TRAIT.
#endif
#if defined(HEAP_TEST) && (!defined(HEAP_TRAIT) && !defined(HEAP_TO_STRING) \
	|| defined(HEAP_TRAIT) && !defined(HEAP_HAS_TO_STRING))
#error Test requires to string.
#endif

#ifndef HEAP_H /* <!-- idempotent */
#define HEAP_H
#if defined(HEAP_CAT_) || defined(HEAP_CAT) || defined(H_) || defined(PH_)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define HEAP_CAT_(n, m) n ## _ ## m
#define HEAP_CAT(n, m) HEAP_CAT_(n, m)
#define H_(n) HEAP_CAT(HEAP_NAME, n)
#define PH_(n) HEAP_CAT(heap, H_(n))
#endif /* idempotent --> */

#if !defined(restrict) && (!defined(__STDC__) || !defined(__STDC_VERSION__) \
	|| __STDC_VERSION__ < 199901L)
#define HEAP_RESTRICT /* Undo this at the end. */
#define restrict /* Attribute only in C99+. */
#endif


#ifndef HEAP_TRAIT /* <!-- base code */

#ifndef HEAP_TYPE
#define HEAP_TYPE unsigned
#endif

/** Valid assignable type used for priority in <typedef:<PH>node>. Defaults to
 `unsigned int` if not set by `HEAP_TYPE`. */
typedef HEAP_TYPE PH_(priority);
typedef const HEAP_TYPE PH_(priority_c); /* This is assuming a lot. */

/** Returns a positive result if `a` is out-of-order with respect to `b`,
 inducing a strict weak order. This is compatible, but less strict then the
 comparators from `bsearch` and `qsort`; it only needs to divide entries into
 two instead of three categories. */
typedef int (*PH_(compare_fn))(PH_(priority_c) a, PH_(priority_c) b);
#ifndef HEAP_COMPARE /* <!-- !cmp */
/** The default `HEAP_COMPARE` on `a` and `b` is `a > b`, which makes a
 minimum-hash. @implements <typedef:<PH>compare_fn> */
static int PH_(default_compare)(PH_(priority_c) a, PH_(priority_c) b)
	{ return a > b; }
#define HEAP_COMPARE &PH_(default_compare)
#endif /* !cmp --> */
/* Check that `HEAP_COMPARE` is a function implementing
 <typedef:<PH>compare_fn>, if defined. */
static const PH_(compare_fn) PH_(compare) = (HEAP_COMPARE);

#ifdef HEAP_VALUE /* <!-- value */
typedef HEAP_VALUE PH_(value);
typedef const HEAP_VALUE PH_(value_c); /* Assume! */
/** If `HEAP_VALUE` is set, this becomes <typedef:<PH>node>. */
struct H_(heapnode) { PH_(priority) priority; PH_(value) value; };
/** If `HEAP_VALUE` is set, (priority, value) set by <tag:<H>heapnode>,
 otherwise it's a (priority) set directly by <typedef:<PH>priority>. */
typedef struct H_(heapnode) PH_(node);
typedef const struct H_(heapnode) PH_(node_c);
#else /* value --><!-- !value */
typedef PH_(priority) PH_(value);
typedef PH_(priority) PH_(node);
typedef PH_(priority_c) PH_(node_c);
#endif /* !value --> */

/* This relies on <src/array.h> which must be in the same directory. */
#define ARRAY_NAME PH_(node)
#define ARRAY_TYPE PH_(node)
#include "array.h"

/** Stores the heap as an implicit binary tree in an array called `a`. To
 initialize it to an idle state, see <fn:<H>heap>, `{0}` (`C99`), or being
 `static`.

 ![States.](../doc/states.png) */
struct H_(heap) { struct PH_(node_array) as_array; };

#define PAH_(n) HEAP_CAT(HEAP_CAT(array, PH_(node)), n)
struct PH_(iterator) { struct PAH_(iterator) _; };
/** @return Before `h`. @implements `forward` */
static struct PH_(iterator) PH_(begin)(struct H_(heap) *const h) {
	struct PH_(iterator) it; it._ = PAH_(begin)(&h->as_array); return it; }
/** @return The next `it` or null. @implements `next_c` */
static int PH_(next)(struct PH_(iterator) *const it, PH_(node) **const v)
	{ return PAH_(next)(&it->_, v); }
#undef PAH_

/** Extracts the <typedef:<PH>priority> of `node`, which must not be null. */
static PH_(priority) PH_(get_priority)(const PH_(node) *const node) {
#ifdef HEAP_VALUE
	return node->priority;
#else
	return *node;
#endif
}

/** Extracts the <typedef:<PH>value> of `node`, which must not be null. */
static PH_(value) PH_(get_value)(const PH_(node) *const node) {
#ifdef HEAP_VALUE
	return node->value;
#else
	return *node;
#endif
}

/** Find the spot in `heap` where `node` goes and put it there.
 @param[heap] At least one entry; the last entry will be replaced by `node`.
 @order \O(log `size`) */
static void PH_(sift_up)(struct H_(heap) *const heap, PH_(node) *const node) {
	PH_(node) *const n0 = heap->as_array.data;
	PH_(priority) p = PH_(get_priority)(node);
	size_t i = heap->as_array.size - 1;
	assert(heap && heap->as_array.size && node);
	if(i) {
		size_t i_up;
		do { /* Note: don't change the `<=`; it's a queue. */
			i_up = (i - 1) >> 1;
			if(PH_(compare)(PH_(get_priority)(n0 + i_up), p) <= 0) break;
			n0[i] = n0[i_up];
		} while((i = i_up));
	}
	n0[i] = *node;
}

/** Pop the head of `heap` and restore the heap by sifting down the last
 element. @param[heap] At least one entry. The head is popped, and the size
 will be one less. */
static void PH_(sift_down)(struct H_(heap) *const heap) {
	const size_t size = (assert(heap && heap->as_array.size), --heap->as_array.size),
		half = size >> 1;
	size_t i = 0, c;
	PH_(node) *const n0 = heap->as_array.data,
		*const down = n0 + size /* Put it at the top. */, *child;
	const PH_(priority) down_p = PH_(get_priority)(down);
	while(i < half) {
		c = (i << 1) + 1;
		if(c + 1 < size && PH_(compare)(PH_(get_priority)(n0 + c),
			PH_(get_priority)(n0 + c + 1)) > 0) c++;
		child = n0 + c;
		if(PH_(compare)(down_p, PH_(get_priority)(child)) <= 0) break;
		n0[i] = *child;
		i = c;
	}
	n0[i] = *down;
}

/** Restore the `heap` by permuting the elements so `i` is in the proper place.
 This reads from the an arbitrary leaf-node into a temporary value, so is
 slightly more complex than <fn:<PH>sift_down>, but the same thing.
 @param[heap] At least `i + 1` entries. */
static void PH_(sift_down_i)(struct H_(heap) *const heap, size_t i) {
	const size_t size = (assert(heap && i < heap->as_array.size), heap->as_array.size),
		half = size >> 1;
	size_t c;
	/* Uninitialized variable warning suppression. */
	PH_(node) *const n0 = heap->as_array.data, *child, temp = *(&temp);
	int temp_valid = 0;
	while(i < half) {
		c = (i << 1) + 1;
		if(c + 1 < size && PH_(compare)(PH_(get_priority)(n0 + c),
			PH_(get_priority)(n0 + c + 1)) > 0) c++;
		child = n0 + c;
		if(temp_valid) {
			if(PH_(compare)(PH_(get_priority)(&temp),
				PH_(get_priority)(child)) <= 0) break;
		} else {
			/* Only happens on the first compare when `i` is in it's original
			 position. */
			if(PH_(compare)(PH_(get_priority)(n0 + i),
				PH_(get_priority)(child)) <= 0) break;
			temp = n0[i], temp_valid = 1;
		}
		n0[i] = *child;
		i = c;
	}
	if(temp_valid) n0[i] = temp;
}

/** Create a `heap` from an array. @order \O(`heap.size`) */
static void PH_(heapify)(struct H_(heap) *const heap) {
	size_t i;
	assert(heap);
	if(heap->as_array.size > 1)
		for(i = heap->as_array.size / 2 - 1; (PH_(sift_down_i)(heap, i), i); i--);
}

/** Removes from `heap`. Must have a non-zero size. */
static PH_(node) PH_(remove)(struct H_(heap) *const heap) {
	const PH_(node) result = *heap->as_array.data;
	assert(heap);
	if(heap->as_array.size > 1) {
		PH_(sift_down)(heap);
	} else {
		assert(heap->as_array.size == 1);
		heap->as_array.size = 0;
	}
	return result;
}

/** Zeroed data (not all-bits-zero) is initialised.
 @return An idle heap. @order \Theta(1) @allow */
static struct H_(heap) H_(heap)(void)
	{ struct H_(heap) heap; heap.as_array = PH_(node_array)(); return heap; }

/** Returns `heap` to the idle state where it takes no dynamic memory.
 @order \Theta(1) @allow */
static void H_(heap_)(struct H_(heap) *const heap)
	{ if(heap) PH_(node_array_)(&heap->as_array); }

/** Sets `heap` to be empty. That is, the size of `heap` will be zero, but if
 it was previously in an active non-idle state, it continues to be.
 @param[heap] If null, does nothing. @order \Theta(1) @allow */
static void H_(heap_clear)(struct H_(heap) *const heap)
	{ assert(heap), PH_(node_array_clear)(&heap->as_array); }

/** @return If the `heap` is not null, returns it's size. @allow */
static size_t H_(heap_size)(const struct H_(heap) *const heap)
	{ return heap ? heap->as_array.size : 0; }

/** Copies `node` into `heap`.
 @return Success. @throws[ERANGE, realloc] @order \O(log `heap.size`) @allow */
static int H_(heap_add)(struct H_(heap) *const heap, PH_(node) node) {
	assert(heap);
	return PH_(node_array_new)(&heap->as_array) && (PH_(sift_up)(heap, &node), 1);
}

/** @return The value of the lowest element in `heap` or null when the heap is
 empty. @order \O(1) @allow */
static PH_(node) *H_(heap_peek)(const struct H_(heap) *const heap)
	{ return assert(heap), heap->as_array.size ? heap->as_array.data : 0; }

/** Only defined when <fn:<H>heap_size> returns true. Removes the lowest
 element. @return The value of the lowest element in `heap`.
 @order \O(\log `size`) @allow */
static PH_(value) H_(heap_pop)(struct H_(heap) *const heap) {
	PH_(node) n;
	return assert(heap && heap->as_array.size), (n = PH_(remove)(heap), PH_(get_value)(&n));
}

/** The capacity of `heap` will be increased to at least `n` elements beyond
 the size. Invalidates pointers in `heap`. All the elements
 `heap.as_array.size` <= `index` < `heap.as_array.capacity` can be used to construct new
 elements without immediately making them part of the heap, then
 <fn:<H>heap_append>.
 @return The start of the buffered space. If `a` is idle and `buffer` is zero,
 a null pointer is returned, otherwise null indicates an error.
 @throws[realloc, ERANGE] @allow */
static PH_(node) *H_(heap_buffer)(struct H_(heap) *const heap,
	const size_t n) { return PH_(node_array_buffer)(&heap->as_array, n); }

/** Adds and heapifies `n` elements to `heap`. Uses <Floyd, 1964, Treesort> to
 sift-down all the internal nodes of heap. The heap elements must exist, see
 <fn:<H>heap_buffer>.
 @param[n] If zero, returns true without heapifying.
 @return Success. @order \O(`heap.size` + `n`) <Doberkat, 1984, Floyd> @allow */
static void H_(heap_append)(struct H_(heap) *const heap, const size_t n) {
	PH_(node) *more;
	/* In practice, pushing uninitialized elements onto the heap does not make
	 sense, so we assert that the elements exist first. */
	assert(heap && n <= heap->as_array.capacity - heap->as_array.size);
	more = PH_(node_array_append)(&heap->as_array, n), assert(more);
	if(n) PH_(heapify)(heap);
}

/** Shallow-copies and heapifies `master` into `heap`.
 @param[master] If null, does nothing. @return Success.
 @order \O(`heap.size` + `copy.size`) @throws[ERANGE, realloc] @allow */
static int H_(heap_affix)(struct H_(heap) *restrict const heap,
	const struct H_(heap) *restrict const master) {
	PH_(node) *n;
	assert(heap);
	if(!master || !master->as_array.size) return 1;
	assert(master->as_array.data);
	if(!(n = PH_(node_array_buffer)(&heap->as_array, master->as_array.size)))
		return 0;
	memcpy(n, master->as_array.data, sizeof *n * master->as_array.size);
	n = PH_(node_array_append)(&heap->as_array, master->as_array.size),
		assert(n);
	PH_(heapify)(heap);
	return 1;
}

static void PH_(unused_base_coda)(void);
static void PH_(unused_base)(void) {
	PH_(node) unused; memset(&unused, 0, sizeof unused);
	PH_(begin)(0); PH_(next)(0, 0);
	H_(heap)(); H_(heap_)(0); H_(heap_clear)(0); H_(heap_size)(0);
	H_(heap_add)(0, unused); H_(heap_peek)(0); H_(heap_pop)(0);
	H_(heap_buffer)(0, 0); H_(heap_append)(0, 0); H_(heap_affix)(0, 0);
	PH_(unused_base_coda)();
}
static void PH_(unused_base_coda)(void) { PH_(unused_base)(); }

/* Box override information. */
#define BOX_TYPE struct H_(heap)
#define BOX_VALUE PH_(node)
#define BOX_ PH_
#define BOX_MAJOR_NAME heap
#define BOX_MINOR_NAME HEAP_NAME

#endif /* base code --> */


#ifdef HEAP_TRAIT /* <-- trait: Will be different on different includes. */
#define BOX_TRAIT_NAME HEAP_TRAIT
#define PHT_(n) PH_(ARRAY_CAT(HEAP_TRAIT, n))
#define HT_(n) H_(ARRAY_CAT(HEAP_TRAIT, n))
#else /* trait --><!-- !trait */
#define PHT_(n) PH_(n)
#define HT_(n) H_(n)
#endif /* !trait --> */


#ifdef HEAP_TO_STRING /* <!-- to string trait */
/** Thunk `n` -> `a`. */
static void PHT_(to_string)(const PH_(node) *n, char (*const a)[12])
	{ HT_(to_string)((const void *)n, a); }
#include "to_string.h" /** \include */
#undef HEAP_TO_STRING
#ifndef HEAP_TRAIT
#define HEAP_HAS_TO_STRING
#endif
#endif /* to string trait --> */
#undef PHT_
#undef HT_


#if defined(HEAP_TEST) && !defined(HEAP_TRAIT) /* <!-- test base */
#include "../test/test_heap.h"
#endif /* test base --> */


#ifdef HEAP_EXPECT_TRAIT /* <!-- more */
#undef HEAP_EXPECT_TRAIT
#else /* more --><!-- done */
#undef BOX_TYPE
#undef BOX_VALUE
#undef BOX_
#undef BOX_MAJOR_NAME
#undef BOX_MINOR_NAME
#undef HEAP_NAME
#undef HEAP_TYPE
#undef HEAP_COMPARE
#ifdef HEAP_VALUE
#undef HEAP_VALUE
#endif
#ifdef HEAP_HAS_TO_STRING
#undef HEAP_HAS_TO_STRING
#endif
#ifdef HEAP_TEST
#undef HEAP_TEST
#endif
#endif /* done --> */
#ifdef HEAP_TRAIT
#undef HEAP_TRAIT
#undef BOX_TRAIT_NAME
#endif
#ifdef HEAP_RESTRICT
#undef HEAP_RESTRICT
#undef restrict
#endif

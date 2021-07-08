/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Priority Queue

 ![Example of heap.](../web/heap.png)

 A <tag:<H>heap> is a priority queue built from <tag:<H>heap_node>. It is a
 binary heap, proposed by <Williams, 1964, Heapsort, p. 347> and using
 terminology of <Knuth, 1973, Sorting>. Internally, it is an
 `<<H>heap_node>array` with implicit heap properties, with an optionally cached
 <typedef:<PH>priority> and an optional <typedef:<PH>value> pointer payload. As
 such, one needs to have `array.h` file in the same directory.

 @param[HEAP_NAME, HEAP_TYPE]
 `<H>` that satisfies `C` naming conventions when mangled and an assignable
 type <typedef:<PH>priority> associated therewith. `HEAP_NAME` is required but
 `HEAP_TYPE` defaults to `unsigned int` if not specified. `<PH>` is private,
 whose names are prefixed in a manner to avoid collisions.

 @param[HEAP_COMPARE]
 A function satisfying <typedef:<PH>compare_fn>. Defaults to minimum-hash on
 `HEAP_TYPE`; as such, required if `HEAP_TYPE` is changed to an incomparable
 type.

 @param[HEAP_VALUE]
 Optional payload <typedef:<PH>adjunct>, that is stored as a reference in
 <tag:<H>heap_node> as <typedef:<PH>value>; declaring it is sufficient.

 @param[HEAP_TEST]
 To string trait contained in <../test/heap_test.h>; optional unit testing
 framework using `assert`. Must be defined equal to a (random) filler function,
 satisfying <typedef:<PH>biaction_fn>. Provides tests for the base code and all later traits. Requires at least one `HEAP_TO_STRING` trait.

 @param[HEAP_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[HEAP_TO_STRING_NAME, HEAP_TO_STRING]
 To string trait contained in <to_string.h>; an optional unique `<Z>`
 that satisfies `C` naming conventions when mangled and function implementing
 <typedef:<PZ>to_string_fn>.

 @depend [array](https://github.com/neil-edelman/array)
 @std C89
 @fixme Add decrease priority. */


#ifndef HEAP_NAME
#error Generic HEAP_NAME undefined.
#endif
#if defined(HEAP_TO_STRING_NAME) || defined(HEAP_TO_STRING) /* <!-- str */
#define HEAP_TO_STRING_TRAIT 1
#else /* str --><!-- !str */
#define HEAP_TO_STRING_TRAIT 0
#endif /* !str --> */
#define HEAP_TRAITS HEAP_TO_STRING_TRAIT
#if HEAP_TRAITS > 1
#error Only one trait per include is allowed; use HEAP_EXPECT_TRAIT.
#endif
#if HEAP_TRAITS != 0 && (!defined(H_) || !defined(CAT) || !defined(CAT_))
#error Use HEAP_EXPECT_TRAIT in include it again.
#endif
#if defined(HEAP_TO_STRING_NAME) && !defined(HEAP_TO_STRING)
#error HEAP_TO_STRING_NAME requires HEAP_TO_STRING.
#endif


#if HEAP_TRAITS == 0 /* <!-- base code */


/* <Kernighan and Ritchie, 1988, p. 231>. */
#if defined(H_) || defined(PH_) \
	|| (defined(HEAP_SUBTYPE) ^ (defined(CAT) || defined(CAT_)))
#error Unexpected P?H_ or CAT_?; possible stray HEAP_EXPECT_TRAIT?
#endif
#ifndef HEAP_SUBTYPE /* <!-- !sub-type */
#define CAT_(x, y) x ## _ ## y
#define CAT(x, y) CAT_(x, y)
#endif /* !sub-type --> */
#define H_(n) CAT(HEAP_NAME, n)
#define PH_(n) CAT(heap, H_(n))
#ifndef HEAP_TYPE
#define HEAP_TYPE unsigned
#endif

/** Valid assignable type used for priority in <typedef:<PH>node>. Defaults to
 `unsigned int` if not set by `HEAP_TYPE`. */
typedef HEAP_TYPE PH_(priority);

/** Returns a positive result if `a` comes after `b`, inducing a strict
 pre-order of `a` with respect to `b`; this is compatible, but less strict then
 the comparators from `bsearch` and `qsort`; it only needs to divide entries
 into two instead of three categories. The default `HEAP_COMPARE` is `a > b`,
 which makes a minimum-hash. */
typedef int (*PH_(compare_fn))(const PH_(priority) a, const PH_(priority) b);
#ifndef HEAP_COMPARE /* <!-- !cmp */
/** Pre-order with `a` and `b`. @implements <typedef:<PH>compare_fn> */
static int PH_(default_compare)(const PH_(priority) a, const PH_(priority) b)
	{ return a > b; }
#define HEAP_COMPARE &PH_(default_compare)
#endif /* !cmp --> */
/* Check that `HEAP_COMPARE` is a function implementing
 <typedef:<PH>compare_fn>, if defined. */
static const PH_(compare_fn) PH_(compare) = (HEAP_COMPARE);

#ifdef HEAP_VALUE /* <!-- value */
/** If `HEAP_VALUE` is set, a declared tag type. */
typedef HEAP_VALUE PH_(adjunct);
/** If `HEAP_VALUE` is set, this is a pointer to it, otherwise a boolean
 value that is true when there is an item. */
typedef PH_(adjunct) *PH_(value);
/** If `HEAP_VALUE` is set, creates a value as the payload of
 <typedef:<PH>node>. */
struct H_(heap_node) { PH_(priority) priority; PH_(value) value; };
/** Internal nodes in the heap. If `HEAP_VALUE` is set, this is a
 <tag:<PH>heap_node>, otherwise it's the same as <typedef:<PH>priority>. */
typedef struct H_(heap_node) PH_(node);
/** Copies `priority` and `value` into a structure for use in  */
/*const struct PH_(store) H_(heap_make_node)()...*/
#else /* value --><!-- !value */
typedef int PH_(value);
typedef PH_(priority) PH_(node);
#endif /* !value --> */

/* This relies on `array.h` which must be in the same directory. */
#define ARRAY_NAME PH_(node)
#define ARRAY_TYPE PH_(node)
#define ARRAY_SUBTYPE
#include "array.h"

/** Stores the heap as an implicit binary tree in an array called `a`. To
 initialize it to an idle state, see <fn:<H>heap>, `HEAP_IDLE`, `{0}` (`C99`),
 or being `static`.

 ![States.](../web/states.png) */
struct H_(heap);
struct H_(heap) { struct PH_(node_array) a; };
#ifndef HEAP_IDLE /* <!-- !zero */
#define HEAP_IDLE { ARRAY_IDLE }
#endif /* !zero --> */

/** Extracts the <typedef:<PH>priority> of `node`, which must not be null. */
static PH_(priority) PH_(get_priority)(const PH_(node) *const node) {
#ifdef HEAP_VALUE /* <-- value */
	return node->priority;
#else /* value --><!-- !value */
	return *node;
#endif /* !value --> */
}

/** Extracts the <typedef:<PH>value> of `node`, which must not be null. */
static PH_(value) PH_(get_value)(const PH_(node) *const node) {
#ifdef HEAP_VALUE /* <-- value */
	return node->value;
#else /* value --><!-- !value */
	(void)(node);
	return 1;
#endif /* !value --> */
}

/** Extracts the <typedef:<PH>value> of `node`, which could be null. */
static PH_(value) PH_(value_or_null)(const PH_(node) *const node)
	{ return node ? PH_(get_value)(node) : 0; }

/** Copies `src` to `dest`. */
static void PH_(copy)(const PH_(node) *const src, PH_(node) *const dest) {
#ifdef HEAP_VALUE /* <!-- value */
	dest->priority = src->priority;
	dest->value = src->value;
#else /* value --><!-- !value */
	*dest = *src;
#endif /* !value --> */
}

/** Find the spot in `heap` where `node` goes and put it there.
 @param[heap] At least one entry; the last entry will be replaced by `node`.
 @order \O(log `size`) */
static void PH_(sift_up)(struct H_(heap) *const heap, PH_(node) *const node) {
	PH_(node) *const n0 = heap->a.data;
	PH_(priority) p = PH_(get_priority)(node);
	size_t i = heap->a.size - 1;
	assert(heap && heap->a.size && node);
	if(i) {
		size_t i_up;
		do { /* Note: don't change the `<=`; it's a queue. */
			i_up = (i - 1) >> 1;
			if(PH_(compare)(PH_(get_priority)(n0 + i_up), p) <= 0) break;
			PH_(copy)(n0 + i_up, n0 + i);
		} while((i = i_up));
	}
	PH_(copy)(node, n0 + i);
}

/** Pop the head of `heap` and restore the heap by sifting down the last
 element. @param[heap] At least one entry. The head is popped, and the size
 will be one less. */
static void PH_(sift_down)(struct H_(heap) *const heap) {
	const size_t size = (assert(heap && heap->a.size), --heap->a.size),
		half = size >> 1;
	size_t i = 0, c;
	PH_(node) *const n0 = heap->a.data,
		*const down = n0 + size /* Put it at the top. */, *child;
	const PH_(priority) down_p = PH_(get_priority)(down);
	while(i < half) {
		c = (i << 1) + 1;
		if(c + 1 < size && PH_(compare)(PH_(get_priority)(n0 + c),
			PH_(get_priority)(n0 + c + 1)) > 0) c++;
		child = n0 + c;
		if(PH_(compare)(down_p, PH_(get_priority)(child)) <= 0) break;
		PH_(copy)(child, n0 + i);
		i = c;
	}
	PH_(copy)(down, n0 + i);
}

/** Restore the `heap` by permuting the elements so `i` is in the proper place.
 This reads from the an arbitrary leaf-node into a temporary value, so is
 slightly more complex than <fn:<PH>sift_down>, but the same thing.
 @param[heap] At least `i + 1` entries. */
static void PH_(sift_down_i)(struct H_(heap) *const heap, size_t i) {
	const size_t size = (assert(heap && i < heap->a.size), heap->a.size),
		half = size >> 1;
	size_t c;
	PH_(node) *const n0 = heap->a.data, *child, temp;
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
			PH_(copy)(n0 + i, &temp), temp_valid = 1;
		}
		PH_(copy)(child, n0 + i);
		i = c;
	}
	if(temp_valid) PH_(copy)(&temp, n0 + i);
}

/** Create a `heap` from an array. @order \O(`heap.size`) */
static void PH_(heapify)(struct H_(heap) *const heap) {
	size_t i;
	assert(heap);
	if(heap->a.size)
		for(i = (heap->a.size >> 1) - 1; (PH_(sift_down_i)(heap, i), i); i--);
}

/** Removes from `heap`. Must have a non-zero size. */
static PH_(node) PH_(remove)(struct H_(heap) *const heap) {
	const PH_(node) result = *heap->a.data;
	assert(heap);
	if(heap->a.size > 1) {
		PH_(sift_down)(heap);
	} else {
		assert(heap->a.size == 1);
		heap->a.size = 0;
	}
	return result;
}

/** Initializes `heap` to be idle. @order \Theta(1) @allow */
static void H_(heap)(struct H_(heap) *const heap)
	{ assert(heap), PH_(node_array)(&heap->a); }

/** Returns `heap` to the idle state where it takes no dynamic memory.
 @order \Theta(1) @allow */
static void H_(heap_)(struct H_(heap) *const heap)
	{ assert(heap), PH_(node_array_)(&heap->a); }

/** Sets `heap` to be empty. That is, the size of `heap` will be zero, but if
 it was previously in an active non-idle state, it continues to be.
 @param[heap] If null, does nothing. @order \Theta(1) @allow */
static void H_(heap_clear)(struct H_(heap) *const heap)
	{ assert(heap), PH_(node_array_clear)(&heap->a); }

/** Copies `node` into `heap`.
 @return Success. @throws[ERANGE, realloc] @order \O(log `heap.size`) @allow */
static int H_(heap_add)(struct H_(heap) *const heap, PH_(node) node) {
	assert(heap);
	return PH_(node_array_new)(&heap->a) && (PH_(sift_up)(heap, &node), 1);
}

/** @return Lowest in `heap` according to `HEAP_COMPARE` or null if the heap is
 empty. This pointer is valid only until one makes structural changes to the
 heap. @order \O(1) @allow */
static PH_(node) *H_(heap_peek)(const struct H_(heap) *const heap)
	{ return assert(heap), heap->a.size ? heap->a.data : 0; }

/** This returns the <typedef:<PH>value> of the <typedef:<PH>node> returned by
 <fn:<H>heap_peek>, for convenience with some applications. If `HEAP_VALUE`,
 this is a child of <fn:<H>heap_peek>, otherwise it is a boolean `int`.
 @return Lowest <typedef:<PH>value> in `heap` element according to
 `HEAP_COMPARE`; if the heap is empty, null or zero. @order \O(1) @allow */
static PH_(value) H_(heap_peek_value)(struct H_(heap) *const heap)
	{ return PH_(value_or_null)(H_(heap_peek)(heap)); }

/** Remove the lowest element according to `HEAP_COMPARE`.
 @param[heap] If null, returns false. @return The <typedef:<PH>value> of the
 element that was removed; if the heap is empty, null or zero.
 @order \O(log `size`) @allow */
static PH_(value) H_(heap_pop)(struct H_(heap) *const heap) {
	PH_(node) n;
	return assert(heap), heap->a.size
		? (n = PH_(remove)(heap), PH_(get_value)(&n)) : 0;
}

/** The capacity of `heap` will be increased to at least `buffer` elements
 beyond the size. Invalidates pointers in `a`.
 @return The start of the buffered space. If `a` is idle and `buffer` is zero,
 a null pointer is returned, otherwise null indicates an error.
 @throws[realloc, ERANGE] @allow */
static PH_(node) *H_(heap_buffer)(struct H_(heap) *const heap,
	const size_t n) { return PH_(node_array_buffer)(&heap->a, n); }

/** Adds and heapifies `add` elements to `heap`. Uses <Doberkat, 1984, Floyd>
 to sift-down all the internal nodes of heap, including any previous elements.
 As such, this function is most efficient on a heap of zero size, and becomes
 increasingly inefficient as the heap grows. For heaps that are already in use,
 it may be better to add each element individually, resulting in a run-time of
 \O(`new elements` \cdot log `heap.size`).
 @param[add] If zero, returns true.
 @return Success. @throws[ERANGE, realloc] In practice, pushing uninitialized
 elements onto the heap does make sense, so <fn:<H>heap_buffer> `n` will be
 called first, in which case, one is guaranteed success.
 @order \O(`heap.size` + `add`) @allow */
static int H_(heap_append)(struct H_(heap) *const heap, const size_t n) {
	assert(heap);
	PH_(node_array_append)(&heap->a, n);
	if(n) PH_(heapify)(heap);
	return 1;
}

/* <!-- iterate interface */
#define BOX_ITERATE
#define PA_(n) CAT(array, CAT(PH_(node), n))
struct PH_(iterator);
struct PH_(iterator) { struct PA_(iterator) a; };
static void PH_(begin)(struct PH_(iterator) *const it,
	const struct H_(heap) *const h) { PA_(begin)(&it->a, &h->a); }
static PH_(node) *PH_(next)(struct PH_(iterator) *const it)
	{ return PA_(next)(&it->a); }
#undef PA_
/* iterate --> */


/* Define these for traits. */
#define BOX_ PH_
#define BOX_CONTAINER struct H_(heap)
#define BOX_CONTENTS PH_(node)

#ifdef HEAP_TEST /* <!-- test */
/* Forward-declare. */
static void (*PH_(to_string))(const PH_(node) *, char (*)[12]);
static const char *(*PH_(heap_to_string))(const struct H_(heap) *);
#include "../test/test_heap.h" /** \include */
#endif /* test --> */

static void PH_(unused_base_coda)(void);
static void PH_(unused_base)(void) {
	H_(heap)(0); H_(heap_)(0); H_(heap_clear)(0); H_(heap_peek_value)(0);
	H_(heap_pop)(0); H_(heap_buffer)(0, 0); H_(heap_append)(0, 0);
	PH_(begin)(0, 0); PH_(next)(0); PH_(unused_base_coda)();
}
static void PH_(unused_base_coda)(void) { PH_(unused_base)(); }


#elif defined(HEAP_TO_STRING) /* base code --><!-- to string trait */


#ifdef HEAP_TO_STRING_NAME /* <!-- name */
#define Z_(n) CAT(H_(heap), CAT(HEAP_TO_STRING_NAME, n))
#else /* name --><!-- !name */
#define Z_(n) CAT(H_(heap), n)
#endif /* !name --> */
#define TO_STRING HEAP_TO_STRING
#include "to_string.h" /** \include */
#ifdef HEAP_TEST /* <!-- expect: we've forward-declared these. */
#undef HEAP_TEST
static void (*PH_(to_string))(const PH_(node) *, char (*)[12]) = PZ_(to_string);
static const char *(*PH_(heap_to_string))(const struct H_(heap) *)
	= &Z_(to_string);
#endif /* expect --> */
#undef PZ_
#undef Z_
#undef HEAP_TO_STRING
#ifdef HEAP_TO_STRING_NAME
#undef HEAP_TO_STRING_NAME
#endif

static void PH_(unused_to_string_coda)(void);
static void PH_(unused_to_string)(void) { H_(heap_to_string)(0);
	PH_(unused_to_string_coda)(); }
static void PH_(unused_to_string_coda)(void) { PH_(unused_to_string)(); }

#endif /* traits --> */


#ifdef HEAP_EXPECT_TRAIT /* <!-- trait */
#undef HEAP_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#if defined(HEAP_TEST)
#error No to string traits defined for test.
#endif
#ifndef HEAP_SUBTYPE /* <!-- !sub-type */
#undef CAT
#undef CAT_
#else /* !sub-type --><!-- sub-type */
#undef HEAP_SUBTYPE
#endif /* sub-type --> */
#undef H_
#undef PH_
#undef HEAP_NAME
#undef HEAP_TYPE
#undef HEAP_COMPARE
#ifdef HEAP_VALUE
#undef HEAP_VALUE
#endif
#ifdef HEAP_TEST
#undef HEAP_TEST
#endif
#ifdef HEAP_TEST_BASE
#undef HEAP_TEST_BASE
#endif
#undef BOX_
#undef BOX_CONTAINER
#undef BOX_CONTENTS
#undef BOX_ITERATE
#endif /* !trait --> */
#undef HEAP_TO_STRING_TRAIT
#undef HEAP_TRAITS

/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Priority-queue

 ![Example of heap.](../web/heap.png)

 A <tag:<H>heap> is a binary heap, proposed by
 <Williams, 1964, Heapsort, p. 347> using terminology of
 <Knuth, 1973, Sorting>. It can be used as an implementation of a priority
 queue; internally, it is a `<<PH>node>array` with implicit heap properties on
 <typedef:<PH>priority> and an optional <typedef:<PH>value> pointer value.

 @param[HEAP_NAME, HEAP_TYPE]
 `<H>` that satisfies `C` naming conventions when mangled and an assignable
 type <typedef:<PH>priority> associated therewith. `HEAP_NAME` is required;
 `HEAP_TYPE` defaults to `unsigned int`. `<PH>` is private, whose names are
 prefixed in a manner to avoid collisions.

 @param[HEAP_COMPARE]
 A function satisfying <typedef:<PH>compare_fn>. Defaults to minimum-hash.
 Required if `HEAP_TYPE` is changed to an incomparable type.

 @param[HEAP_VALUE]
 Optional value <typedef:<PH>value>, that is stored as a reference in
 <tag:<H>heap_node>; declaring it is sufficient. If set, has no effect on the
 ranking, but affects <typedef:<PH>value>.

 @param[HEAP_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a parameterized
 trait.

 @param[HEAP_TO_STRING_NAME, HEAP_TO_STRING]
 To string trait contained in <to_string.h>; an optional unique `<SZ>`
 that satisfies `C` naming conventions when mangled and function implementing
 <typedef:<PSZ>to_string_fn>.

 @depend [array](https://github.com/neil-edelman/array)
 @std C89
 @fixme Add decrease priority.
 @fixme Add replace. */

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
#if defined(HEAP_TO_STRING_NAME) && !defined(HEAP_TO_STRING)
#error HEAP_TO_STRING_NAME requires HEAP_TO_STRING.
#endif

#ifndef HEAP_H /* <!-- idempotent */
#define HEAP_H
#if defined(HEAP_CAT_) || defined(HEAP_CAT) || defined(H_) || defined(PH_) \
	|| defined(HEAP_IDLE)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define HEAP_CAT_(n, m) n ## _ ## m
#define HEAP_CAT(n, m) HEAP_CAT_(n, m)
#define H_(n) HEAP_CAT(HEAP_NAME, n)
#define PH_(n) HEAP_CAT(heap, H_(n))
#define HEAP_IDLE { ARRAY_IDLE }
#endif /* idempotent --> */


#if HEAP_TRAITS == 0 /* <!-- base code */


#ifndef HEAP_TYPE
#define HEAP_TYPE unsigned
#endif

/** Valid assignable type used for priority in <typedef:<PH>node>. Defaults to
 `unsigned int` if not set by `HEAP_TYPE`. */
typedef HEAP_TYPE PH_(priority);

/** Returns a positive result if `a` is out-of-order with respect to `b`,
 inducing a strict pre-order. This is compatible, but less strict then the
 comparators from `bsearch` and `qsort`; it only needs to divide entries into
 two instead of three categories. */
typedef int (*PH_(compare_fn))(const PH_(priority) a, const PH_(priority) b);
#ifndef HEAP_COMPARE /* <!-- !cmp */
/** The default `HEAP_COMPARE` on `a` and `b` is `a > b`, which makes a
 minimum-hash. @implements <typedef:<PH>compare_fn> */
static int PH_(default_compare)(const PH_(priority) a, const PH_(priority) b)
	{ return a > b; }
#define HEAP_COMPARE &PH_(default_compare)
#endif /* !cmp --> */
/* Check that `HEAP_COMPARE` is a function implementing
 <typedef:<PH>compare_fn>, if defined. */
static const PH_(compare_fn) PH_(compare) = (HEAP_COMPARE);

#ifdef HEAP_VALUE /* <!-- value */
typedef HEAP_VALUE PH_(value_data);
typedef PH_(value_data) *PH_(value);
/** If `HEAP_VALUE` is set, this becomes <typedef:<PH>node>. Memory management
 for this structure is the responsibility of the caller. */
struct H_(heap_node) { PH_(priority) priority; PH_(value) value; };
/** If `HEAP_VALUE` is set, (priority, value) set by <tag:<H>heap_node>,
 otherwise it's a (priority) set directly by <typedef:<PH>priority>. */
typedef struct H_(heap_node) PH_(node);
#else /* value --><!-- !value */
typedef PH_(priority) PH_(value);
typedef PH_(priority) PH_(node);
#endif /* !value --> */

/* This relies on `array.h` which must be in the same directory. */
#define ARRAY_NAME PH_(node)
#define ARRAY_TYPE PH_(node)
#include "array.h"

/** Stores the heap as an implicit binary tree in an array called `a`. To
 initialize it to an idle state, see <fn:<H>heap>, `HEAP_IDLE`, `{0}` (`C99`),
 or being `static`.

 ![States.](../web/states.png) */
struct H_(heap) { struct PH_(node_array) a; };

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
#ifdef HEAP_VALUE /* <-- value */
	return node->value;
#else /* value --><!-- !value */
	return *node;
#endif /* !value --> */
}

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
	if(heap->a.size > 1)
		for(i = heap->a.size / 2 - 1; (PH_(sift_down_i)(heap, i), i); i--);
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

/** Empty is `!size`. @return Size of the `heap`. @allow */
static size_t H_(heap_size)(const struct H_(heap) *const heap)
	{ return assert(heap), heap->a.size; }

/** Copies `node` into `heap`.
 @return Success. @throws[ERANGE, realloc] @order \O(log `heap.size`) @allow */
static int H_(heap_add)(struct H_(heap) *const heap, PH_(node) node) {
	assert(heap);
	return PH_(node_array_new)(&heap->a) && (PH_(sift_up)(heap, &node), 1);
}

/** @return The lowest element in `heap` according to `HEAP_COMPARE` or
 null/zero if the heap is empty. On some heaps, one may have to call
 <fn:<H>heap_size> in order to differentiate. @order \O(1) @allow */
static PH_(value) H_(heap_peek)(const struct H_(heap) *const heap)
	{ return assert(heap), heap->a.size ? PH_(get_value)(heap->a.data) : 0; }

/** Remove the lowest element in `heap` according to `HEAP_COMPARE`.
 @return The same as <fn:<H>heap_peek>. @order \O(log `size`) @allow */
static PH_(value) H_(heap_pop)(struct H_(heap) *const heap) {
	PH_(node) n;
	return assert(heap), heap->a.size
		? (n = PH_(remove)(heap), PH_(get_value)(&n)) : 0;
}

/** The capacity of `heap` will be increased to at least `n` elements beyond
 the size. Invalidates pointers in `heap.a`. All the elements in `heap.a.size`
 are part of the heap, but `heap.a.size` <= `index` < `heap.a.capacity`
 can be used to construct new elements without immediately making them part of
 the heap, then <fn:<H>heap_append>.
 @return The start of the buffered space. If `a` is idle and `buffer` is zero,
 a null pointer is returned, otherwise null indicates an error.
 @throws[realloc, ERANGE] @allow */
static PH_(node) *H_(heap_buffer)(struct H_(heap) *const heap,
	const size_t n) { return PH_(node_array_buffer)(&heap->a, n); }

/** Adds and heapifies `n` elements to `heap`. Uses <Floyd, 1964, Treesort> to
 sift-down all the internal nodes of heap. The heap elements must exist, see
 <fn:<H>heap_buffer>.
 @param[n] If zero, returns true without heapifying.
 @return Success. @order \O(`heap.size` + `n`) <Doberkat, 1984, Floyd> @allow */
static void H_(heap_append)(struct H_(heap) *const heap, const size_t n) {
	PH_(node) *more;
	/* In practice, pushing uninitialized elements onto the heap does not make
	 sense, so we assert that the elements exist first. */
	assert(heap && n <= heap->a.capacity - heap->a.size);
	more = PH_(node_array_append)(&heap->a, n), assert(more);
	if(n) PH_(heapify)(heap);
}

/** Shallow-copies and heapifies `master` into `heap`.
 @param[master] If null, does nothing. @return Success.
 @order \O(`heap.size` + `copy.size`) @throws[ERANGE, realloc] @allow */
static int H_(heap_affix)(struct H_(heap) *const heap,
	const struct H_(heap) *const master) {
	PH_(node) *n;
	assert(heap);
	if(!master || !master->a.size) return 1;
	assert(master->a.data);
	if(!(n = PH_(node_array_buffer)(&heap->a, master->a.size))) return 0;
	memcpy(n, master->a.data, sizeof *n * master->a.size);
	n = PH_(node_array_append)(&heap->a, master->a.size), assert(n);
	PH_(heapify)(heap);
	return 1;
}

/* <!-- iterate interface: Forward the responsibility to array. */
#define PAH_(n) HEAP_CAT(array, HEAP_CAT(PH_(node), n))
/** Contains all the iteration parameters. */
struct PH_(iterator);
struct PH_(iterator) { struct PAH_(iterator) a; };
/** Begins the forward iteration `it` at `h`. */
static void PH_(begin)(struct PH_(iterator) *const it,
	const struct H_(heap) *const h) { PAH_(begin)(&it->a, &h->a); }
/** @return The next `it` or null. */
static PH_(node) *PH_(next)(struct PH_(iterator) *const it)
	{ return PAH_(next)(&it->a); }
#undef PAH_
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
	PH_(node) lol;
	memset(&lol, 0, sizeof lol);
	H_(heap)(0); H_(heap_)(0); H_(heap_clear)(0); H_(heap_size)(0);
	H_(heap_add)(0, lol); H_(heap_peek)(0); H_(heap_pop)(0);
	H_(heap_buffer)(0, 0); H_(heap_append)(0, 0); H_(heap_affix)(0, 0);
	PH_(begin)(0, 0); PH_(next)(0); PH_(unused_base_coda)();
}
static void PH_(unused_base_coda)(void) { PH_(unused_base)(); }


#elif defined(HEAP_TO_STRING) /* base code --><!-- to string trait */


#ifdef HEAP_TO_STRING_NAME /* <!-- name */
#define SZ_(n) HEAP_CAT(H_(heap), HEAP_CAT(HEAP_TO_STRING_NAME, n))
#else /* name --><!-- !name */
#define SZ_(n) HEAP_CAT(H_(heap), n)
#endif /* !name --> */
#define TO_STRING HEAP_TO_STRING
#include "to_string.h" /** \include */
#ifdef HEAP_TEST /* <!-- expect: greedy satisfy forward-declared. */
#undef HEAP_TEST
static PSZ_(to_string_fn) PH_(to_string) = PSZ_(to_string);
static const char *(*PH_(heap_to_string))(const struct H_(heap) *)
	= &SZ_(to_string);
#endif /* expect --> */
#undef SZ_
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
#error No HEAP_TO_STRING traits defined for HEAP_TEST.
#endif
#undef HEAP_NAME
#undef HEAP_TYPE
#undef HEAP_COMPARE
#ifdef HEAP_VALUE
#undef HEAP_VALUE
#endif
#undef BOX_
#undef BOX_CONTAINER
#undef BOX_CONTENTS
/* box (multiple traits) --> */
#endif /* !trait --> */
#undef HEAP_TO_STRING_TRAIT
#undef HEAP_TRAITS

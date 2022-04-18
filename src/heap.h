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
 queue; internally, it is an array with implicit heap properties on
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

 @param[HEAP_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a parameterized
 trait.

 @param[HEAP_TO_STRING_NAME, HEAP_TO_STRING]
 To string trait contained in <src/to_string.h>. An optional mangled name for
 uniqueness and function implementing <typedef:<PSTR>to_string_fn>.

 @depend [array](https://github.com/neil-edelman/array)
 @std C89 */

#ifndef HEAP_NAME
#error Name HEAP_NAME undefined.
#endif
#if defined(HEAP_TO_STRING_NAME) || defined(HEAP_TO_STRING)
#define HEAP_TO_STRING_TRAIT 1
#else
#define HEAP_TO_STRING_TRAIT 0
#endif
#define HEAP_TRAITS HEAP_TO_STRING_TRAIT
#if HEAP_TRAITS > 1
#error Only one trait per include is allowed; use HEAP_EXPECT_TRAIT.
#endif
#if defined(HEAP_TO_STRING_NAME) && !defined(HEAP_TO_STRING)
#error HEAP_TO_STRING_NAME requires HEAP_TO_STRING.
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


#if HEAP_TRAITS == 0 /* <!-- base code */


#ifndef HEAP_TYPE
#define HEAP_TYPE unsigned
#endif

/** Valid assignable type used for priority in <typedef:<PH>node>. Defaults to
 `unsigned int` if not set by `HEAP_TYPE`. */
typedef HEAP_TYPE PH_(priority);

/** Returns a positive result if `a` is out-of-order with respect to `b`,
 inducing a strict total order. This is compatible, but less strict then the
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
typedef HEAP_VALUE PH_(value);
typedef const HEAP_VALUE PH_(value_c);
/** If `HEAP_VALUE` is set, this becomes <typedef:<PH>node>. */
struct H_(heapnode) { PH_(priority) priority; PH_(value) value; };
/** If `HEAP_VALUE` is set, (priority, value) set by <tag:<H>heapnode>,
 otherwise it's a (priority) set directly by <typedef:<PH>priority>. */
typedef struct H_(heapnode) PH_(node);
#else /* value --><!-- !value */
typedef PH_(priority) PH_(value);
typedef PH_(priority) PH_(node);
#endif /* !value --> */

/* This relies on <src/array.h> which must be in the same directory. */
#define ARRAY_NAME PH_(node)
#define ARRAY_TYPE PH_(node)
#include "array.h"

/* Box override information. */
#define BOX_ PH_
#define BOX struct H_(heap)

/** Stores the heap as an implicit binary tree in an array called `a`. To
 initialize it to an idle state, see <fn:<H>heap>, `HEAP_IDLE`, `{0}` (`C99`),
 or being `static`.

 ![States.](../doc/states.png) */
struct H_(heap) { struct PH_(node_array) _; };

#define BOX_CONTENT PH_(node) *
#define PAH_(n) HEAP_CAT(HEAP_CAT(array, PH_(node)), n)
/** Is `x` not null? @implements `is_content` */
static int PH_(is_content)(const PH_(node) *const x) { return !!x; }
/* @implements `forward` */
struct PH_(forward) { struct PAH_(forward) _; };
/** @return Before `h`. @implements `forward_begin` */
static struct PH_(forward) PH_(forward_begin)(const struct H_(heap) *const h) {
	struct PH_(forward) it; it._ = PAH_(forward_begin)(&h->_); return it; }
/** @return The next `it` or null. @implements `forward_next` */
static const PH_(node) *PH_(forward_next)(struct PH_(forward) *const it)
	{ return PAH_(forward_next)(&it->_); }
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
	PH_(node) *const n0 = heap->_.data;
	PH_(priority) p = PH_(get_priority)(node);
	size_t i = heap->_.size - 1;
	assert(heap && heap->_.size && node);
	if(i) {
		size_t i_up;
		do { /* Note: don't change the `<=`; it's a queue. */
			i_up = (i - 1) >> 1;
			if(PH_(compare)(PH_(get_priority)(n0 + i_up), p) <= 0) break;
			n0[i] = n0[i_up]; /*PH_(copy)(n0 + i_up, n0 + i);*/
		} while((i = i_up));
	}
	n0[i] = *node; /*PH_(copy)(node, n0 + i);*/
}

/** Pop the head of `heap` and restore the heap by sifting down the last
 element. @param[heap] At least one entry. The head is popped, and the size
 will be one less. */
static void PH_(sift_down)(struct H_(heap) *const heap) {
	const size_t size = (assert(heap && heap->_.size), --heap->_.size),
		half = size >> 1;
	size_t i = 0, c;
	PH_(node) *const n0 = heap->_.data,
		*const down = n0 + size /* Put it at the top. */, *child;
	const PH_(priority) down_p = PH_(get_priority)(down);
	while(i < half) {
		c = (i << 1) + 1;
		if(c + 1 < size && PH_(compare)(PH_(get_priority)(n0 + c),
			PH_(get_priority)(n0 + c + 1)) > 0) c++;
		child = n0 + c;
		if(PH_(compare)(down_p, PH_(get_priority)(child)) <= 0) break;
		n0[i] = *child; /*PH_(copy)(child, n0 + i);*/
		i = c;
	}
	n0[i] = *down;/*PH_(copy)(down, n0 + i);*/
}

/** Restore the `heap` by permuting the elements so `i` is in the proper place.
 This reads from the an arbitrary leaf-node into a temporary value, so is
 slightly more complex than <fn:<PH>sift_down>, but the same thing.
 @param[heap] At least `i + 1` entries. */
static void PH_(sift_down_i)(struct H_(heap) *const heap, size_t i) {
	const size_t size = (assert(heap && i < heap->_.size), heap->_.size),
		half = size >> 1;
	size_t c;
	/* Uninitialized variable warning suppression. */
	PH_(node) *const n0 = heap->_.data, *child, temp = *(&temp);
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
			temp = n0[i]/*PH_(copy)(n0 + i, &temp)*/, temp_valid = 1;
		}
		n0[i] = *child;/*PH_(copy)(child, n0 + i)*/;
		i = c;
	}
	if(temp_valid) n0[i] = temp/*PH_(copy)(&temp, n0 + i)*/;
}

/** Create a `heap` from an array. @order \O(`heap.size`) */
static void PH_(heapify)(struct H_(heap) *const heap) {
	size_t i;
	assert(heap);
	if(heap->_.size > 1)
		for(i = heap->_.size / 2 - 1; (PH_(sift_down_i)(heap, i), i); i--);
}

/** Removes from `heap`. Must have a non-zero size. */
static PH_(node) PH_(remove)(struct H_(heap) *const heap) {
	const PH_(node) result = *heap->_.data;
	assert(heap);
	if(heap->_.size > 1) {
		PH_(sift_down)(heap);
	} else {
		assert(heap->_.size == 1);
		heap->_.size = 0;
	}
	return result;
}

/** Zeroed data (not all-bits-zero) is initialised.
 @return An idle heap. @order \Theta(1) @allow */
static struct H_(heap) H_(heap)(void)
	{ struct H_(heap) heap; heap._ = PH_(node_array)(); return heap; }

/** Returns `heap` to the idle state where it takes no dynamic memory.
 @order \Theta(1) @allow */
static void H_(heap_)(struct H_(heap) *const heap)
	{ if(heap) PH_(node_array_)(&heap->_); }

/** Sets `heap` to be empty. That is, the size of `heap` will be zero, but if
 it was previously in an active non-idle state, it continues to be.
 @param[heap] If null, does nothing. @order \Theta(1) @allow */
static void H_(heap_clear)(struct H_(heap) *const heap)
	{ assert(heap), PH_(node_array_clear)(&heap->_); }

/** @return If the `heap` is not null, returns it's size. @allow */
static size_t H_(heap_size)(const struct H_(heap) *const heap)
	{ return heap ? heap->_.size : 0; }

/** Copies `node` into `heap`.
 @return Success. @throws[ERANGE, realloc] @order \O(log `heap.size`) @allow */
static int H_(heap_add)(struct H_(heap) *const heap, PH_(node) node) {
	assert(heap);
	return PH_(node_array_new)(&heap->_) && (PH_(sift_up)(heap, &node), 1);
}

/** @return The value of the lowest element in `heap` or null when the heap is
 empty. @order \O(1) @allow */
static PH_(node) *H_(heap_peek)(const struct H_(heap) *const heap)
	{ return assert(heap), heap->_.size ? heap->_.data : 0; }

/** Only defined when <fn:<H>heap_size> returns true. Removes the lowest
 element. @return The value of the lowest element in `heap`.
 @order \O(\log `size`) @allow */
static PH_(value) H_(heap_pop)(struct H_(heap) *const heap) {
	PH_(node) n;
	return assert(heap && heap->_.size), (n = PH_(remove)(heap), PH_(get_value)(&n));
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
	const size_t n) { return PH_(node_array_buffer)(&heap->_, n); }

/** Adds and heapifies `n` elements to `heap`. Uses <Floyd, 1964, Treesort> to
 sift-down all the internal nodes of heap. The heap elements must exist, see
 <fn:<H>heap_buffer>.
 @param[n] If zero, returns true without heapifying.
 @return Success. @order \O(`heap.size` + `n`) <Doberkat, 1984, Floyd> @allow */
static void H_(heap_append)(struct H_(heap) *const heap, const size_t n) {
	PH_(node) *more;
	/* In practice, pushing uninitialized elements onto the heap does not make
	 sense, so we assert that the elements exist first. */
	assert(heap && n <= heap->_.capacity - heap->_.size);
	more = PH_(node_array_append)(&heap->_, n), assert(more);
	if(n) PH_(heapify)(heap);
}

/** Shallow-copies and heapifies `master` into `heap`.
 @param[master] If null, does nothing. @return Success.
 @order \O(`heap.size` + `copy.size`) @throws[ERANGE, realloc] @allow */
static int H_(heap_affix)(struct H_(heap) *restrict const heap,
	const struct H_(heap) *restrict const master) {
	PH_(node) *n;
	assert(heap);
	if(!master || !master->_.size) return 1;
	assert(master->_.data);
	if(!(n = PH_(node_array_buffer)(&heap->_, master->_.size))) return 0;
	memcpy(n, master->_.data, sizeof *n * master->_.size);
	n = PH_(node_array_append)(&heap->_, master->_.size), assert(n);
	PH_(heapify)(heap);
	return 1;
}

#ifdef HEAP_TEST /* <!-- test */
/* Forward-declare. */
static void (*PH_(to_string))(const PH_(node) *, char (*const)[12]);
static const char *(*PH_(heap_to_string))(const struct H_(heap) *);
#include "../test/test_heap.h"
#endif /* test --> */

static void PH_(unused_base_coda)(void);
static void PH_(unused_base)(void) {
	PH_(node) unused; memset(&unused, 0, sizeof unused);
	PH_(is_content)(0); PH_(forward_begin)(0); PH_(forward_next)(0);
	H_(heap)(); H_(heap_)(0); H_(heap_clear)(0); H_(heap_size)(0);
	H_(heap_add)(0, unused); H_(heap_peek)(0); H_(heap_pop)(0);
	H_(heap_buffer)(0, 0); H_(heap_append)(0, 0); H_(heap_affix)(0, 0);
	PH_(unused_base_coda)();
}
static void PH_(unused_base_coda)(void) { PH_(unused_base)(); }


#elif defined(HEAP_TO_STRING) /* base code --><!-- to string trait */


#ifdef HEAP_TO_STRING_NAME
#define STR_(n) HEAP_CAT(H_(heap), HEAP_CAT(HEAP_TO_STRING_NAME, n))
#else
#define STR_(n) HEAP_CAT(H_(heap), n)
#endif
#define TSTR_(n) HEAP_CAT(heap_sz, STR_(n))
#ifdef HEAP_VALUE /* <!-- value */
/* Check that `HEAP_TO_STRING` is a function implementing this prototype. */
static void (*const TSTR_(actual_to_string))(const PH_(value_c),
	char (*const)[12]) = (HEAP_TO_STRING);
/** Call <data:<TSTR>actual_to_string> with just the value of `node` and `z`. */
static void TSTR_(thunk_to_string)(const PH_(node) *const node,
	char (*const z)[12]) { TSTR_(actual_to_string)(node->value, z); }
#define TO_STRING &TSTR_(thunk_to_string)
#else
#define TO_STRING HEAP_TO_STRING
#endif
#include "to_string.h" /** \include */
#ifdef HEAP_TEST /* <!-- expect: greedy satisfy forward-declared. */
#undef HEAP_TEST
static PSTR_(to_string_fn) PH_(to_string) = PSTR_(to_string);
static const char *(*PH_(heap_to_string))(const struct H_(heap) *)
	= &STR_(to_string);
#endif /* expect --> */
#undef STR_
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
#ifdef HEAP_TEST
#error No HEAP_TO_STRING traits defined for HEAP_TEST.
#endif
#undef HEAP_NAME
#undef HEAP_TYPE
#undef HEAP_COMPARE
#ifdef HEAP_VALUE
#undef HEAP_VALUE
#endif
#undef BOX_
#undef BOX
#undef BOX_CONTENT
#endif /* !trait --> */
#undef HEAP_TO_STRING_TRAIT
#undef HEAP_TRAITS
#ifdef HEAP_RESTRICT
#undef HEAP_RESTRICT
#undef restrict
#endif

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
 Optional payload <typedef:<PH>value>, that is stored as a reference in
 <tag:<H>heap_node>; declaring it is sufficient.

 @param[HEAP_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[HEAP_TO_STRING_NAME, HEAP_TO_STRING]
 To string trait contained in <to_string.h>; `<Z>` that satisfies `C` naming
 conventions when mangled and function implementing <typedef:<PZ>to_string_fn>.
 There can be multiple to string traits, but only one can omit
 `HEAP_TO_STRING_NAME`.

 @param[HEAP_TEST]
 To string trait contained in <../test/heap_test.h>; optional unit testing
 framework using `assert`. Can only be defined once _per_ `heap`. Must be
 defined equal to a (random) filler function, satisfying
 <typedef:<PH>biaction_fn>. Output will be shown with the to string trait in
 which it's defined; provides tests for the base code and all later traits.

 @depend [array](https://github.com/neil-edelman/array)
 @std C89
 @fixme Add decrease priority. */


#ifndef HEAP_NAME
#error Generic HEAP_NAME undefined.
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
#if HEAP_TRAITS != 0 && (!defined(H_) || !defined(CAT) || !defined(CAT_))
#error H_ or CAT_? not yet defined; use HEAP_EXPECT_TRAIT?
#endif
#if (HEAP_TRAITS == 0) && defined(HEAP_TEST)
#error HEAP_TEST must be defined in HEAP_TO_STRING trait.
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
#define H_(thing) CAT(HEAP_NAME, thing)
#define PH_(thing) CAT(heap, H_(thing))
#ifndef HEAP_TYPE
#define HEAP_TYPE unsigned
#endif

/** Valid assignable type used for priority in <tag:<H>heap_node>. Defaults to
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
/** If `HEAP_VALUE` is set, a valid tag type, used as a pointer in
 <tag:<H>heap_node>. */
typedef HEAP_VALUE PH_(value);
/** If `HEAP_VALUE` is set, a pointer to the <typedef:<PH>value>, otherwise a
 boolean `int` that is true (one) if the value exists and false (zero) if
 not. */
typedef PH_(value) *PH_(pvalue);
#else /* value --><!-- !value */
typedef int PH_(pvalue);
#endif /* value --> */

/** Stores a <typedef:<PH>priority> as `priority`, which can be set by
 `HEAP_TYPE`. If `HEAP_VALUE` is set, also stores a pointer
 <typedef:<PH>pvalue> called `value`. */
struct H_(heap_node);
struct H_(heap_node) {
	PH_(priority) priority;
#ifdef HEAP_VALUE /* <!-- value */
	PH_(pvalue) value;
#endif /* value --> */
};

/* This relies on `array.h` which must be in the same directory. */
#define ARRAY_NAME H_(heap_node)
#define ARRAY_TYPE struct H_(heap_node)
#define ARRAY_SUBTYPE
#include "array.h"

/** Stores the heap as an implicit binary tree in an array. To initialise it to
 an idle state, see <fn:<H>heap>, `HEAP_IDLE`, `{0}` (`C99`), or being `static`.

 ![States.](../web/states.png) */
struct H_(heap);
struct H_(heap) { struct H_(heap_node_array) a; };
#ifndef HEAP_IDLE /* <!-- !zero */
#define HEAP_IDLE { { 0, 0, 0 } }
#endif /* !zero --> */

/** Extracts the <typedef:<PH>pvalue> of `node`, which must not be null. */
static PH_(pvalue) PH_(get_value)(const struct H_(heap_node) *const node) {
#ifdef HEAP_VALUE /* <-- value */
	return node->value;
#else /* value --><!-- !value */
	(void)(node);
	return 1;
#endif /* !value --> */
}

/** Extracts the <typedef:<PH>pvalue> of `node`, which could be null. */
static PH_(pvalue) PH_(value_or_null)(const struct H_(heap_node) *const node)
	{ return node ? PH_(get_value)(node) : 0; }

/** Copies `src` to `dest`. */
static void PH_(copy)(const struct H_(heap_node) *const src,
	struct H_(heap_node) *const dest) {
	dest->priority = src->priority;
#ifdef HEAP_VALUE /* <!-- value */
	dest->value = src->value;
#endif /* value --> */
}

/** Find the spot in `heap` where `node` goes and put it there.
 @param[heap] At least one entry; the last entry will be replaced by `node`.
 @order \O(log `size`) */
static void PH_(sift_up)(struct H_(heap) *const heap,
	struct H_(heap_node) *const node) {
	struct H_(heap_node) *const n0 = heap->a.data;
	size_t i = heap->a.size - 1;
	assert(heap && heap->a.size && node);
	if(i) {
		size_t i_up;
		do { /* Note: don't change the `<=`; it's a queue. */
			i_up = (i - 1) >> 1;
			if(PH_(compare)(n0[i_up].priority, node->priority) <= 0) break;
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
	struct H_(heap_node) *const n0 = heap->a.data,
		*const down = n0 + size /* Put it at the top. */, *child;
	while(i < half) {
		c = (i << 1) + 1;
		if(c + 1 < size && PH_(compare)(n0[c].priority,
			n0[c + 1].priority) > 0) c++;
		child = n0 + c;
		if(PH_(compare)(down->priority, child->priority) <= 0) break;
		PH_(copy)(child, n0 + i);
		i = c;
	}
	PH_(copy)(down, n0 + i);
}

/** Restore the `heap` by permuting the elements so `i` is in the proper place.
 This reads from the an arbitrary leaf-node into a temporary value, so is
 slightly more complex then <fn:<PH>sift_down>, but the same thing.
 @param[heap] At least `i + 1` entries. */
static void PH_(sift_down_i)(struct H_(heap) *const heap, size_t i) {
	const size_t size = (assert(heap && i < heap->a.size), heap->a.size),
		half = size >> 1;
	size_t c;
	struct H_(heap_node) *const n0 = heap->a.data, *child, temp;
	int temp_valid = 0;
	while(i < half) {
		c = (i << 1) + 1;
		if(c + 1 < size && PH_(compare)(n0[c].priority,
			n0[c + 1].priority) > 0) c++;
		child = n0 + c;
		if(temp_valid) {
			if(PH_(compare)(temp.priority, child->priority) <= 0) break;
		} else {
			/* Only happens on the first compare when `i` is in it's original
			 position. */
			if(PH_(compare)(n0[i].priority, child->priority) <= 0) break;
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
static PH_(pvalue) PH_(remove)(struct H_(heap) *const heap) {
	const PH_(pvalue) result = PH_(get_value)(heap->a.data);
	assert(heap);
	if(heap->a.size > 1) {
		PH_(sift_down)(heap);
	} else {
		assert(heap->a.size == 1);
		heap->a.size = 0;
	}
	return result;
}

/** Initialises `heap` to be idle. @order \Theta(1) @allow */
static void H_(heap)(struct H_(heap) *const heap)
	{ assert(heap), H_(heap_node_array)(&heap->a); }

/** Returns `heap` to the idle state where it takes no dynamic memory.
 @order \Theta(1) @allow */
static void H_(heap_)(struct H_(heap) *const heap)
	{ assert(heap), H_(heap_node_array_)(&heap->a); }

/** @return The size of `heap`. @order \Theta(1) @allow */
static size_t H_(heap_size)(const struct H_(heap) *const heap)
	{ return assert(heap), heap->a.size; }

/** Sets `heap` to be empty. That is, the size of `heap` will be zero, but if
 it was previously in an active non-idle state, it continues to be.
 @param[heap] If null, does nothing. @order \Theta(1) @allow */
static void H_(heap_clear)(struct H_(heap) *const heap)
	{ assert(heap), heap->a.size = 0; }

/** Copies `node` into `heap`. @return Success. @throws[ERANGE, realloc]
 @order \O(log `heap.size`) @allow */
static int H_(heap_add)(struct H_(heap) *const heap,
	struct H_(heap_node) node) {
	assert(heap);
	/* `new` adds an uninitialized element to the back; <fn:<PH>sift_up>
	 replaces the back element with a copy of `node`. */
	return H_(heap_node_array_new)(&heap->a) && (PH_(sift_up)(heap, &node), 1);
}

/** @return Lowest in `heap` according to `HEAP_COMPARE` or null if the heap is
 empty. This pointer is valid only until one makes structural changes to the
 heap. @order \O(1) @allow */
static struct H_(heap_node) *H_(heap_peek)(const struct H_(heap) *const heap)
	{ return assert(heap), heap->a.size ? heap->a.data : 0; }

/** This returns the <typedef:<PH>pvalue> of the <tag:<H>heap_node> returned by
 <fn:<H>heap_peek>, for convenience with some applications. If `HEAP_VALUE`,
 this is a child of <fn:<H>heap_peek>, otherwise it is a boolean `int`.
 @return Lowest <typedef:<PH>value> in `heap` element according to
 `HEAP_COMPARE`; if the heap is empty, null or zero. @order \O(1) @allow */
static PH_(pvalue) H_(heap_peek_value)(struct H_(heap) *const heap)
	{ return PH_(value_or_null)(H_(heap_peek)(heap)); }

/** Remove the lowest element according to `HEAP_COMPARE`.
 @param[heap] If null, returns false. @return The <typedef:<PH>pvalue> of the
 element that was removed; if the heap is empty, null or zero.
 @order \O(log `size`) @allow */
static PH_(pvalue) H_(heap_pop)(struct H_(heap) *const heap)
	{ return assert(heap), heap->a.size ? PH_(remove)(heap) : 0; }

/** Ensures that `heap` is `reserve` capacity beyond the elements already in
 the heap, but doesn't add to the size.
 @param[reserve] If zero and idle, returns null
 @return The end of the `heap`, where are `reserve` elements. Writing on this
 memory space is safe, but one will have to increase the size manually, (see
 <fn:<H>heap_buffer>.)
 @throws[ERANGE, realloc] @order Amortised \O(`reserve`). @allow */
static struct H_(heap_node) *H_(heap_reserve)(struct H_(heap) *const heap,
	const size_t reserve) {
	assert(heap);
	if(!reserve) return heap->a.data ? heap->a.data + heap->a.size : 0;
	if(heap->a.size > (size_t)-1 - reserve) { errno = ERANGE; return 0; }
	if(!H_(heap_node_array_reserve)(&heap->a, heap->a.size + reserve)) return 0;
	return heap->a.data + heap->a.size;
}

/** Adds and heapifies `add` elements to `heap`. Uses <Doberkat, 1984, Floyd>
 to sift-down all the internal nodes of heap, including any previous elements.
 As such, this function is most efficient on a heap of zero size, and becomes
 more inefficient as the existing heap grows. For heaps that are already in
 use, it may be better to add each element individually, resulting in a
 run-time of \O(`new elements` \cdot log `heap.size`).
 @param[add] If zero, returns true.
 @return Success. @throws[ERANGE, realloc] If <fn:<H>heap_reserve> has been
 successful in reserving at least `add` elements, one is guaranteed success.
 Practically, it really doesn't make any sense to call this without calling
 <fn:<H>heap_reserve> and setting the values, because then one would be
 inserting un-initialised values on the heap.
 @order \O(`heap.size` + `add`) @allow */
static int H_(heap_buffer)(struct H_(heap) *const heap, const size_t add) {
	assert(heap);
	if(!add) return 1;
	if(heap->a.size > (size_t)-1 - add) { errno = ERANGE; return 0; }
	if(!H_(heap_node_array_reserve)(&heap->a, heap->a.size + add)) return 0;
	heap->a.size += add;
	PH_(heapify)(heap);
	return 1;
}

/** Contains all iteration parameters. */
struct PH_(iterator);
struct PH_(iterator) { const struct H_(heap_node_array) *a; size_t i; };

/** Loads `heap` into `it`. @implements begin */
static void PH_(begin)(struct PH_(iterator) *const it,
	const struct H_(heap) *const heap)
	{ assert(it && heap), it->a = &heap->a, it->i = 0; }

/** Advances `it`. @implements next */
static const struct H_(heap_node) *PH_(next)(struct PH_(iterator) *const it) {
	assert(it && it->a);
	return it->i < it->a->size ? it->a->data + it->i++ : 0;
}

#if defined(ITERATE) || defined(ITERATE_BOX) || defined(ITERATE_TYPE) \
	|| defined(ITERATE_BEGIN) || defined(ITERATE_NEXT)
#error Unexpected ITERATE*.
#endif

#define ITERATE struct PH_(iterator)
#define ITERATE_BOX struct H_(heap)
#define ITERATE_TYPE struct H_(heap_node)
#define ITERATE_BEGIN PH_(begin)
#define ITERATE_NEXT PH_(next)

static void PH_(unused_base_coda)(void);
static void PH_(unused_base)(void) {
	struct H_(heap_node) h;
	memset(&h, 0, sizeof h);
	H_(heap)(0); H_(heap_)(0); H_(heap_size)(0); H_(heap_clear)(0);
	H_(heap_peek_value)(0); H_(heap_pop)(0); H_(heap_reserve)(0, 0);
	H_(heap_buffer)(0, 0); PH_(unused_base_coda)();
}
static void PH_(unused_base_coda)(void) { PH_(unused_base)(); }


#elif defined(HEAP_TO_STRING) /* base code --><!-- to string trait */


#ifdef HEAP_TO_STRING_NAME /* <!-- name */
#define Z_(thing) CAT(H_(heap), CAT(HEAP_TO_STRING_NAME, thing))
#else /* name --><!-- !name */
#define Z_(thing) CAT(H_(heap), thing)
#endif /* !name --> */
#define TO_STRING HEAP_TO_STRING
#include "to_string.h" /** \include */

#if !defined(HEAP_TEST_BASE) && defined(HEAP_TEST) /* <!-- test */
#define HEAP_TEST_BASE /* Only one instance of base tests. */
#include "../test/test_heap.h" /** \include */
#endif /* test --> */

#undef Z_
#undef HEAP_TO_STRING
#ifdef HEAP_TO_STRING_NAME
#undef HEAP_TO_STRING_NAME
#endif


#endif /* traits --> */


#ifdef HEAP_EXPECT_TRAIT /* <!-- trait */
#undef HEAP_EXPECT_TRAIT
#else /* trait --><!-- !trait */
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
#undef ITERATE
#undef ITERATE_BOX
#undef ITERATE_TYPE
#undef ITERATE_BEGIN
#undef ITERATE_NEXT	
#endif /* !trait --> */

#undef HEAP_TO_STRING_TRAIT
#undef HEAP_TRAITS

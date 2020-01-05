/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Parameterised Priority Queue

 ![Example of heap.](../web/heap.png)

 A <tag:<H>Heap> is a priority queue built from <tag:<H>HeapNode> as an array
 `<<H>HeapNode>Array`. It is a very simple binary heap.

 `<H>Heap` is not synchronised. Errors are returned with `errno`. The
 parameters are `#define` preprocessor macros, and are all undefined at the end
 of the file for convenience. `assert.h` is used; to stop assertions, use
 `#define NDEBUG` before inclusion.

 @param[HEAP_NAME, HEAP_TYPE]
 `<H>` that satisfies `C` naming conventions when mangled and a valid
 <typedef:<H>Type> associated therewith; `HEAP_NAME` is required. `<PH>` is
 private, whose names are prefixed in a manner to avoid collisions; any should
 be re-defined prior to use elsewhere.

 @param[HEAP_PRIORITY]
 A function satisfying <typedef:<PH>Priority>; required if and only if
 `HEAP_TYPE`.

 @param[HEAP_PRIORITY_TYPE]
 This is <typedef:<PE>Priority> and defaults to `unsigned int`.

 @param[HEAP_PRIORITY_COMPARE]
 A function satisfying <typedef:<PE>Compare>. Defaults to minimum-hash using
 less-then on `HEAP_PRIORITY_TYPE`.

 @param[HEAP_TO_STRING]
 Optional print function implementing <typedef:<PH>ToString>; makes available
 <fn:<H>SetToString>.

 @param[HEAP_TEST]
 Unit testing framework <fn:<H>HeapTest>, included in a separate header,
 <../test/HeapTest.h>. Must be defined equal to a (random) filler function,
 satisfying <typedef:<PH>Action>. Requires `HEAP_TO_STRING` and not `NDEBUG`.

 @depend Array.h
 @std C89
 @cf [Array](https://github.com/neil-edelman/Array)
 @cf [List](https://github.com/neil-edelman/List)
 @cf [Orcish](https://github.com/neil-edelman/Orcish)
 @cf [Pool](https://github.com/neil-edelman/Pool)
 @cf [Set](https://github.com/neil-edelman/Set) */

/* Check defines. */
#ifndef HEAP_NAME
#error Generic HEAP_NAME undefined.
#endif
#if (defined(HEAP_TYPE) && !defined(HEAP_PRIORITY)) \
	|| (!defined(HEAP_TYPE) && defined(HEAP_PRIORITY))
#error HEAP_TYPE has to match HEAP_PRIORITY.
#endif
#if defined(HEAP_TEST) && !defined(HEAP_TO_STRING)
#error HEAP_TEST requires HEAP_TO_STRING.
#endif
#if defined(H_) || defined(PH_)
#error H_ and PH_ cannot be defined.
#endif
#ifndef HEAP_PRIORITY_TYPE
#define HEAP_PRIORITY_TYPE unsigned
#endif

/* <Kernighan and Ritchie, 1988, p. 231>. */
#ifdef CAT
#undef CAT
#endif
#ifdef CAT_
#undef CAT_
#endif
#ifdef PCAT
#undef PCAT
#endif
#ifdef PCAT_
#undef PCAT_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define H_(thing) CAT(HEAP_NAME, thing)
#define PH_(thing) PCAT(heap, PCAT(HEAP_NAME, thing))

/** Valid type used for caching priority. */
typedef HEAP_PRIORITY_TYPE PH_(Priority);

#ifdef HEAP_TYPE /* <!-- type */
/** A valid tag type set by `HEAP_TYPE`. If `HEAP_TYPE` is omitted, then this
 defaults to <typedef:<PH>Priority>. */
typedef HEAP_TYPE PH_(Type);
#else /* type --><!-- !type */
typedef PH_(Priority) PH_(Type);
#endif

#ifdef HEAP_TEST /* <!-- test */
/** Operates by side-effects. Used for `HEAP_TEST`. */
typedef void (*PH_(Action))(PH_(Type) *);
#endif /* test --> */

/** Partial-order function that returns a positive result if `a` comes after
 `b`. */
typedef int (*PH_(Compare))(const PH_(Priority), const PH_(Priority));
#ifndef HEAP_PRIORITY_COMPARE /* <!-- !cmp */
/** Default `a` comes after `b` which makes a min-hash. */
static int PH_(default_compare)(const PH_(Priority) a, const PH_(Priority) b)
	{ return a > b; }
#define HEAP_PRIORITY_COMPARE &PH_(default_compare)
#endif /* !cmp --> */
/* Check that `HEAP_PRIORITY_COMPARE` is a function implementing
 <typedef:<PH>Compare>, if defined. */
static const PH_(Compare) PH_(compare) = (HEAP_PRIORITY_COMPARE);

/** Stores a <typedef:<PH>Priority> and, if `HASH_TYPE`, a pointer to the
 value <typedef:<PH>Type>. */
struct H_(HeapNode);
struct H_(HeapNode) {
	PH_(Priority) priority;
#ifdef HEAP_TYPE
	PH_(Type) *value;
#endif
};

/* This relies on `Array.h` which must be in the same directory. */
#define ARRAY_NAME H_(HeapNode)
#define ARRAY_TYPE struct H_(HeapNode)
#define ARRAY_CHILD
#include "Array.h"

/** Stores the heap as an implicit binary tree in an array. To initialise it to
 an idle state, see <fn:<H>Heap>, `HEAP_IDLE`, `{0}` (`C99`), or being `static`.

 ![States.](../web/states.png) */
struct H_(Heap);
struct H_(Heap) { struct H_(HeapNodeArray) a; };
#ifndef HEAP_IDLE /* <!-- !zero */
#define HEAP_IDLE { { 0, 0, 0 } }
#endif /* !zero --> */



/* parent(i)    = floor((i-1) / 2)
 left_child(i)  = 2*i + 1
 right_child(i) = 2*i + 2 */

/* Heap functions... */

/** Copies `src` to `dest`. */
static void PH_(copy)(const struct H_(HeapNode) *const src,
	struct H_(HeapNode) *const dest) {
	dest->priority = src->priority;
#ifdef HEAP_TYPE /* <!-- type */
	dest->value = src->value;
#endif /* type --> */
}

/** Inserts item `x` at position `k` in `heap`. */
static void PH_(sift_down)(struct H_(Heap) *const heap, const size_t ik) {
	struct H_(HeapNode) *const k = heap->a.data + ik, *c, *left, *right;
	size_t isize = heap->a.size, ihalf = heap->a.size >> 1, ileft, iright;
	while(ik < ihalf) {
		ileft = (ik << 1) + 1;
		left = heap->a.data + ileft;
		iright = ileft + 1;
		right = heap->a.data + iright;
		if(iright < isize
			&& PH_(compare)(left->priority, right->priority) > 0) {
			
		}
	}
}

/** Heap invariants on `heap`. */
static void PH_(heapify)(struct H_(Heap) *const heap) {
	size_t i;
	for(i = (heap->a.size >> 1) - 1; i >= 0; i--) PH_(sift_down)(heap, i);
}

/** Sift-up `node` in `heap`.
 @order \O(log `size`) */
static void PH_(sift_up)(struct H_(Heap) *const heap, const size_t inode) {
	struct H_(HeapNode) temp, *elem, *elem_up;
	size_t ielem, ielem_up;
	int start_permutation = 0;
	assert(heap && inode < heap->a.size);
	for(ielem = inode; ielem; ielem = ielem_up) {
		elem = heap->a.data + ielem;
		elem_up = heap->a.data + (ielem_up = (ielem - 1) >> 1);
		if(PH_(compare)(elem->priority, elem_up->priority) >= 0) break;
		if(!start_permutation) PH_(copy)(elem, &temp), start_permutation = 1;
		PH_(copy)(elem_up, elem);
	}
	if(start_permutation) PH_(copy)(&temp, elem);
}

#ifndef HEAP_CHILD /* <!-- !sub-type */



#ifdef HEAP_TO_STRING /* <!-- string */
/** Responsible for turning <typedef:<PH>Type> into a maximum 11-`char`
 string. */
typedef void (*PH_(ToString))(const PH_(Type) *, char (*)[12]);
/* Check that `HEAP_TO_STRING` is a function implementing
 <typedef:<PH>ToString>. Used for `HEAP_TO_STRING`. */
static const PH_(ToString) PH_(to_string) = (HEAP_TO_STRING);
#endif /* string --> */



/* Un-define all macros. */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#else /* !sub-type --><!-- sub-type */
#undef HEAP_CHILD
static void PT_(unused_coda)(void);
/** This is a subtype of another, more specialised type. `CAT`, _etc_, have to
 have the same meanings; they will be replaced with these, and `T` and `H`
 cannot be used. */
static void PH_(unused_set)(void) {
	/* <fn:<PH>up...> are integral; we want
	 to be notified when these are not called. Other stuff, not really. */
	PH_(unused_coda)();
}
static void PH_(unused_coda)(void) { PH_(unused_set)(); }
#endif /* sub-type --> */
#undef HEAP_NAME
#undef H_
#undef PH_
#undef HEAP_PRIORITY_TYPE
#undef HEAP_PRIORITY_COMPARE
#ifdef HEAP_TYPE
#undef HEAP_TYPE
#endif
#ifdef HEAP_PRIORITY
#undef HEAP_PRIORITY
#endif
#ifdef HEAP_TO_STRING
#undef HEAP_TO_STRING
#endif
#ifdef HEAP_TEST
#undef HEAP_TEST
#endif

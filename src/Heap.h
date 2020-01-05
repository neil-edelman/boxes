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
 This is <typedef:<PH>Priority> and defaults to `unsigned int`.

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
#define PT_(thing) PCAT(array, PCAT(CAT(HEAP_NAME, HeapNode), thing))

/** Stores the heap as an implicit binary tree in an array. To initialise it to
 an idle state, see <fn:<H>Heap>, `HEAP_IDLE`, `{0}` (`C99`), or being `static`.

 ![States.](../web/states.png) */
struct H_(Heap);
struct H_(Heap) { struct H_(HeapNodeArray) a; };
#ifndef HEAP_IDLE /* <!-- !zero */
#define HEAP_IDLE { { 0, 0, 0 } }
#endif /* !zero --> */

/** Copies `src` to `dest`. We could have made the priority a function that
 calls the object, then we would only need one, but by caching the priority, we
 expect a decent speed increase. */
static void PH_(copy)(const struct H_(HeapNode) *const src,
	struct H_(HeapNode) *const dest) {
	dest->priority = src->priority;
#ifdef HEAP_TYPE /* <!-- type */
	dest->value = src->value;
#endif /* type --> */
}

/** Restore order when inserting with index `inode` in `heap`.
 @order \O(log `size`) */
static void PH_(sift_up)(struct H_(Heap) *const heap,
	struct H_(HeapNode) *const node) {
	struct H_(HeapNode) temp, *elem, *elem_up;
	size_t ielem, ielem_up;
	int start_permutation = 0;
	assert(heap && heap->a.size);
	for(ielem = node - heap->a.data; ielem; ielem = ielem_up) {
		elem = heap->a.data + ielem;
		elem_up = heap->a.data + (ielem_up = (ielem - 1) >> 1);
		if(PH_(compare)(elem->priority, elem_up->priority) >= 0) break;
		if(!start_permutation) PH_(copy)(elem, &temp), start_permutation = 1;
		PH_(copy)(elem_up, elem);
	}
	if(start_permutation) PH_(copy)(&temp, elem);
}

/** Restore order when extracting node replaced with index `node` in `heap`. */
static void PH_(sift_down)(struct H_(Heap) *const heap, const size_t inode) {
	struct H_(HeapNode) temp, *parent, *child;
	size_t isize = heap->a.size, ihalf = heap->a.size >> 1,
		iparent = inode, ichild;
	int start_permutation = 0;
	while(iparent < ihalf) {
		child = heap->a.data + (ichild = (iparent << 1) + 1); /* Select left. */
		if(ichild + 1 < isize && PH_(compare)(child->priority,
			(child + 1)->priority) > 0) child++; /* Maybe switch right. */
		parent = heap->a.data + iparent; /* fixme: node->iparent->parent. */
		if(PH_(compare)(parent->priority, child->priority) <= 0) break;
		if(!start_permutation) PH_(copy)(parent, &temp), start_permutation = 1;
		PH_(copy)(child, parent);
		iparent = ichild;
	}
	if(start_permutation) PH_(copy)(&temp, parent);
}

/** Create a `heap` from an array. */
static void PH_(heapify)(struct H_(Heap) *const heap) {
	size_t i;
	for(i = (heap->a.size >> 1) - 1; (PH_(sift_down)(heap, i), i); i--);
}

/** Peek at the top node in `heap`.
 @order \Theta(1) */
static struct H_(HeapNode) *PH_(peek)(const struct H_(Heap) *const heap) {
	assert(heap);
	return heap->a.size ? heap->a.data : 0;
}

#ifndef HEAP_CHILD /* <!-- !sub-type */

/** Returns `heap` to the idle state where it takes no dynamic memory.
 @param[a] If null, does nothing.
 @order \Theta(1)
 @allow */
static void H_(Heap_)(struct H_(Heap) *const heap) {
	if(heap) { free(heap->a.data); PT_(array)(&heap->a); }
}

/** Initialises `heap` to be idle.
 @order \Theta(1)
 @allow */
static void H_(Heap)(struct H_(Heap) *const heap) {
	if(heap) { PT_(array)(&heap->a); }
}

/** Copies `node` into `heap`.
 @param[heap] If null, returns false.
 @return Success.
 @throws[realloc]
 @order \O(log `size`) */
static int H_(HeapAdd)(struct H_(Heap) *const heap, struct H_(HeapNode) node) {
	struct H_(HeapNode) *in_heap;
	if(heap && (in_heap = PT_(new)(&heap->a, 0))) {
		PH_(copy)(&node, in_heap);
		PH_(sift_up)(heap, in_heap);
		return 1;
	}
	return 0;
}

/** Gets the lowest element according to `` `node` into `heap`.
 @param[heap] If null, returns false.
 @return Success.
 @throws[realloc]
 @order \O(log `size`) */
static int H_(HeapAdd)(struct H_(Heap) *const heap, struct H_(HeapNode) node) {
	struct H_(HeapNode) *in_heap;
	if(heap && (in_heap = PT_(new)(&heap->a, 0))) {
		PH_(copy)(&node, in_heap);
		PH_(sift_up)(heap, in_heap);
		return 1;
	}
	return 0;
}

#ifdef HEAP_TO_STRING /* <!-- string */
/** Responsible for turning <typedef:<PH>Type> into a maximum 11-`char`
 string. */
typedef void (*PH_(ToString))(const PH_(Type) *, char (*)[12]);
/* Check that `HEAP_TO_STRING` is a function implementing
 <typedef:<PH>ToString>. Used for `HEAP_TO_STRING`. */
static const PH_(ToString) PH_(to_string) = (HEAP_TO_STRING);
#endif /* string --> */


static void PH_(unused_coda)(void);
static void PH_(unused_set)(void) {
	struct H_(HeapNode) h;
	H_(Heap_)(0);
	H_(Heap)(0);
	H_(HeapAdd)(0, h);
	PH_(unused_coda)();
}
static void PH_(unused_coda)(void) { PH_(unused_set)(); }

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
#undef PT_
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

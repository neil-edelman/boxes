/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Parameterised Priority Queue

 ![Example of heap.](../web/heap.png)

 A <tag:<H>Heap> is a priority queue built from <tag:<H>HeapNode>. It is a
 binary heap, proposed by <Williams, 1964, Heapsort, p. 347> and using
 terminology of <Knuth, 1973, Sorting>. Internally, it is an
 `<<H>HeapNode>Array` with implicit heap properties; as such, one needs to have
 `Array.h` file in the same directory.

 `<H>Heap` is not synchronised. Errors are returned with `errno`. The
 parameters are `#define` preprocessor macros, and are all undefined at the end
 of the file for convenience. `assert.h` is used; to stop assertions, use
 `#define NDEBUG` before inclusion.

 @param[HEAP_NAME, HEAP_TYPE]
 `<H>` that satisfies `C` naming conventions when mangled and an assignable
 type <typedef:<PH>Priority> associated therewith. `HEAP_NAME` is required but
 `HEAP_TYPE` defaults to `unsigned int` if not specified. `<PH>` is private,
 whose names are prefixed in a manner to avoid collisions.

 @param[HEAP_COMPARE]
 A function satisfying <typedef:<PH>Compare>. Defaults to minimum-hash using
 less-then on `HEAP_TYPE`; as such, if `HEAP_TYPE` is changed, this may be
 required.

 @param[HEAP_VALUE]
 This is <typedef:<PH>Value>, the optional payload that is stored as a
 reference in <tag:<H>HeapNode>.

 @param[HEAP_TO_STRING]
 Optional print function implementing <typedef:<PH>ToString>; makes available
 <fn:<H>HeapToString>.

 @param[HEAP_TEST]
 Unit testing framework <fn:<H>HeapTest>, included in a separate header,
 <../test/HeapTest.h>. Must be defined equal to a (random) filler function,
 satisfying <typedef:<PH>BiAction>. Requires `HEAP_TO_STRING` and not `NDEBUG`.

 @depend [Array.h](../../Array/)
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
#if defined(HEAP_TEST) && !defined(HEAP_TO_STRING)
#error HEAP_TEST requires HEAP_TO_STRING.
#endif
#if defined(H_) || defined(PH_)
#error H_ and PH_ cannot be defined.
#endif
#ifndef HEAP_TYPE
#define HEAP_TYPE unsigned
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

/** Valid type used for caching priority set by `HEAP_TYPE` and used in
 <tag:<H>HeapNode>. Defaults to `unsigned int`. */
typedef HEAP_TYPE PH_(Priority);

/** Returns a positive result if `a` comes after `b`, inducing a strict
 pre-order of `a` with respect to `b`; this is compatible, but less strict then
 the comparators from `bsearch` and `qsort`; it only needs to divide entries
 into two instead of three categories. The default `HEAP_COMPARE` is `a > b`,
 which makes a minimum-hash. */
typedef int (*PH_(Compare))(const PH_(Priority) a, const PH_(Priority) b);
#ifndef HEAP_COMPARE /* <!-- !cmp */
/** Pre-order with `a` and `b`. */
static int PH_(default_compare)(const PH_(Priority) a, const PH_(Priority) b) {
	return a > b;
}
#define HEAP_COMPARE &PH_(default_compare)
#endif /* !cmp --> */
/* Check that `HEAP_COMPARE` is a function implementing
 <typedef:<PH>Compare>, if defined. */
static const PH_(Compare) PH_(compare) = (HEAP_COMPARE);

#ifdef HEAP_VALUE /* <!-- value */
/** If `HEAP_VALUE` is set, a valid tag type, used in <tag:<H>HeapNode>. */
typedef HEAP_VALUE PH_(Value);
/** If `HEAP_VALUE` is set, a pointer to the <typedef:<PH>Value>; may be null
 if one has put null values in or if the node is null, otherwise a boolean
 `int` that is true (one) if the value was there and false (zero) if not. */
typedef PH_(Value) *PH_(PValue);
#else /* value --><!-- !value */
typedef int PH_(PValue);
#endif /* value --> */

/** Stores a <typedef:<PH>Priority> as `priority`, which can be set by
 `HASH_TYPE`. If `HASH_VALUE` is set, a <typedef:<PH>PValue> called `value`. */
struct H_(HeapNode);
struct H_(HeapNode) {
	PH_(Priority) priority;
#ifdef HEAP_VALUE /* <!-- value */
	PH_(PValue) value;
#endif /* value --> */
};

/* This relies on `Array.h` which must be in the same directory. */
#define ARRAY_NAME H_(HeapNode)
#define ARRAY_TYPE struct H_(HeapNode)
#define ARRAY_CHILD
#include "Array.h"
/* This is fairy ugly and knows too much about `Array`. */
#define PT_(thing) PCAT(array, PCAT(CAT(HEAP_NAME, HeapNode), thing))

/** Stores the heap as an implicit binary tree in an array. To initialise it to
 an idle state, see <fn:<H>Heap>, `HEAP_IDLE`, `{0}` (`C99`), or being `static`.

 ![States.](../web/states.png) */
struct H_(Heap);
struct H_(Heap) { struct H_(HeapNodeArray) a; };
#ifndef HEAP_IDLE /* <!-- !zero */
#define HEAP_IDLE { { 0, 0, 0, 0 } }
#endif /* !zero --> */

/** Extracts the <typedef:<PH>PValue> of `node`, which must not be null. */
static PH_(PValue) PH_(value)(const struct H_(HeapNode) *const node) {
#ifdef HEAP_VALUE /* <-- value */
	return node->value;
#else /* value --><!-- !value */
	(void)(node);
	return 1;
#endif /* !value --> */
}

/** Extracts the <typedef:<PH>PValue> of `node`, which could be null. */
static PH_(PValue) PH_(value_or_null)(const struct H_(HeapNode) *const node) {
	return node ? PH_(value)(node) : 0;
}

/** Copies `src` to `dest`. */
static void PH_(copy)(const struct H_(HeapNode) *const src,
	struct H_(HeapNode) *const dest) {
	dest->priority = src->priority;
#ifdef HEAP_VALUE /* <!-- value */
	dest->value = src->value;
#endif /* value --> */
}

/** Find the spot in `heap` where `node` goes and put it there.
 @param[heap] At least one entry; the last entry will be replaced by `node`.
 @order \O(log `size`) */
static void PH_(sift_up)(struct H_(Heap) *const heap,
	struct H_(HeapNode) *const node) {
	struct H_(HeapNode) *const n0 = heap->a.data;
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
 element.
 @param[heap] At least one entry. The head is popped, and the size will be one
 less. */
static void PH_(sift_down)(struct H_(Heap) *const heap) {
	const size_t size = (assert(heap && heap->a.size), --heap->a.size),
		half = size >> 1;
	size_t i = 0, c;
	struct H_(HeapNode) *const n0 = heap->a.data,
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
static void PH_(sift_down_i)(struct H_(Heap) *const heap, size_t i) {
	const size_t size = (assert(heap && i < heap->a.size), heap->a.size),
		half = size >> 1;
	size_t c;
	struct H_(HeapNode) *const n0 = heap->a.data, *child, temp;
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

/** Add a `node` to `heap`.
 @order \O(log `size`) */
static int PH_(add)(struct H_(Heap) *const heap,
	struct H_(HeapNode) *const node) {
	/* `new` adds an uninitialised element to the back; <fn:<PH>sift_up>
	 replaces the back element with `node`. */
	return PT_(new)(&heap->a, 0) ? (PH_(sift_up)(heap, node), 1) : 0;
}

/** Removes from `heap`. Must have a non-zero size. */
static PH_(PValue) PH_(remove)(struct H_(Heap) *const heap) {
	const PH_(PValue) result = PH_(value)(heap->a.data);
	assert(heap);
	if(heap->a.size > 1) {
		PH_(sift_down)(heap);
	} else {
		assert(heap->a.size == 1);
		heap->a.size = 0;
	}
	return result;
}

/** Create a `heap` from an array.
 @order \O(`size`) */
static void PH_(heapify)(struct H_(Heap) *const heap) {
	size_t i;
	assert(heap);
	if(heap->a.size)
		for(i = (heap->a.size >> 1) - 1; (PH_(sift_down_i)(heap, i), i); i--);
}

/** Peek at the top node in `heap`.
 @order \Theta(1) */
static struct H_(HeapNode) *PH_(peek)(const struct H_(Heap) *const heap) {
	assert(heap);
	return heap->a.size ? heap->a.data : 0;
}

#ifndef HEAP_CHILD /* <!-- !sub-type */

/** Returns `heap` to the idle state where it takes no dynamic memory.
 @param[heap] If null, does nothing.
 @order \Theta(1)
 @allow */
static void H_(Heap_)(struct H_(Heap) *const heap) {
	if(heap) free(heap->a.data), PT_(array)(&heap->a);
}

/** Initialises `heap` to be idle.
 @param[heap] If null, does nothing.
 @order \Theta(1)
 @allow */
static void H_(Heap)(struct H_(Heap) *const heap) {
	if(heap) PT_(array)(&heap->a);
}

/** @param[heap] If null, returns zero;
 @return The size of `heap`.
 @order \Theta(1)
 @allow */
static size_t H_(HeapSize)(const struct H_(Heap) *const heap) {
	return heap ? heap->a.size : 0;
}

/** Sets `heap` to be empty. That is, the size of `heap` will be zero, but if
 it was previously in an active non-idle state, it continues to be.
 @param[heap] If null, does nothing.
 @order \Theta(1)
 @allow */
static void H_(HeapClear)(struct H_(Heap) *const heap) {
	if(heap) heap->a.size = 0;
}

/** Copies `node` into `heap`.
 @param[heap] If null, returns false.
 @return Success.
 @throws[realloc]
 @order \O(log `size`)
 @allow */
static int H_(HeapAdd)(struct H_(Heap) *const heap, struct H_(HeapNode) node) {
	return heap ? PH_(add)(heap, &node) : 0;
}

/** @param[heap] If null, returns null.
 @return Lowest in `heap` according to `HEAP_COMPARE` or null if the heap is
 empty. This pointer is valid only until one makes structural changes to the
 heap.
 @order \O(1)
 @allow */
static struct H_(HeapNode) *H_(HeapPeek)(struct H_(Heap) *const heap) {
	return heap ? PH_(peek)(heap) : 0;
}

/** This returns the value of the <tag:<H>HeapNode>, a child of
 <fn:<H>HeapPeek>, for convenience with some applications.
 @param[heap] If null, returns null.
 @return Lowest <typedef:<PH>Value> in `heap` element according to
 `HEAP_COMPARE`; if the heap is empty, null or zero.
 @order \O(1)
 @allow */
static PH_(PValue) H_(HeapPeekValue)(struct H_(Heap) *const heap) {
	return heap ? PH_(value_or_null)(PH_(peek)(heap)) : 0;
}

/** Remove the lowest element according to `HEAP_COMPARE`.
 @param[heap] If null, returns false.
 @return The <typedef:<PH>PValue> of the element that was removed; if the heap
 is empty, null or zero.
 @order \O(log `size`)
 @allow */
static PH_(PValue) H_(HeapPop)(struct H_(Heap) *const heap) {
	return heap && heap->a.size ? PH_(remove)(heap) : 0;
}

/** Ensures that `heap` is `reserve` capacity beyond the elements already in
 the heap, but doesn't add to the size.
 @param[heap] If null, returns false.
 @param[reserve] If zero, returns true.
 @return The end of the `heap`, where are `reserve` elements, or
 null and `errno` will be set. Writing on this memory space is safe, but one
 will have to increase the size manually, (see <fn:<H>HeapBuffer>.)
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 error and doesn't follow [IEEE Std 1003.1-2001
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc]
 @order Amortised \O(`reserve`).
 @allow */
static struct H_(HeapNode) *H_(HeapReserve)(struct H_(Heap) *const heap,
	const size_t reserve) {
	if(!heap) return 0;
	if(!reserve) return heap->a.data ? heap->a.data + heap->a.size : 0;
	if(heap->a.size > (size_t)-1 - reserve) { errno = ERANGE; return 0; }
	if(!PT_(reserve)(&heap->a, heap->a.size + reserve, 0)) return 0;
	return heap->a.data + heap->a.size;
}

/** Adds `add` elements to `heap`. Uses <Doberkat, 1984, Floyd> to sift-down
 all the internal nodes of heap. As such, this function is most efficient on a
 heap of zero size, and becomes more inefficient as the existing heap grows.
 For heaps that are already in use, it may be better to add each element
 individually, resulting in a run-time of \O(`new elements` \cdot log `size`).
 @param[heap] If null, returns null.
 @param[add] If zero, returns null.
 @return Success.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 error and doesn't follow [IEEE Std 1003.1-2001
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html). If
 <fn:<H>HeapReserve> has been successful in reserving at least `add` elements,
 one is guaranteed success. Practically, it really doesn't make any sense to
 call this without calling <fn:<H>HeapReserve> because then one would be
 inserting un-initialised values on the heap.
 @throws[realloc]
 @order \O(`new size`)
 @allow */
static int H_(HeapBuffer)(struct H_(Heap) *const heap, const size_t add) {
	if(!heap || !add) return 0;
	if(heap->a.size > (size_t)-1 - add) { errno = ERANGE; return 0; }
	if(!PT_(reserve)(&heap->a, heap->a.size + add, 0)) return 0;
	heap->a.size += add;
	PH_(heapify)(heap);
	return 1;
}

#ifdef HEAP_TO_STRING /* <!-- string */

/** Responsible for turning <tag:<H>HeapNode> into a maximum 11-`char` string.
 Used for `HEAP_TO_STRING`. */
typedef void (*PH_(ToString))(const struct H_(HeapNode) *, char (*)[12]);
/* Check that `HEAP_TO_STRING` is a function implementing
 <typedef:<PH>ToString>. */
static const PH_(ToString) PH_(to_string) = (HEAP_TO_STRING);

/** Can print 4 things at once before it overwrites. One must a
 `HEAP_TO_STRING` to a function implementing <typedef:<PH>ToString> to get
 this functionality.
 @return Prints `heap` in a static buffer.
 @order \Theta(1); it has a 255 character limit; every element takes some of it.
 @allow */
static const char *H_(HeapToString)(const struct H_(Heap) *const heap) {
	static char buffers[4][256];
	static size_t buffer_i;
	char *const buffer = buffers[buffer_i++], *b = buffer;
	const size_t buffers_no = sizeof buffers / sizeof *buffers,
	buffer_size = sizeof *buffers / sizeof **buffers;
	const char start = '(', comma = ',', space = ' ', end = ')',
	*const ellipsis_end = ",…)", *const null = "null",
	*const idle = "idle";
	const size_t ellipsis_end_len = strlen(ellipsis_end),
	null_len = strlen(null), idle_len = strlen(idle);
	size_t i;
	struct H_(HeapNode) *e, *e_end;
	int is_first = 1;
	assert(!(buffers_no & (buffers_no - 1)) && ellipsis_end_len >= 1
		   && buffer_size >= 1 + 11 + ellipsis_end_len + 1
		   && buffer_size >= null_len + 1
		   && buffer_size >= idle_len + 1);
	/* Advance the buffer for next time. */
	buffer_i &= buffers_no - 1;
	if(!heap) { memcpy(b, null, null_len), b += null_len; goto terminate; }
	if(!heap->a.data) { memcpy(b, idle, idle_len), b += idle_len;
		goto terminate; }
	*b++ = start;
	for(e = heap->a.data, e_end = heap->a.data + heap->a.size; ; ) {
		if(!is_first) *b++ = comma, *b++ = space;
		else is_first = 0;
		PH_(to_string)(e, (char (*)[12])b);
		for(i = 0; *b != '\0' && i < 12; b++, i++);
		if(++e >= e_end) break;
		if((size_t)(b - buffer) > buffer_size - 2 - 11 - ellipsis_end_len - 1)
			goto ellipsis;
	}
	*b++ = end;
	goto terminate;
ellipsis:
	memcpy(b, ellipsis_end, ellipsis_end_len), b += ellipsis_end_len;
terminate:
	*b++ = '\0';
	assert(b <= buffer + buffer_size);
	return buffer;
}

#endif /* string --> */

#ifdef HEAP_TEST /* <!-- test: need this file. */
#include "../test/TestHeap.h" /** \include */
#endif /* test --> */

static void PH_(unused_coda)(void);
/** This silences unused function warnings. */
static void PH_(unused_set)(void) {
	struct H_(HeapNode) h;
	memset(&h, 0, sizeof h);
	H_(Heap_)(0);
	H_(Heap)(0);
	H_(HeapSize)(0);
	H_(HeapClear)(0);
	H_(HeapAdd)(0, h);
	H_(HeapPeek)(0);
	H_(HeapPeekValue)(0);
	H_(HeapPop)(0);
	H_(HeapReserve)(0, 0);
	H_(HeapBuffer)(0, 0);
	H_(HeapToString)(0);
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
	PH_(value)(0);
	PH_(copy)(0, 0);
	PH_(sift_up)(0, 0);
	PH_(sift_down)(0, 0);
	PH_(sift_down_i)(0, 0);
	PH_(remove)(0);
	PH_(heapify)(0);
	PH_(peek)(0);
	PH_(unused_coda)();
}
static void PH_(unused_coda)(void) { PH_(unused_set)(); }
#endif /* sub-type --> */
#undef HEAP_NAME
#undef H_
#undef PH_
#undef PT_
#undef HEAP_TYPE
#undef HEAP_COMPARE
#ifdef HEAP_VALUE
#undef HEAP_VALUE
#endif
#ifdef HEAP_TO_STRING
#undef HEAP_TO_STRING
#endif
#ifdef HEAP_TEST
#undef HEAP_TEST
#endif

/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Header <../src/heap.h> depends on <../src/array.h>; examples
 <../test/test_heap.c>.

 @subtitle Priority-queue

 ![Example of heap.](../doc/heap/heap.png)

 A <tag:<T>heap> is a binary heap, proposed by
 <Williams, 1964, Heapsort, p. 347> using terminology of
 <Knuth, 1973, Sorting>. It is an array implementation of a priority queue. It
 is not stable.

 @param[HEAP_NAME, HEAP_TYPE]
 `<S>` that satisfies `C` naming conventions when mangled. `HEAP_NAME` is
 required; `HEAP_TYPE` defaults to `unsigned int`.

 @param[HEAP_DISORDERED]
 A function satisfying <typedef:<pT>disordered_fn>. Defaults to minimum-hash.
 Required if `HEAP_TYPE` is changed to an incomparable type. For example, a
 maximum heap, `(a, b) -> a < b`.

 @param[HEAP_VALUE]
 Optional value <typedef:<PS>value>, that, on `HEAP_VALUE`, is stored in
 <tag:<S>heapnode>.

 @param[HEAP_TO_STRING]
 To string trait contained in <src/to_string.h>. Require
 `<name>[<trait>]to_string` be declared as <typedef:<PS>to_string_fn>.

 @param[BOX_EXPECT_TRAIT, HEAP_TRAIT]
 Named traits are obtained by including `heap.h` multiple times with
 `BOX_EXPECT_TRAIT` and then subsequently including the name in `HEAP_TRAIT`.

 @param[HEAP_DECLARE_ONLY]
 For headers in different compilation units.

 @depend [array](https://github.com/neil-edelman/array)
 @std C89 */

#ifndef HEAP_NAME
#error Name undefined.
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

/** Valid assignable type used for priority in <typedef:<PH>node>. Defaults to
 `unsigned int` if not set by `HEAP_TYPE`. */
typedef HEAP_TYPE pT_(priority);
/* fixme: Are you sure you need this, now? `<pT>priority`? */
typedef const HEAP_TYPE pT_(priority_c); /* This is assuming a lot? */

#	ifdef HEAP_VALUE
typedef HEAP_VALUE pT_(value);
typedef const HEAP_VALUE pT_(value_c); /* Assume! */
/** If `HEAP_VALUE` is set, this becomes <typedef:<PH>node>. */
struct T_(heapnode) { pT_(priority) priority; pT_(value) value; };
/** If `HEAP_VALUE` is set, (priority, value) set by <tag:<H>heapnode>,
 otherwise it's a (priority) set directly by <typedef:<PH>priority>. */
typedef struct T_(heapnode) pT_(node);
typedef const struct T_(heapnode) pT_(node_c);
#	else
typedef pT_(priority) pT_(value);
typedef pT_(priority) pT_(node);
typedef pT_(priority_c) pT_(node_c); /* fixme? */
#	endif

/* Temporary. Avoid recursion. This must match <box.h>. */
#	undef BOX_MINOR
#	undef BOX_MAJOR
#	define pT2_(n) BOX_CAT(private, BOX_CAT(HEAP_NAME, BOX_CAT(heap, n)))
#	define ARRAY_NAME pT2_(node)
#	define ARRAY_TYPE pT2_(node)
/* This relies on <array.h> which must be in the same directory. */
#	include "array.h"
#	undef pT2_
#	define BOX_MINOR HEAP_NAME
#	define BOX_MAJOR heap

/** Stores the heap as an array—implicit binary tree in an array called `a`. To
 initialize it to an idle state, see <fn:<H>heap>, `{0}` (`C99`), or being
 `static`.

 ![States.](../doc/heap/states.png) */
struct t_(heap) { struct pT_(node_array) as_array; };

struct T_(cursor) { struct pT_(node_array_cursor) _; };

#	ifndef HEAP_DECLARE_ONLY /* <!-- body */

//#		ifdef BOX_BODY /* <!-- real body: get the array functions, if separate. */
// wtf? why is this even here?
//#define ARRAY_NAME pT_(node)
//#define ARRAY_TYPE pT_(node)
//#define ARRAY_DEFINE_ONLY
//#include "array.h"
//#		endif /* real body --> */

/** Inducing a strict weak order by returning a positive result if `a` is
 out-of-order with respect to `b`. It only needs to divide entries into
 two instead of three categories—is compatible, but less strict then the
 comparators from `bsearch` and `qsort`. For example, `return a > b` or
 `return strcmp(a, b)` would give a minimum-hash. */
typedef int (*pT_(less_fn))(pT_(priority_c) a, pT_(priority_c) b);

static struct T_(cursor) T_(begin)(struct t_(heap) *const h)
	{ struct T_(cursor) it; it._ = pT_(node_array_begin)(&h->as_array); return it; }
static int T_(cursor_exists)(struct T_(cursor) *const cur)
	{ return pT_(node_array_cursor_exists)(&cur->_); }
static pT_(node) *T_(cursor_look)(struct T_(cursor) *const cur)
	{ return pT_(node_array_cursor_look)(&cur->_); }
static void T_(cursor_next)(struct T_(cursor) *const cur)
	{ pT_(node_array_cursor_next)(&cur->_); }

/** Extracts the <typedef:<PH>priority> of `node`, which must not be null. */
static pT_(priority) pT_(get_priority)(const pT_(node) *const node) {
#		ifdef HEAP_VALUE
	return node->priority;
#		else
	return *node;
#		endif
}
/** Extracts the <typedef:<PH>value> of `node`, which must not be null. */
static pT_(value) pT_(get_value)(const pT_(node) *const node) {
#		ifdef HEAP_VALUE
	return node->value;
#		else
	return *node;
#		endif
}
/** Find the spot in `heap` where `node` goes and put it there.
 @param[heap] At least one entry; the last entry will be replaced by `node`.
 @order \O(log `size`) */
static void pT_(sift_up)(struct t_(heap) *const heap, pT_(node) *const node) {
	pT_(node) *const n0 = heap->as_array.data;
	pT_(priority) p = pT_(get_priority)(node);
	size_t i = heap->as_array.size - 1;
	if(i) {
		size_t i_up;
		do { /* Note: don't change the `<=`; it's a queue. */
			i_up = (i - 1) >> 1;
			/* Make sure that `<HEAP_NAME>_less` is defined. */
			if(t_(less)(pT_(get_priority)(n0 + i_up), p) <= 0) break;
			n0[i] = n0[i_up];
		} while((i = i_up));
	}
	n0[i] = *node;
}
/** Pop the head of `heap` and restore the heap by sifting down the last
 element. @param[heap] At least one entry. The head is popped, and the size
 will be one less. */
static void pT_(sift_down)(struct t_(heap) *const heap) {
	const size_t size = (assert(heap && heap->as_array.size),
		--heap->as_array.size), half = size >> 1;
	size_t i = 0, c;
	pT_(node) *const n0 = heap->as_array.data,
		*const down = n0 + size /* Put it at the top. */, *child;
	const pT_(priority) down_p = pT_(get_priority)(down);
	while(i < half) {
		c = (i << 1) + 1;
		if(c + 1 < size && t_(less)(pT_(get_priority)(n0 + c),
			pT_(get_priority)(n0 + c + 1)) > 0) c++;
		child = n0 + c;
		if(t_(less)(down_p, pT_(get_priority)(child)) <= 0) break;
		n0[i] = *child;
		i = c;
	}
	n0[i] = *down;
}
/** Restore the `heap` by permuting the elements so `i` is in the proper place.
 This reads from the an arbitrary leaf-node into a temporary value, so is
 slightly more complex than <fn:<PH>sift_down>, but the same thing.
 @param[heap] At least `i + 1` entries. */
static void pT_(sift_down_i)(struct t_(heap) *const heap, size_t i) {
	const size_t size = (assert(heap && i < heap->as_array.size),
		heap->as_array.size), half = size >> 1;
	size_t c;
	/* Uninitialized variable warning suppression. */
	pT_(node) *const n0 = heap->as_array.data, *child, temp = *(&temp);
	int temp_valid = 0;
	while(i < half) {
		c = (i << 1) + 1;
		if(c + 1 < size && t_(less)(pT_(get_priority)(n0 + c),
			pT_(get_priority)(n0 + c + 1)) > 0) c++;
		child = n0 + c;
		if(temp_valid) {
			if(t_(less)(pT_(get_priority)(&temp),
				pT_(get_priority)(child)) <= 0) break;
		} else {
			/* Only happens on the first compare when `i` is in it's original
			 position. */
			if(t_(less)(pT_(get_priority)(n0 + i),
				pT_(get_priority)(child)) <= 0) break;
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
static pT_(node) pT_(remove)(struct t_(heap) *const heap) {
	const pT_(node) result = *heap->as_array.data;
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
	{ struct t_(heap) heap; heap.as_array = pT_(node_array)(); return heap; }

/** Returns `heap` to the idle state where it takes no dynamic memory.
 @order \Theta(1) @allow */
static void t_(heap_)(struct t_(heap) *const heap)
	{ if(heap) pT_(node_array_)(&heap->as_array); }

/** Sets `heap` to be empty. That is, the size of `heap` will be zero, but if
 it was previously in an active non-idle state, it continues to be.
 @param[heap] If null, does nothing. @order \Theta(1) @allow */
static void T_(clear)(struct t_(heap) *const heap)
	{ assert(heap), pT_(node_array_clear)(&heap->as_array); }

/** @return If the `heap` is not null, returns it's size. @allow */
static size_t T_(size)(const struct t_(heap) *const heap)
	{ return heap ? heap->as_array.size : 0; }

/** Copies `node` into `heap`.
 @return Success. @throws[ERANGE, realloc] @order \O(log `heap.size`) @allow */
static int T_(add)(struct t_(heap) *const heap, pT_(node) node) {
	assert(heap);
	return pT_(node_array_new)(&heap->as_array) && (pT_(sift_up)(heap, &node), 1);
}

/** @return The value of the lowest element in `heap` or null when the heap is
 empty. @order \O(1) @allow */
static pT_(node) *T_(peek)(const struct t_(heap) *const heap)
	{ return assert(heap), heap->as_array.size ? heap->as_array.data : 0; }

/** Only defined when <fn:<H>heap_size> returns true. Removes the lowest
 element. @return The value of the lowest element in `heap`.
 @order \O(\log `size`) @allow */
static pT_(value) T_(pop)(struct t_(heap) *const heap) {
	pT_(node) n;
	return assert(heap && heap->as_array.size),
		(n = pT_(remove)(heap), pT_(get_value)(&n));
}

/** The capacity of `heap` will be increased to at least `n` elements beyond
 the size. Invalidates pointers in `heap`. All the elements
 `heap.as_array.size` <= `index` < `heap.as_array.capacity` can be used to
 construct new elements without immediately making them part of the heap, then
 <fn:<H>heap_append>.
 @return The start of the buffered space. If `a` is idle and `buffer` is zero,
 a null pointer is returned, otherwise null indicates an error.
 @throws[realloc, ERANGE] @allow */
static pT_(node) *T_(buffer)(struct t_(heap) *const heap,
	const size_t n) { return pT_(node_array_buffer)(&heap->as_array, n); }

/** Adds and heapifies `n` elements to `heap`. Uses <Floyd, 1964, Treesort> to
 sift-down all the internal nodes of heap. The heap elements must exist, see
 <fn:<H>heap_buffer>.
 @param[n] If zero, returns true without heapifying.
 @return Success. @order \O(`heap.size` + `n`) <Doberkat, 1984, Floyd> @allow */
static void T_(append)(struct t_(heap) *const heap, const size_t n) {
	pT_(node) *more;
	/* In practice, pushing uninitialized elements onto the heap does not make
	 sense, so we assert that the elements exist first. */
	assert(heap && n <= heap->as_array.capacity - heap->as_array.size);
	more = pT_(node_array_append)(&heap->as_array, n), assert(more);
	if(n) pT_(heapify)(heap);
}

/** Shallow-copies and heapifies `master` into `heap`.
 @param[master] If null, does nothing. @return Success.
 @order \O(`heap.size` + `copy.size`) @throws[ERANGE, realloc] @allow */
static int T_(affix)(struct t_(heap) *restrict const heap,
	const struct t_(heap) *restrict const master) {
	pT_(node) *n;
	assert(heap);
	if(!master || !master->as_array.size) return 1;
	assert(master->as_array.data);
	if(!(n = pT_(node_array_buffer)(&heap->as_array, master->as_array.size)))
		return 0;
	memcpy(n, master->as_array.data, sizeof *n * master->as_array.size);
	n = pT_(node_array_append)(&heap->as_array, master->as_array.size),
		assert(n);
	pT_(heapify)(heap);
	return 1;
}

static void pT_(unused_base_coda)(void);
static void pT_(unused_base)(void) {
	pT_(node) unused; memset(&unused, 0, sizeof unused);
	T_(begin)(0); T_(cursor_exists)(0); T_(cursor_look)(0); T_(cursor_next)(0);
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
#		define TO_STRING_LEFT '['
#		define TO_STRING_RIGHT ']'
#		include "to_string.h" /** \include */
#		undef HEAP_TO_STRING
#		ifndef HEAP_TRAIT
#			define HEAP_HAS_TO_STRING /* Warning about tests. */
#		endif
#	endif

#endif /* Produce code. */
#ifdef HEAP_TRAIT
#	undef HEAP_TRAIT
#	undef BOX_TRAIT
#endif


#ifdef HEAP_TO_STRING /* <!-- to string trait */
/** Thunk `n` -> `a`. */
static void PHT_(to_string)(const pT_(node) *n, char (*const a)[12]) {
#	ifdef HEAP_VALUE
	HT_(to_string)(n->priority, n->value, a);
#	else
	HT_(to_string)(n, a);
#	endif
}
#	define TO_STRING_LEFT '['
#	define TO_STRING_RIGHT ']'
#	include "to_string.h" /** \include */
#	undef HEAP_TO_STRING
#	ifndef HEAP_TRAIT
#		define HEAP_HAS_TO_STRING
#	endif
#endif /* to string trait --> */
#undef PHT_
#undef HT_


#if defined(HEAP_TEST) && !defined(HEAP_TRAIT) /* <!-- test base */
#	include "../test/test_heap.h"
#endif /* test base --> */


#ifdef BOX_EXPECT_TRAIT
#	undef BOX_EXPECT_TRAIT
#else
#	undef BOX_MINOR
#	undef BOX_MAJOR
#	undef HEAP_NAME
#	undef HEAP_TYPE

#	undef BOX_CONTENT
#	undef BOX_
#	undef BOX_MAJOR
#	ifdef HEAP_VALUE
#		undef HEAP_VALUE
#	endif
#	ifdef HEAP_HAS_TO_STRING
#		undef HEAP_HAS_TO_STRING
#	endif
#	ifdef HEAP_TEST
#		undef HEAP_TEST
#	endif
#	ifdef BOX_BODY
#		undef BOX_BODY
#	endif
#	ifdef HEAP_DECLARE_ONLY
#		undef HEAP_DECLARE_ONLY
#	endif
#endif
#ifdef HEAP_TRAIT
#	undef HEAP_TRAIT
#endif
#define BOX_END
#include "box.h"

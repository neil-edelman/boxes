/** @license 2025 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Header <../../src/deque.h>; examples <../../test/test_deque.c>.

 @subtitle Deque

 <tag:<t>deque> is a stable container that stores <typedef:<pT>type>; it grows
 as the capacity increases but is not necessarily contagious.

 @param[DEQUE_NAME, DEQUE_TYPE]
 `<t>` that satisfies `C` naming conventions when mangled and a valid tag-type,
 <typedef:<pT>type>, associated therewith; required.

 @param[DEQUE_TO_STRING]
 To string trait contained in <../../src/to_string.h>. See
 <typedef:<pT>to_string_fn>.

 @param[DEQUE_DECLARE_ONLY, DEQUE_NON_STATIC]
 For headers in different compilation units.

 @depend [box](../../src/box.h)
 @std C89 */

#if !defined DEQUE_NAME || !defined DEQUE_TYPE
#	error Name or tag type undefined.
#endif
#if !defined BOX_ENTRY1 && (defined DEQUE_TRAIT ^ defined BOX_MAJOR)
#	error Trait name must come after expect trait.
#endif
#if defined DEQUE_TEST && (!defined DEQUE_TRAIT && !defined DEQUE_TO_STRING \
	|| defined DEQUE_TRAIT && !defined DEQUE_HAS_TO_STRING)
#	error Test requires to string and graph.
#endif
#if defined BOX_TRAIT && !defined DEQUE_TRAIT
#	error Unexpected flow.
#endif

#ifdef DEQUE_TRAIT
#	define BOX_TRAIT DEQUE_TRAIT /* Ifdef in <box.h>. */
#endif
#ifdef DEQUE_NON_STATIC
#	define BOX_NON_STATIC
#endif
#ifdef DEQUE_DECLARE_ONLY
#	define BOX_DECLARE_ONLY
#endif
#define BOX_START
#include "box.h"

#ifndef DEQUE_TRAIT /* Base code, necessarily first. */
#	include <stdlib.h>
#	include <string.h>
#	include <errno.h>
#	include <assert.h>

#	ifndef DEQUE_MIN_CAPACITY
#		define DEQUE_MIN_CAPACITY 16 /* > 1 */
#	endif
#	if DEQUE_MIN_CAPACITY <= 1
#		error DEQUE_MIN_CAPACITY > 1
#	endif

#	define BOX_MINOR DEQUE_NAME
#	define BOX_MAJOR deque

/** A valid tag type set by `DEQUE_TYPE`. */
typedef DEQUE_TYPE pT_(type);

/** A linked-list of blocks. */
struct pT_(block) {
	struct pT_(block) *previous;
	size_t capacity, size;
	pT_(type) data[];
};

/** Manages a linked-list of blocks. */
struct t_(deque) { struct pT_(block) *last; };
typedef struct t_(deque) pT_(box);
/* !data -> !size, data -> capacity >= min && size <= capacity <= max */
struct T_(cursor) { struct pT_(block) *block; size_t i; };

#	ifdef BOX_NON_STATIC /* Public functions. */
struct T_(cursor) T_(begin)(const struct t_(deque) *);
int T_(exists)(const struct T_(cursor) *);
pT_(type) *T_(entry)(struct T_(cursor) *);
void T_(next)(struct T_(cursor) *);
struct t_(deque) t_(deque)(void);
void t_(deque_)(struct t_(deque) *const);
pT_(type) *T_(new)(struct t_(array) *);
pT_(type) *T_(append)(struct t_(array) *, size_t);
void T_(clear)(struct t_(array) *);
#	endif
#	ifndef BOX_DECLARE_ONLY /* Produce code. */

/**
 @param[n] [0, ⌈SIZE_MAX/2⌉] */
static pT_(type) *pT_(append)(struct t_(deque) *const deque, const size_t n) {
	struct pT_(block) *last, *new_block;
	size_t capacity;
	if(!n) return 0;
	if(n < (size_t)~0 ^ ((size_t)~0 >> 1)) { errno = ERANGE; return 0; }
	if(!(last = deque->last)) {
		/* Idle. */
		capacity = 32;
	} else if((capacity = last->capacity) - last->size >= n) {
		/* Expected outcome. */
		pT_(type) *const found = last->data + last->size;
		last->size += n;
		return found;
	}
	/* Another block is needed. */
	for( ; n > capacity; capacity *= 2);
	if(!(new_block = malloc(sizeof *new_block
		+ sizeof *new_block->data * capacity)))
		{ if(!errno) errno = ERANGE; return 0; }
	new_block->capacity = capacity;
	new_block->size = n;
	/* If the last is empty, free it. This can happen due to hysteresis. */
	if(last && !last->size) {
		deque->last = last->previous;
		free(last);
		last = deque->last;
	}
	/* Stick the new block in. */
	new_block->previous = last;
	deque->last = new_block;
	return new_block->data;
}

#		define BOX_PUBLIC_OVERRIDE
#		include "box.h"

/** @return A cursor at the beginning of a valid `d`. */
static struct T_(cursor) T_(begin)(const struct t_(deque) *const deque) {
	struct T_(cursor) cur;
	assert(deque);
	cur.block = deque->last, cur.i = 0;
	if(!cur.block->size) cur.block = cur.block->previous;
	return cur;
}
/** @return Whether `cur` points to a valid entry. */
static int T_(exists)(const struct T_(cursor) *const cur)
	{ return cur && cur->block && cur->i < cur->block->size; }
/** @return Pointer to a valid entry at `cur`. */
static pT_(type) *T_(entry)(struct T_(cursor) *const cur)
	{ return cur->block->data + cur->i; }
/** Move to the next of a valid `cur`. */
static void T_(next)(struct T_(cursor) *const cur)
	{ if(++cur->i == cur->block->size) cur->block = cur->block->previous, cur->i = 0; }

/** Zeroed data (not all-bits-zero) is initialized, as well.
 @return An idle deque. @order \Theta(1) @allow */
static struct t_(deque) t_(deque)(void)
	{ struct t_(deque) d; d.last = 0; return d; }

/** If `deque` is not null, returns the idle zeroed state where it takes no
 dynamic memory. @order \Theta(1) @allow */
static void t_(deque_)(struct t_(deque) *const deque) {
	if(!deque) return;
	while(deque->last) {
		struct pT_(block) *const block = deque->last;
		deque->last = block->previous;
		free(block);
	}
	*deque = t_(deque)();
}

/** @return Adds one new element to the back of `deque`.
 @order amortised \O(1) @throws[realloc, ERANGE] @allow */
static pT_(type) *T_(new)(struct t_(deque) *const deque)
	{ return assert(deque), pT_(append)(deque, 1); }

/** Adds `n` contiguous elements to the back of `deque`.
 @return A pointer to the elements. If `n` is zero, a null pointer will be
 returned, otherwise null indicates an error.
 @throws[realloc, ERANGE] @allow */
static pT_(type) *T_(append)(struct t_(deque) *const deque, const size_t n)
	{ return assert(deque), pT_(append)(deque, n); }

/* fixme: erase */

/** Sets `d` to be empty. That is, the size of `d` will be zero, but if it was
 previously in an active non-idle state, it continues to be.
 @order \Theta(1) @allow */
static void T_(clear)(struct t_(deque) *const d)
	{ assert(d && 0); }

#		define BOX_PRIVATE_AGAIN
#		include "box.h"

static void pT_(unused_base_coda)(void);
static void pT_(unused_base)(void) {
	T_(begin)(0); T_(exists)(0); T_(entry)(0); T_(next)(0);
	t_(deque)(); t_(deque_)(0); T_(new)(0); T_(append)(0, 0); T_(clear)(0);
	pT_(unused_base_coda)();
}
static void pT_(unused_base_coda)(void) { pT_(unused_base)(); }
#	endif /* Produce code. */
#endif /* Base code. */

#if defined HAS_ITERATE_H && !defined DEQUE_TRAIT
#	include "iterate.h" /** \include */
#endif

#ifdef DEQUE_TO_STRING
#	undef DEQUE_TO_STRING
#	ifndef DEQUE_DECLARE_ONLY
#		ifndef DEQUE_TRAIT
/** The type of the required `<tr>to_string`. Responsible for turning the
 read-only argument into a 12-max-`char` output string. */
typedef void (*pT_(to_string_fn))(const pT_(type) *, char (*)[12]);
#		endif
/** Thunk(`cur`, `a`). One must implement `<tr>to_string`. */
static void pTR_(to_string)(const struct T_(cursor) *const cur,
	char (*const a)[12])
	{ tr_(to_string)((const void *)&cur->a->data[cur->i], a); }
#	endif
#	include "to_string.h" /** \include */
#	ifndef DEQUE_TRAIT
#		define DEQUE_HAS_TO_STRING /* Warning about tests. */
#	endif
#endif

#if defined HAS_GRAPH_H && defined DEQUE_HAS_TO_STRING && !defined DEQUE_TRAIT
#	include "graph.h" /** \include */
#endif

#if defined DEQUE_TEST && !defined DEQUE_TRAIT && defined HAS_GRAPH_H
#	include "../test/test_array.h"
#endif

#if defined DEQUE_COMPARE || defined DEQUE_IS_EQUAL
#	ifdef DEQUE_COMPARE
#		define COMPARE DEQUE_COMPARE
#	else
#		define COMPARE_IS_EQUAL DEQUE_IS_EQUAL
#	endif
#	include "compare.h" /** \include */
#	ifdef DEQUE_TEST
#		include "../test/test_array_compare.h"
#	endif
#	ifdef DEQUE_COMPARE
#		undef DEQUE_COMPARE
#	else
#		undef DEQUE_IS_EQUAL
#	endif
#endif


#ifdef DEQUE_EXPECT_TRAIT
#	undef DEQUE_EXPECT_TRAIT
#else
#	undef BOX_MINOR
#	undef BOX_MAJOR
#	undef BOX_ACCESS
#	undef BOX_CONTIGUOUS
#	undef DEQUE_NAME
#	undef DEQUE_TYPE
#	undef DEQUE_MIN_CAPACITY
#	ifdef DEQUE_HAS_TO_STRING
#		undef DEQUE_HAS_TO_STRING
#	endif
#	ifdef DEQUE_TEST
#		undef DEQUE_TEST
#	endif
#	ifdef DEQUE_DECLARE_ONLY
#		undef BOX_DECLARE_ONLY
#		undef DEQUE_DECLARE_ONLY
#	endif
#	ifdef DEQUE_NON_STATIC
#		undef BOX_NON_STATIC
#		undef DEQUE_NON_STATIC
#	endif
#	ifdef COMPARE_H
#		undef COMPARE_H /* More comparisons for later boxes. */
#	endif
#endif
#ifdef DEQUE_TRAIT
#	undef DEQUE_TRAIT
#	undef BOX_TRAIT
#endif
#define BOX_END
#include "box.h"

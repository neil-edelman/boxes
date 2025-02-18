/** @license 2025 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Header <../../src/deque.h>; examples <../../test/test_deque.c>.

 @subtitle Deque

 <tag:<t>deque> is a stable container that stores <typedef:<pT>type>; it grows
 as the capacity increases but is not necessarily contagious. The default
 behaviour is a stack that grows downwards.

 @param[DEQUE_NAME, DEQUE_TYPE]
 `<t>` that satisfies `C` naming conventions when mangled and a valid tag-type,
 <typedef:<pT>type>, associated therewith; required.

 @param[DEQUE_FRONT]
 This adds double-linking in the blocks so the deque can iterated forwards.
 Changes the order of the default iteration.

 @param[DEQUE_PUSH_FRONT]
 The default deque is only a stack that grows down. This replaces the index
 size with a range in all the blocks. Implies `DEQUE_FRONT`.

 @param[DEQUE_TO_STRING]
 To string trait contained in <../../src/to_string.h>. See
 <typedef:<pT>to_string_fn>.

 @param[DEQUE_DECLARE_ONLY, DEQUE_NON_STATIC]
 For headers in different compilation units.

 @depend [box](../../src/box.h)
 @std C89, but recommend C99 flexible array members instead of "struct hack". */

#if !defined DEQUE_NAME || !defined DEQUE_TYPE
#	error Name or tag type undefined.
#endif
#if !defined BOX_ENTRY1 && (defined DEQUE_TRAIT ^ defined BOX_MAJOR)
#	error Trait name must come after expect trait.
#endif
#if defined DEQUE_PUSH_FRONT && defined DEQUE_FRONT
#	error Remove DEQUE_FRONT when DEQUE_PUSH_FRONT is defined.
#endif
#ifdef DEQUE_PUSH_FRONT
#	error Not implemented.
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
#	ifdef DEQUE_FRONT
	struct pT_(block) *next;
#	endif
	size_t capacity, size;
#	if defined __STDC__ && defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
	pT_(type) data[];
#	else
	pT_(type) data[1];
#	endif
};

/** Manages a linked-list of blocks. Only the front can have a block-size of
 zero. */
struct t_(deque) {
	struct pT_(block) *back;
#	ifdef DEQUE_FRONT
	struct pT_(block) *front;
#	endif
};
typedef struct t_(deque) pT_(box);
struct T_(cursor) { struct pT_(block) *block; size_t i; };

#	ifdef BOX_NON_STATIC /* Public functions. */
struct T_(cursor) T_(end)(const struct t_(deque) *);
int T_(exists)(const struct T_(cursor) *);
pT_(type) *T_(entry)(struct T_(cursor) *);
void T_(previous)(struct T_(cursor) *);
#		ifdef DEQUE_FRONT
struct T_(cursor) T_(begin)(const struct t_(deque) *);
void T_(next)(struct T_(cursor) *);
#		endif
struct t_(deque) t_(deque)(void);
void t_(deque_)(struct t_(deque) *const);
pT_(type) *T_(new_back)(struct t_(deque) *);
pT_(type) *T_(append_back)(struct t_(deque) *, size_t);
void T_(clear)(struct t_(deque) *);
#	endif
#	ifndef BOX_DECLARE_ONLY /* Produce code. */

/** Appends `n` contiguous items to `deque`.
 @param[n] [0, ⌈SIZE_MAX/2⌉]. Returns null if `n = 0`.
 @throws[malloc, ERANGE] */
static pT_(type) *pT_(append_back)(struct t_(deque) *const deque, const size_t n) {
	struct pT_(block) *back, *new_block;
	size_t capacity;
	if(!n) return 0;
	if(n > ~((size_t)~0 >> 1)) { errno = ERANGE; return 0; }
	if(!(back = deque->back)) {
		capacity = 16; /* Idle. Default. Set this to half the desired value. */
	} else if((capacity = back->capacity) - back->size >= n) {
		/* Expected outcome. */
		pT_(type) *const found = back->data + back->size;
		back->size += n;
		return found;
	}
	/* Another block is needed. */
	if(capacity < ~((size_t)~0 >> 1)) capacity *= 2;
	for( ; n > capacity; capacity *= 2);
	if(!(new_block = malloc(
#	if defined __STDC__ && defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
		sizeof *new_block + sizeof *new_block->data * capacity
#	else
		/* Old "struct hack" from before flexible array members. Not entirely
		 conforming, but widely accepted. */
		offsetof(struct pT_(block), data[capacity])
#	endif
		))) { if(!errno) errno = ERANGE; return 0; }
	new_block->capacity = capacity;
	new_block->size = n;
	/* If the last is empty, free it. This can happen due to hysteresis. */
	if(back && !back->size) {
		deque->back = back->previous;
#	ifdef DEQUE_FRONT
		if(back->previous) back->previous->next = 0;
		else deque->front = 0;
#	endif
		free(back);
		back = deque->back;
	}
	/* Stick the new block in. */
#	ifdef DEQUE_FRONT
	new_block->next = 0;
	if(deque->back) assert(!deque->back->next), deque->back->next = new_block;
	else assert(!deque->front), deque->front = new_block;
#	endif
	new_block->previous = back;
	deque->back = new_block;
	return new_block->data;
}

#		define BOX_PUBLIC_OVERRIDE
#		include "box.h"

/** @return A cursor at end beginning of a valid `d`. */
static struct T_(cursor) T_(end)(const struct t_(deque) *const deque) {
	struct T_(cursor) cur;
	assert(deque);
	cur.block = deque->back, cur.i = 0;
	/* Can have the head have 0 items, but not the rest. */
	if(cur.block && !cur.block->size) cur.block = cur.block->previous;
	cur.i = cur.block ? cur.block->size - 1 : 0;
	return cur;
}
/** @return Whether `cur` points to a valid entry. */
static int T_(exists)(const struct T_(cursor) *const cur)
	{ return cur && cur->block; }
/** @return Pointer to a valid entry at `cur`. */
static pT_(type) *T_(entry)(struct T_(cursor) *const cur)
	{ return cur->block->data + cur->i; }
/** Move to the next of a valid `cur`. */
static void T_(previous)(struct T_(cursor) *const cur) {
	if(cur->i) cur->i--;
	else if(cur->block = cur->block->previous)
		cur->i = cur->block->size - 1;
}
#		ifdef DEQUE_FRONT
/** @return A cursor at the beginning of a valid `d`. */
static struct T_(cursor) T_(begin)(const struct t_(deque) *const deque) {
	struct T_(cursor) cur;
	assert(deque);
	cur.block = deque->front, cur.i = 0;
	if(cur.block && !cur.block->size) cur.block = 0;
	return cur;
}
/** Move to the next of a valid `cur`. */
static void T_(next)(struct T_(cursor) *const cur) {
	cur->i++;
	if(cur->i < cur->block->size) return;
	cur->block = cur->block->next;
	cur->i = 0;
	if(!cur->block || cur->block->size) return;
	cur->block = 0;
}
#		endif

/** Zeroed data (not all-bits-zero) is initialized, as well.
 @return An idle deque. @order \Theta(1) @allow */
static struct t_(deque) t_(deque)(void) {
	struct t_(deque) d;
	d.back = 0;
#		ifdef DEQUE_FRONT
	d.front = 0;
#		endif
	return d;
}

/** If `deque` is not null, returns the idle zeroed state where it takes no
 dynamic memory. @order \Theta(1) @allow */
static void t_(deque_)(struct t_(deque) *const deque) {
	if(!deque) return;
	while(deque->back) {
		struct pT_(block) *const block = deque->back;
		deque->back = block->previous;
		free(block);
	}
	*deque = t_(deque)();
}

/** @return Adds one new element to the back of `deque`.
 @order amortised \O(1) @throws[realloc, ERANGE] @allow */
static pT_(type) *T_(new_back)(struct t_(deque) *const deque)
	{ return assert(deque), pT_(append_back)(deque, 1); }

/** Adds `n` contiguous elements to the back of `deque`.
 @return A pointer to the elements. If `n` is zero, a null pointer will be
 returned, otherwise null indicates an error.
 @throws[realloc, ERANGE] @allow */
static pT_(type) *T_(append_back)(struct t_(deque) *const deque, const size_t n)
	{ return assert(deque), pT_(append_back)(deque, n); }

/* fixme: erase */

/** Sets `deque` to be empty. That is, the size will be zero, but if it was
 previously in an active non-idle state, it continues to be.
 @order \Theta(1) @allow */
static void T_(clear)(struct t_(deque) *const deque) {
	struct pT_(block) *back;
	assert(deque);
	if(!(back = deque->back)) return;
	back->size = 0;
	while(back->previous) {
		struct pT_(block) *fold = back->previous->previous;
		free(back->previous);
		back->previous = fold;
	}
#		ifdef DEQUE_FRONT
	deque->front = back;
	back->next = 0;
#		endif
}

#		define BOX_PRIVATE_AGAIN
#		include "box.h"

static void pT_(unused_base_coda)(void);
static void pT_(unused_base)(void) {
	T_(end)(0); T_(exists)(0); T_(entry)(0); T_(previous)(0);
	t_(deque)(); t_(deque_)(0); T_(new_back)(0); T_(append_back)(0, 0);
	T_(clear)(0);
#	ifdef DEQUE_FRONT
	T_(begin)(0); T_(next)(0);
#	endif
	pT_(unused_base_coda)();
}
static void pT_(unused_base_coda)(void) { pT_(unused_base)(); }
#	endif /* Produce code. */
#endif /* Base code. */

#if defined HAS_ITERATE_H && !defined DEQUE_TRAIT
#	define ITERATE_BACK
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
	{ tr_(to_string)((const void *)&cur->block->data[cur->i], a); }
#	endif
/* A deque without the front end is a stack, and we can only go back. */
#	ifndef DEQUE_FRONT
#		define TO_STRING_BACK
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
#	include "../test/test_deque.h"
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
#	ifdef DEQUE_FRONT
#		undef DEQUE_FRONT
#	endif
#	ifdef DEQUE_PUSH_FRONT
#		undef DEQUE_PUSH_FRONT
#	endif
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

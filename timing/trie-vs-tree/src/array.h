/** @license 2016 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Stand-alone header <../src/array.h>; examples <../test/test_array.c>.

 @subtitle Contiguous dynamic array

 ![Example of array.](../doc/array/array.png)

 <tag:<A>array> is a dynamic array that stores contiguous <typedef:<PA>type>.
 Resizing may be necessary when increasing the size of the array; this incurs
 amortised cost, and any pointers to this memory may become stale.

 @param[ARRAY_NAME, ARRAY_TYPE]
 `<A>` that satisfies `C` naming conventions when mangled and a valid tag-type,
 <typedef:<PA>type>, associated therewith; required. `<PA>` is private, whose
 names are prefixed in a manner to avoid collisions.

 @param[ARRAY_COMPARE, ARRAY_IS_EQUAL]
 Compare `<CMP>` trait contained in <src/compare.h>. Requires
 `<name>[<trait>]compare` to be declared as <typedef:<PCMP>compare_fn> or
 `<name>[<trait>]is_equal` to be declared as <typedef:<PCMP>bipredicate_fn>,
 respectfully, (but not both.)

 @param[ARRAY_TO_STRING]
 To string `<STR>` trait contained in <src/to_string.h>. Requires
 `<name>[<trait>]to_string` be declared as <typedef:<PSTR>to_string_fn>.

 @param[ARRAY_EXPECT_TRAIT, ARRAY_TRAIT]
 Named traits are obtained by including `array.h` multiple times with
 `ARRAY_EXPECT_TRAIT` and then subsequently including the name in
 `ARRAY_TRAIT`.

 @param[ARRAY_DECLARE_ONLY, ARRAY_DEFINE_ONLY]
 These go together to allow exporting non-static data between compilation units
 by separating the header head from the code body. `ARRAY_DECLARE_ONLY` needs identical
 `ARRAY_NAME` and `ARRAY_TYPE`.

 @std C89 */

#if !defined(ARRAY_NAME) || !defined(ARRAY_TYPE)
#error Name or tag type undefined.
#endif
#if defined(ARRAY_TRAIT) ^ defined(BOX_TYPE)
#error ARRAY_TRAIT name must come after ARRAY_EXPECT_TRAIT.
#endif
#if defined(ARRAY_COMPARE) && defined(ARRAY_IS_EQUAL)
#error Only one can be defined at a time.
#endif
#if defined(ARRAY_TEST) && (!defined(ARRAY_TRAIT) && !defined(ARRAY_TO_STRING) \
	|| defined(ARRAY_TRAIT) && !defined(ARRAY_HAS_TO_STRING))
#error Test requires to string.
#endif
#if defined ARRAY_DECLARE_ONLY && (defined ARRAY_DEFINE_ONLY || defined ARRAY_TRAIT)
#error Can not be simultaneously defined.
#endif

#ifndef ARRAY_H /* <!-- idempotent */
#define ARRAY_H
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#if defined(ARRAY_CAT_) || defined(ARRAY_CAT) || defined(A_) || defined(PA_)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define ARRAY_CAT_(n, m) n ## _ ## m
#define ARRAY_CAT(n, m) ARRAY_CAT_(n, m)
#define A_(n) ARRAY_CAT(ARRAY_NAME, n)
#define PA_(n) ARRAY_CAT(array, A_(n))
#endif /* idempotent --> */

#if !defined(restrict) && (!defined(__STDC__) || !defined(__STDC_VERSION__) \
	|| __STDC_VERSION__ < 199901L)
#define ARRAY_RESTRICT /* Undo this at the end. */
#define restrict /* Attribute only in C99+. */
#endif


#ifndef ARRAY_TRAIT /* <!-- base code */

#ifndef ARRAY_MIN_CAPACITY /* <!-- !min; */
#define ARRAY_MIN_CAPACITY 3 /* > 1 */
#endif /* !min --> */

#ifndef ARRAY_DEFINE_ONLY /* <!-- head */

/** A valid tag type set by `ARRAY_TYPE`. */
typedef ARRAY_TYPE PA_(type);

/** Manages the array field `data` which has `size` elements. The space is
 indexed up to `capacity`, which is at least `size`.

 ![States.](../doc/array/states.png) */
struct A_(array) { PA_(type) *data; size_t size, capacity; };
/* !data -> !size, data -> capacity >= min && size <= capacity <= max */

/* `a` non-null; `i >= elements` empty; insert-delete on left like C++. */
struct PA_(iterator) { struct A_(array) *a; size_t i; };
/** May become invalid after a topological change to any items previous. */
struct A_(array_iterator);
struct A_(array_iterator) { struct PA_(iterator) _; };

#endif /* head --> */
#ifndef ARRAY_DECLARE_ONLY /* <!-- body */

/** @return Iterator at end of (non-null) valid `a`. */
static struct PA_(iterator) PA_(iterator)(struct A_(array) *const a) {
	struct PA_(iterator) it;
	assert(a), it.a = a, it.i = (size_t)~0;
	return it;
}
/** @return Iterator at element `i` of non-null `a`. */
static struct PA_(iterator) PA_(iterator_at)(struct A_(array) *a, size_t i) {
	struct PA_(iterator) it;
	assert(a), it.a = a, it.i = i < a->size ? i : (size_t)~0;
	return it;
}
/** @return Dereference the element pointed to by valid `it`. */
static PA_(type) *PA_(element)(struct PA_(iterator) *const it)
	{ return it->a->data + it->i; }
/** Next `it`. @return Valid element? */
static int PA_(next)(struct PA_(iterator) *const it) {
	assert(it && it->a);
	if(it->i >= it->a->size) it->i = (size_t)~0;
	return ++it->i < it->a->size;
}
/** Previous `it`. @return Valid element? */
static int PA_(previous)(struct PA_(iterator) *const it) {
	assert(it && it->a);
	if(it->i > it->a->size) it->i = it->a->size; /* Clip. */
	return --it->i < it->a->size;
}
/* fixme: static struct PA_(iterator)
 PA_(remove)(struct PA_(iterator) *const it) */
/** Size of `a`. @implements `size` */
static size_t PA_(size)(const struct A_(array) *a) { return a ? a->size : 0; }
/** @return Element `idx` of `a`. @implements `at` */
static PA_(type) *PA_(at)(const struct A_(array) *a, const size_t idx)
	{ return a->data + idx; }
/** Writes `size` to `a`. @implements `tell_size` */
static void PA_(tell_size)(struct A_(array) *a, const size_t size)
	{ assert(a); a->size = size; }

/** Zeroed data (not all-bits-zero) is initialized.
 @return An idle array. @order \Theta(1) @allow */
static struct A_(array) A_(array)(void)
	{ struct A_(array) a; a.data = 0, a.capacity = a.size = 0; return a; }

/** If `a` is not null, destroys and returns it to idle. @allow */
static void A_(array_)(struct A_(array) *const a)
	{ if(a) free(a->data), *a = A_(array)(); }

/** Ensures `min` capacity of `a`. Invalidates pointers in `a`. @param[min] If
 zero, does nothing. @return Success; otherwise, `errno` will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 doesn't follow POSIX. @throws[realloc] @allow */
static int A_(array_reserve)(struct A_(array) *const a, const size_t min) {
	size_t c0;
	PA_(type) *data;
	const size_t max_size = (size_t)~0 / sizeof *a->data;
	assert(a);
	if(a->data) {
		assert(a->size <= a->capacity);
		if(min <= a->capacity) return 1;
		c0 = a->capacity < ARRAY_MIN_CAPACITY
			? ARRAY_MIN_CAPACITY : a->capacity;
	} else { /* Idle. */
		assert(!a->size && !a->capacity);
		if(!min) return 1;
		c0 = ARRAY_MIN_CAPACITY;
	}
	if(min > max_size) return errno = ERANGE, 0;
	/* `c_n = a1.625^n`, approximation golden ratio `\phi ~ 1.618`. */
	while(c0 < min) { /* \O(\log min), in practice, negligible. */
		size_t c1 = c0 + (c0 >> 1) + (c0 >> 3);
		if(c0 >= c1) { c0 = max_size; break; } /* Unlikely. */
		c0 = c1;
	}
	if(!(data = realloc(a->data, sizeof *a->data * c0)))
		{ if(!errno) errno = ERANGE; return 0; }
	a->data = data, a->capacity = c0;
	return 1;
}

/** The capacity of `a` will be increased to at least `n` elements beyond the
 size. Invalidates any pointers in `a`.
 @return The start of the buffered space at the back of the array. If `a` is
 idle and `buffer` is zero, a null pointer is returned, otherwise null
 indicates an error. @throws[realloc] @allow */
static PA_(type) *A_(array_buffer)(struct A_(array) *const a, const size_t n) {
	assert(a);
	if(a->size > (size_t)~0 - n) { errno = ERANGE; return 0; }
	return A_(array_reserve)(a, a->size + n) && a->data ? a->data + a->size : 0;
}

/** Appends `n` contiguous items on the back of `a`.
 @implements `append` from `BOX_CONTIGUOUS` */
static PA_(type) *PA_(append)(struct A_(array) *const a, const size_t n) {
	PA_(type) *b;
	if(!(b = A_(array_buffer)(a, n))) return 0;
	assert(n <= a->capacity && a->size <= a->capacity - n);
	return a->size += n, b;
}

/** Adds `n` un-initialised elements at position `at` in `a`. It will
 invalidate any pointers in `a` if the buffer holds too few elements.
 @param[at] A number smaller than or equal to `a.size`; if `a.size`, this
 function behaves as <fn:<A>array_append>.
 @return A pointer to the start of the new region, where there are `n`
 elements. @throws[realloc, ERANGE] @allow */
static PA_(type) *A_(array_insert)(struct A_(array) *const a,
	const size_t n, const size_t at) {
	/* Investigate `n` is better than `element`; all the other are element. But
	 also, when would I ever use this? */
	const size_t old_size = a->size;
	PA_(type) *const b = PA_(append)(a, n);
	assert(a && at <= old_size);
	if(!b) return 0;
	memmove(a->data + at + n, a->data + at, sizeof *a->data * (old_size - at));
	return a->data + at;
}

/** @return Adds (push back) one new element of `a`. The buffer space holds at
 least one element, or it may invalidate pointers in `a`.
 @order amortised \O(1) @throws[realloc, ERANGE] @allow */
static PA_(type) *A_(array_new)(struct A_(array) *const a)
	{ return PA_(append)(a, 1); }

/** Shrinks the capacity `a` to the size, freeing unused memory. If the size is
 zero, it will be in an idle state. Invalidates pointers in `a`.
 @return Success. @throws[ERANGE, realloc] (Unlikely) `realloc` error. */
static int A_(array_shrink)(struct A_(array) *const a) {
	PA_(type) *data;
	size_t c;
	assert(a && a->capacity >= a->size);
	if(!a->data) return assert(!a->size && !a->capacity), 1;
	c = a->size && a->size > ARRAY_MIN_CAPACITY ? a->size : ARRAY_MIN_CAPACITY;
	if(!(data = realloc(a->data, sizeof *a->data * c)))
		{ if(!errno) errno = ERANGE; return 0; }
	a->data = data, a->capacity = c;
	return 1;
}

/** Removes `element` from `a`. Do not attempt to remove an element that is not
 in `a`. @order \O(`a.size`). @allow */
static void A_(array_remove)(struct A_(array) *const a,
	PA_(type) *const element) {
	const size_t n = (size_t)(element - a->data);
	assert(a && element && element >= a->data && element < a->data + a->size);
	memmove(element, element + 1, sizeof *element * (--a->size - n));
}

/** Removes `datum` from `a` and replaces it with the tail. Do not attempt to
 remove an element that is not in `a`. @order \O(1). @allow */
static void A_(array_lazy_remove)(struct A_(array) *const a,
	PA_(type) *const datum) {
	size_t n = (size_t)(datum - a->data);
	assert(a && datum && datum >= a->data && datum < a->data + a->size);
	if(--a->size != n) memcpy(datum, a->data + a->size, sizeof *datum);
}

/** Sets `a` to be empty. That is, the size of `a` will be zero, but if it was
 previously in an active non-idle state, it continues to be.
 @order \Theta(1) @allow */
static void A_(array_clear)(struct A_(array) *const a)
	{ assert(a), a->size = 0; }

/** @return The last element or null if `a` is empty. @order \Theta(1) @allow */
static PA_(type) *A_(array_peek)(const struct A_(array) *const a)
	{ return assert(a), a->size ? a->data + a->size - 1 : 0; }

/** @return Value from the the top of `a` that is removed or null if the array
 is empty. @order \Theta(1) @allow */
static PA_(type) *A_(array_pop)(struct A_(array) *const a)
	{ return assert(a), a->size ? a->data + --a->size : 0; }

/** Adds `n` elements to the back of `a`. It will invalidate pointers in `a` if
 `n` is greater than the buffer space.
 @return A pointer to the elements. If `a` is idle and `n` is zero, a null
 pointer will be returned, otherwise null indicates an error.
 @throws[realloc, ERANGE] @allow */
static PA_(type) *A_(array_append)(struct A_(array) *const a, const size_t n)
	{ return assert(a), PA_(append)(a, n); }

/** Indices [`i0`, `i1`) of `a` will be replaced with a copy of `b`.
 @param[b] Can be null, which acts as empty, but cannot overlap with `a`.
 @return Success. @throws[realloc, ERANGE] @allow */
static int A_(array_splice)(struct A_(array) *restrict const a,
	const struct A_(array) *restrict const b,
	const size_t i0, const size_t i1) {
	const size_t a_range = i1 - i0, b_range = b ? b->size : 0;
	assert(a && a != b && i0 <= i1 && i1 <= a->size);
	if(a_range < b_range) { /* The output is bigger. */
		const size_t diff = b_range - a_range;
		if(!A_(array_buffer)(a, diff)) return 0;
		memmove(a->data + i1 + diff, a->data + i1,
			(a->size - i1) * sizeof *a->data);
		a->size += diff;
	} else if(b_range < a_range) { /* The output is smaller. */
		memmove(a->data + i0 + b_range, a->data + i1,
			(a->size - i1) * sizeof *a->data);
		a->size -= a_range - b_range;
	}
	if(b) memcpy(a->data + i0, b->data, b->size * sizeof *a->data);
	return 1;
}

/* Box override information. */
#define BOX_TYPE struct A_(array)
#define BOX_CONTENT PA_(type)
#define BOX_ PA_
#define BOX_MAJOR_NAME array
#define BOX_MINOR_NAME ARRAY_NAME
#define BOX_ACCESS
#define BOX_CONTIGUOUS

#ifdef HAVE_ITERATE_H /* <!-- iterate */
#include "iterate.h" /** \include */
#endif /* iterate --> */

static void PA_(unused_base_coda)(void);
static void PA_(unused_base)(void) {
	PA_(iterator)(0); PA_(iterator_at)(0, 0); PA_(element)(0);
	PA_(next)(0); PA_(previous)(0);
	PA_(size)(0); PA_(at)(0, 0); PA_(tell_size)(0, 0);
	A_(array)(); A_(array_)(0);
	A_(array_insert)(0, 0, 0); A_(array_new)(0);
	A_(array_shrink)(0); A_(array_remove)(0, 0); A_(array_lazy_remove)(0, 0);
	A_(array_clear)(0); A_(array_peek)(0); A_(array_pop)(0);
	A_(array_append)(0, 0); A_(array_splice)(0, 0, 0, 0);
	PA_(unused_base_coda)();
}
static void PA_(unused_base_coda)(void) { PA_(unused_base)(); }

#endif /* body --> */

#endif /* base code --> */


#ifdef ARRAY_TRAIT /* <-- trait: Will be different on different includes. */
#define BOX_TRAIT_NAME ARRAY_TRAIT
#define PAT_(n) PA_(ARRAY_CAT(ARRAY_TRAIT, n))
#define AT_(n) A_(ARRAY_CAT(ARRAY_TRAIT, n))
#else /* trait --><!-- !trait */
#define PAT_(n) PA_(n)
#define AT_(n) A_(n)
#endif /* !trait --> */


#ifdef ARRAY_TO_STRING /* <!-- to string trait */
/** Thunk `e` -> `a`. */
static void PAT_(to_string)(const PA_(type) *e, char (*const a)[12])
	{ AT_(to_string)((const void *)e, a); }
#include "to_string.h" /** \include */
#undef ARRAY_TO_STRING
#ifndef ARRAY_TRAIT
#define ARRAY_HAS_TO_STRING
#endif
#endif /* to string trait --> */
#undef PAT_
#undef AT_


#if defined(ARRAY_TEST) && !defined(ARRAY_TRAIT) /* <!-- test base */
#include "../test/test_array.h"
#endif /* test base --> */


#if defined(ARRAY_COMPARE) || defined(ARRAY_IS_EQUAL) /* <!-- compare trait */
#ifdef ARRAY_COMPARE /* <!-- cmp */
#define COMPARE ARRAY_COMPARE
#else /* cmp --><!-- eq */
#define COMPARE_IS_EQUAL ARRAY_IS_EQUAL
#endif /* eq --> */
#include "compare.h" /** \include */
#ifdef ARRAY_TEST /* <!-- test: this detects and outputs compare test. */
#include "../test/test_array_compare.h"
#endif /* test --> */
#undef CMP_ /* From <compare.h>. */
#undef CMPCALL_
#ifdef ARRAY_COMPARE
#undef ARRAY_COMPARE
#else
#undef ARRAY_IS_EQUAL
#endif
#endif /* compare trait --> */


#ifdef ARRAY_EXPECT_TRAIT /* <!-- more */
#undef ARRAY_EXPECT_TRAIT
#else /* more --><!-- done */
#undef BOX_TYPE
#undef BOX_CONTENT
#undef BOX_
#undef BOX_MAJOR_NAME
#undef BOX_MINOR_NAME
#undef BOX_ACCESS
#undef BOX_CONTIGUOUS
#undef ARRAY_NAME
#undef ARRAY_TYPE
#undef ARRAY_MIN_CAPACITY
#ifdef ARRAY_HAS_TO_STRING
#undef ARRAY_HAS_TO_STRING
#endif
#ifdef ARRAY_TEST
#undef ARRAY_TEST
#endif
#ifdef ARRAY_DEFINE_ONLY
#undef ARRAY_DEFINE_ONLY
#endif
#ifdef ARRAY_DECLARE_ONLY
#undef ARRAY_DECLARE_ONLY
#endif
#endif /* done --> */
#ifdef ARRAY_TRAIT
#undef ARRAY_TRAIT
#undef BOX_TRAIT_NAME
#endif
#ifdef ARRAY_RESTRICT
#undef ARRAY_RESTRICT
#undef restrict
#endif

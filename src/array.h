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

 @param[BOX_EXPECT_TRAIT, BOX_TRAIT]
 Named traits are obtained by including `array.h` multiple times with
 `BOX_EXPECT_TRAIT` and then subsequently including the name in `BOX_TRAIT`.

 @param[BOX_DECLARE_ONLY]
 For headers in different compilation units.

 @std C89 */

#if !defined(ARRAY_NAME) || !defined(ARRAY_TYPE)
#	error Name or tag type undefined.
#endif
#if defined(ARRAY_TRAIT) ^ defined(BOX_MAJOR)
#	error ARRAY_TRAIT name must come after ARRAY_EXPECT_TRAIT.
#endif
#if defined(ARRAY_COMPARE) && defined(ARRAY_IS_EQUAL)
#	error Only one can be defined at a time.
#endif
#if defined(ARRAY_TEST) && (!defined(ARRAY_TRAIT) && !defined(ARRAY_TO_STRING) \
	|| defined(ARRAY_TRAIT) && !defined(ARRAY_HAS_TO_STRING))
#	error Test requires to string.
#endif

#ifndef BOX_H
#	define BOX_H
#	if defined(BOX_CAT_) || defined(BOX_CAT) || defined(T_) || defined(PT_)
#		error Unexpected preprocessor symbols.
#	endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#	define BOX_CAT_(n, m) n ## _ ## m
#	define BOX_CAT(n, m) BOX_CAT_(n, m)
#	define T_(n) BOX_CAT(BOX_MINOR_NAME, n)
#	define PT_(n) BOX_CAT(BOX_CAT(private, BOX_MAJOR_NAME), T_(n))
#endif
#ifdef BOX_TRAIT
#	define TU_(n) T_(BOX_CAT(BOX_TRAIT, n))
#	define PTU_(n) PT_(BOX_CAT(BOX_TRAIT, n))
#else /* Anonymous trait. */
#	define TU_(n) T_(n)
#	define PTU_(n) PT_(n)
#endif
/* I omitted the `BOX_MAJOR_NAME` for clarity, but for common files. */
#define BOXTU_(n) TU_(BOX_CAT(BOX_MAJOR_NAME, n))


#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#if !defined(restrict) && (!defined(__STDC__) || !defined(__STDC_VERSION__) \
	|| __STDC_VERSION__ < 199901L)
#	define BOX_RESTRICT /* Undo this at the end. */
#	define restrict /* Attribute only in C99+. */
#endif

#ifndef ARRAY_TRAIT /* Base code, necessarily first. */

/* Box override information stays until the box is done. */
#	define BOX_MINOR_NAME ARRAY_NAME
#	define BOX_MINOR PT_(type)
#	define BOX_MAJOR_NAME array
#	define BOX_MAJOR struct T_(array)
#	define BOX_ACCESS
#	define BOX_CONTIGUOUS

#	ifndef ARRAY_MIN_CAPACITY
#		define ARRAY_MIN_CAPACITY 3 /* > 1 */
#	endif
#	if ARRAY_MIN_CAPACITY <= 1
#		error ARRAY_MIN_CAPACITY > 1
#	endif

/** A valid tag type set by `ARRAY_TYPE`. */
typedef ARRAY_TYPE PT_(type);

/** Manages the array field `data` which has `size` elements. The space is
 indexed up to `capacity`, which is at least `size`.

 ![States.](../doc/array/states.png) */
struct T_(array) { PT_(type) *data; size_t size, capacity; };
typedef struct T_(array) PT_(box);
/* !data -> !size, data -> capacity >= min && size <= capacity <= max */

/* fixme: the iterator needs to be updated into a view. What's with all the
 secrecy? Why not only `T_(cursor)`? */

/* `a` non-null; `i >= elements` empty; insert-delete on left like C++. */
struct PT_(iterator) { struct T_(array) *a; size_t i; };
/** May become invalid after a topological change to any items previous. */
struct T_(array_iterator);
struct T_(array_iterator) { struct PT_(iterator) _; };



/* fixme: a wrapper is a terrible way to make functions accessible; something
 like ARRAY_EXPORT_CONS… #ifdef ARRAY_EXPORT_SIZE, #define static, size_t T_()… */

#	ifndef BOX_DECLARE_ONLY /* Produce code: not for headers. */

/** @return Iterator at end of (non-null) valid `a`. */
static struct PT_(iterator) PT_(iterator)(struct T_(array) *const a) {
	struct PT_(iterator) it;
	assert(a), it.a = a, it.i = (size_t)~0;
	return it;
}
/** @return Iterator at element `i` of non-null `a`. */
static struct PT_(iterator) PT_(iterator_at)(struct T_(array) *a, size_t i) {
	struct PT_(iterator) it;
	assert(a), it.a = a, it.i = i < a->size ? i : (size_t)~0;
	return it;
}
/** @return Dereference the element pointed to by valid `it`. */
static PT_(type) *PT_(element)(struct PT_(iterator) *const it)
	{ return it->a->data + it->i; }
/** Next `it`. @return Valid element? */
static int PT_(next)(struct PT_(iterator) *const it) {
	assert(it && it->a);
	if(it->i >= it->a->size) it->i = (size_t)~0;
	return ++it->i < it->a->size;
}
/** Previous `it`. @return Valid element? */
static int PT_(previous)(struct PT_(iterator) *const it) {
	assert(it && it->a);
	if(it->i > it->a->size) it->i = it->a->size; /* Clip. */
	return --it->i < it->a->size;
}
/* fixme: static struct PT_(iterator)
 PT_(remove)(struct PT_(iterator) *const it) */
/** Size of `a`. @implements `size` */
static size_t PT_(size)(const struct T_(array) *a) { return a ? a->size : 0; }
/** @return Element `idx` of `a`. @implements `at` */
static PT_(type) *PT_(at)(const struct T_(array) *a, const size_t idx)
	{ return a->data + idx; }
/** Writes `size` to `a`. @implements `tell_size` */
static void PT_(tell_size)(struct T_(array) *a, const size_t size)
	{ assert(a); a->size = size; }

/** Zeroed data (not all-bits-zero) is initialized.
 @return An idle array. @order \Theta(1) @allow */
static struct T_(array) T_(array)(void)
	{ struct T_(array) a; a.data = 0, a.capacity = a.size = 0; return a; }

/** If `a` is not null, destroys and returns it to idle. @allow */
static void T_(array_)(struct T_(array) *const a)
	{ if(a) free(a->data), *a = T_(array)(); }

/** Ensures `min` capacity of `a`. Invalidates pointers in `a`. @param[min] If
 zero, does nothing. @return Success; otherwise, `errno` will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 doesn't follow POSIX. @throws[realloc] @allow */
static int T_(array_reserve)(struct T_(array) *const a, const size_t min) {
	size_t c0;
	PT_(type) *data;
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
static PT_(type) *T_(array_buffer)(struct T_(array) *const a, const size_t n) {
	assert(a);
	if(a->size > (size_t)~0 - n) { errno = ERANGE; return 0; }
	return T_(array_reserve)(a, a->size + n) && a->data ? a->data + a->size : 0;
}

/** Appends `n` contiguous items on the back of `a`.
 @implements `append` from `BOX_CONTIGUOUS` */
static PT_(type) *PT_(append)(struct T_(array) *const a, const size_t n) {
	PT_(type) *b;
	if(!(b = T_(array_buffer)(a, n))) return 0;
	assert(n <= a->capacity && a->size <= a->capacity - n);
	return a->size += n, b;
}

/** Adds `n` un-initialised elements at position `at` in `a`. It will
 invalidate any pointers in `a` if the buffer holds too few elements.
 @param[at] A number smaller than or equal to `a.size`; if `a.size`, this
 function behaves as <fn:<A>array_append>.
 @return A pointer to the start of the new region, where there are `n`
 elements. @throws[realloc, ERANGE] @allow */
static PT_(type) *T_(array_insert)(struct T_(array) *const a,
	const size_t n, const size_t at) {
	/* Investigate `n` is better than `element`; all the other are element. But
	 also, when would I ever use this? */
	const size_t old_size = a->size;
	PT_(type) *const b = PT_(append)(a, n);
	assert(a && at <= old_size);
	if(!b) return 0;
	memmove(a->data + at + n, a->data + at, sizeof *a->data * (old_size - at));
	return a->data + at;
}

/** @return Adds (push back) one new element of `a`. The buffer space holds at
 least one element, or it may invalidate pointers in `a`.
 @order amortised \O(1) @throws[realloc, ERANGE] @allow */
static PT_(type) *T_(array_new)(struct T_(array) *const a)
	{ return PT_(append)(a, 1); }

/** Shrinks the capacity `a` to the size, freeing unused memory. If the size is
 zero, it will be in an idle state. Invalidates pointers in `a`.
 @return Success. @throws[ERANGE, realloc] (Unlikely) `realloc` error. */
static int T_(array_shrink)(struct T_(array) *const a) {
	PT_(type) *data;
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
static void T_(array_remove)(struct T_(array) *const a,
	PT_(type) *const element) {
	const size_t n = (size_t)(element - a->data);
	assert(a && element && element >= a->data && element < a->data + a->size);
	memmove(element, element + 1, sizeof *element * (--a->size - n));
}

/** Removes `datum` from `a` and replaces it with the tail. Do not attempt to
 remove an element that is not in `a`. @order \O(1). @allow */
static void T_(array_lazy_remove)(struct T_(array) *const a,
	PT_(type) *const datum) {
	size_t n = (size_t)(datum - a->data);
	assert(a && datum && datum >= a->data && datum < a->data + a->size);
	if(--a->size != n) memcpy(datum, a->data + a->size, sizeof *datum);
}

/** Sets `a` to be empty. That is, the size of `a` will be zero, but if it was
 previously in an active non-idle state, it continues to be.
 @order \Theta(1) @allow */
static void T_(array_clear)(struct T_(array) *const a)
	{ assert(a), a->size = 0; }

/** @return The last element or null if `a` is empty. @order \Theta(1) @allow */
static PT_(type) *T_(array_peek)(const struct T_(array) *const a)
	{ return assert(a), a->size ? a->data + a->size - 1 : 0; }

/** @return Value from the the top of `a` that is removed or null if the array
 is empty. @order \Theta(1) @allow */
static PT_(type) *T_(array_pop)(struct T_(array) *const a)
	{ return assert(a), a->size ? a->data + --a->size : 0; }

/** Adds `n` elements to the back of `a`. It will invalidate pointers in `a` if
 `n` is greater than the buffer space.
 @return A pointer to the elements. If `a` is idle and `n` is zero, a null
 pointer will be returned, otherwise null indicates an error.
 @throws[realloc, ERANGE] @allow */
static PT_(type) *T_(array_append)(struct T_(array) *const a, const size_t n)
	{ return assert(a), PT_(append)(a, n); }

/** Indices [`i0`, `i1`) of `a` will be replaced with a copy of `b`.
 @param[b] Can be null, which acts as empty, but cannot overlap with `a`.
 @return Success. @throws[realloc, ERANGE] @allow */
static int T_(array_splice)(struct T_(array) *restrict const a,
	const struct T_(array) *restrict const b,
	const size_t i0, const size_t i1) {
	const size_t a_range = i1 - i0, b_range = b ? b->size : 0;
	assert(a && a != b && i0 <= i1 && i1 <= a->size);
	if(a_range < b_range) { /* The output is bigger. */
		const size_t diff = b_range - a_range;
		if(!T_(array_buffer)(a, diff)) return 0;
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

#		ifdef HAVE_ITERATE_H
#			include "iterate.h" /** \include */
#		endif

static void PT_(unused_base_coda)(void);
static void PT_(unused_base)(void) {
	/* fixme: private and public iterators? why again… surely we can think of a
	 simpler solution. */
	PT_(iterator)(0); PT_(iterator_at)(0, 0); PT_(element)(0);
	PT_(next)(0); PT_(previous)(0);
	PT_(size)(0); PT_(at)(0, 0); PT_(tell_size)(0, 0);
	T_(array)(); T_(array_)(0);
	T_(array_insert)(0, 0, 0); T_(array_new)(0);
	T_(array_shrink)(0); T_(array_remove)(0, 0); T_(array_lazy_remove)(0, 0);
	T_(array_clear)(0); T_(array_peek)(0); T_(array_pop)(0);
	T_(array_append)(0, 0); T_(array_splice)(0, 0, 0, 0);
	PT_(unused_base_coda)();
}
static void PT_(unused_base_coda)(void) { PT_(unused_base)(); }

#	endif /* Produce code. */

#endif /* Base code. */



#if defined(ARRAY_TO_STRING) \
	&& !defined(ARRAY_DECLARE_ONLY) /* <!-- to string trait */
/** Thunk `e` -> `a`. */
/*static void PTU_(to_string)(const PT_(type) *e, char (*const a)[12])
	{ TU_(to_string)((const void *)e, a); } (fixme: now we suddenly don't need
	this? what changed? I mean, yeah, I would not want to use this hack, but
	still…) */
#	include "to_string.h" /** \include */
#	undef ARRAY_TO_STRING
#	ifndef ARRAY_TRAIT
#		define ARRAY_HAS_TO_STRING
#	endif
#endif /* to string trait --> */


#if defined(ARRAY_TEST) && !defined(ARRAY_TRAIT) \
	&& !defined(ARRAY_DECLARE_ONLY) /* <!-- test base */
#	include "../test/test_array.h"
#endif /* test base --> */


#if (defined(ARRAY_COMPARE) || defined(ARRAY_IS_EQUAL)) \
	&& !defined(ARRAY_DECLARE_ONLY) /* <!-- compare trait */
#	ifdef ARRAY_COMPARE
#		define COMPARE ARRAY_COMPARE
#	else
#		define COMPARE_IS_EQUAL ARRAY_IS_EQUAL
#	endif
#	include "compare.h" /** \include */
#	ifdef ARRAY_TEST
#		include "../test/test_array_compare.h"
#	endif
#	ifdef ARRAY_COMPARE
#		undef ARRAY_COMPARE
#	else
#		undef ARRAY_IS_EQUAL
#	endif
#endif /* compare trait --> */


#ifdef ARRAY_EXPECT_TRAIT
#	undef ARRAY_EXPECT_TRAIT
#else
#	undef BOX_MINOR_NAME
#	undef BOX_MINOR
#	undef BOX_MAJOR_NAME
#	undef BOX_MAJOR
#	undef BOX_ACCESS
#	undef BOX_CONTIGUOUS
#	undef ARRAY_NAME
#	undef ARRAY_TYPE
#	undef ARRAY_MIN_CAPACITY
#	ifdef ARRAY_HAS_TO_STRING
#		undef ARRAY_HAS_TO_STRING
#	endif
#	ifdef ARRAY_TEST
#		undef ARRAY_TEST
#	endif
#	ifdef ARRAY_DECLARE_ONLY
#		undef ARRAY_DECLARE_ONLY
#	endif
#	ifdef BOX_RESTRICT
#		undef BOX_RESTRICT
#		undef restrict
#	endif
#	undef TU_
#	undef PTU_
#	undef BOXTU_
#endif
#ifdef BOX_TRAIT
#	undef BOX_TRAIT
#endif

/** @license 2016 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Stand-alone header <../src/array.h>; examples <../test/test_array.c>.

 @subtitle Contiguous dynamic array

 ![Example of array.](../doc/array/array.png)

 <tag:<A>array> is a dynamic array that stores contiguous <typedef:<PA>type>.
 Resizing may be necessary when increasing the size of the array; this incurs
 amortised cost. As such, the contents are not stable.

 @param[BOX_NAME, BOX_TYPE]
 `<T>` that satisfies `C` naming conventions when mangled and a valid tag-type,
 <typedef:<PA>type>, associated therewith; required.

 @param[BOX_COMPARE, BOX_IS_EQUAL]
 Compare trait contained in <src/compare.h>. Requires
 `<name>[<trait>]compare` to be declared as <typedef:<PTU>compare_fn> or
 `<name>[<trait>]is_equal` to be declared as <typedef:<PTU>bipredicate_fn>,
 respectfully, (but not both.)

 @param[BOX_TO_STRING]
 To string trait contained in <src/to_string.h>. Requires
 `<name>[<trait>]to_string` be declared as <typedef:<PT>to_string_fn>.

 @param[BOX_EXPECT_TRAIT, BOX_TRAIT]
 Named traits are obtained by including `array.h` multiple times with
 `BOX_EXPECT_TRAIT` and then subsequently including the name in `BOX_TRAIT`.

 @param[BOX_DECLARE_ONLY]
 For headers in different compilation units.

 @std C89 */

#if !defined(BOX_NAME) || !defined(BOX_TYPE)
#	error Name or tag type undefined.
#endif
#if defined(BOX_TRAIT) ^ defined(BOX_MAJOR)
#	error BOX_TRAIT name must come after BOX_EXPECT_TRAIT.
#endif
#if defined(BOX_COMPARE) && defined(BOX_IS_EQUAL)
#	error Only one can be defined at a time.
#endif
#if defined(BOX_TEST) && (!defined(BOX_TRAIT) && !defined(BOX_TO_STRING) \
	|| defined(BOX_TRAIT) && !defined(BOX_HAS_TO_STRING))
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
#	define t_(n) BOX_CAT(BOX_MINOR_NAME, n)
#	define T_(n) t_(BOX_CAT(BOX_MAJOR_NAME, n))
#	define PT_(n) BOX_CAT(private, T_(n))
#endif
#ifdef BOX_TRAIT
#	define tu_(n) t_(BOX_CAT(BOX_TRAIT, n))
#	define TU_(n) T_(BOX_CAT(BOX_TRAIT, n))
#	define PTU_(n) PT_(BOX_CAT(BOX_TRAIT, n))
#else /* Anonymous trait. */
#	define tu_(n) t_(n)
#	define TU_(n) T_(n)
#	define PTU_(n) PT_(n)
#endif


#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#if !defined(restrict) && (!defined(__STDC__) || !defined(__STDC_VERSION__) \
	|| __STDC_VERSION__ < 199901L)
#	define BOX_RESTRICT /* Undo this at the end. */
#	define restrict /* Attribute only in C99+. */
#endif

#ifndef BOX_TRAIT /* Base code, necessarily first. */

/* Box override information stays until the box is done. */
#	define BOX_MINOR_NAME BOX_NAME
#	define BOX_MINOR PT_(type)
#	define BOX_MAJOR_NAME array
#	define BOX_MAJOR struct T_(array)
#	define BOX_ACCESS
#	define BOX_CONTIGUOUS

#	ifndef BOX_MIN_CAPACITY
#		define BOX_MIN_CAPACITY 3 /* > 1 */
#	endif
#	if BOX_MIN_CAPACITY <= 1
#		error BOX_MIN_CAPACITY > 1
#	endif

/** A valid tag type set by `BOX_TYPE`. */
typedef BOX_TYPE PT_(type);

/** Manages the array field `data` which has `size` elements. The space is
 indexed up to `capacity`, which is at least `size`.

 ![States.](../doc/array/states.png) */
struct t_(array) { size_t size, capacity; PT_(type) *data; };
typedef struct t_(array) PT_(box);
/* !data -> !size, data -> capacity >= min && size <= capacity <= max */

struct T_(cursor) { struct t_(array) *a; size_t i; };

/* fixme: a wrapper is a terrible way to make functions accessible; something
 like BOX_EXPORT_CONS… #ifdef BOX_EXPORT_SIZE, #define static, size_t t_(array)… */

#	ifndef BOX_DECLARE_ONLY /* Produce code: not for headers. */

/** @return A cursor at the beginning of a valid `a`. */
static struct T_(cursor) T_(begin)(struct t_(array) *const a)
	{ struct T_(cursor) cur; assert(a), cur.a = a, cur.i = 0; return cur; }
static struct T_(cursor) T_(end)(struct t_(array) *const a)
	{ struct T_(cursor) cur; assert(a), cur.a = a, cur.i = a->size; return cur;}
/** @return Whether the `cur` points to an element. */
static int T_(cursor_exists)(const struct T_(cursor) *const cur)
	{ return cur && cur->a && cur->a->data && cur->i < cur->a->size; }
/** @return Dereference the element pointed to by `cur` that exists. */
static PT_(type) *T_(cursor_look)(struct T_(cursor) *const cur)
	{ return cur->a->data + cur->i; }
/** Move next on `cur` that exists. */
static void T_(cursor_next)(struct T_(cursor) *const cur)
	{ if(cur->i == (size_t)~0) cur->a = 0; else cur->i++; }
/** Move back on `cur` that exists. */
static void T_(cursor_back)(struct T_(cursor) *const cur) {
	if(cur->i > cur->a->size) cur->i = cur->a->size; /* Clip. */
	if(!cur->i) cur->a = 0;
	else cur->i--;
}
/* T_(cursor) T_(cursor_remove)(struct T_(cursor) *const cur)?
  T_(cursor) T_(cursor_insert)(cur)? */
/** Size of `a`. @implements `size` */
static size_t T_(size)(const struct t_(array) *a) { return a->size; }
/** @return Element `idx` of `a`. @implements `at` */
static PT_(type) *T_(look)(const struct t_(array) *a, const size_t idx)
	{ return a->data + idx; }
/** Writes `size` to `a`. @implements `tell_size` */
static void T_(tell_size)(struct t_(array) *a, const size_t size) { a->size = size; }
/* fixme: why again? */

/** Zeroed data (not all-bits-zero) is initialized.
 @return An idle array. @order \Theta(1) @allow */
static struct t_(array) t_(array)(void)
	{ struct t_(array) a; a.data = 0, a.capacity = a.size = 0; return a; }

/** If `a` is not null, destroys and returns it to idle. @allow */
static void t_(array_)(struct t_(array) *const a)
	{ if(a) free(a->data), *a = t_(array)(); }

/** Ensures `min` capacity of `a`. Invalidates pointers in `a`. @param[min] If
 zero, does nothing. @return Success; otherwise, `errno` will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 doesn't follow POSIX. @throws[realloc] @allow */
static int T_(reserve)(struct t_(array) *const a, const size_t min) {
	size_t c0;
	PT_(type) *data;
	const size_t max_size = (size_t)~0 / sizeof *a->data;
	if(a->data) {
		assert(a->size <= a->capacity);
		if(min <= a->capacity) return 1;
		c0 = a->capacity < BOX_MIN_CAPACITY
			? BOX_MIN_CAPACITY : a->capacity;
	} else { /* Idle. */
		assert(!a->size && !a->capacity);
		if(!min) return 1;
		c0 = BOX_MIN_CAPACITY;
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
static PT_(type) *T_(buffer)(struct t_(array) *const a, const size_t n) {
	if(a->size > (size_t)~0 - n) { errno = ERANGE; return 0; }
	return T_(reserve)(a, a->size + n) && a->data ? a->data + a->size : 0;
}

/** Adds `n` elements to the back of `a`. It will invalidate pointers in `a` if
 `n` is greater than the buffer space.
 @implements `append` from `BOX_CONTIGUOUS`
 @return A pointer to the elements. If `a` is idle and `n` is zero, a null
 pointer will be returned, otherwise null indicates an error.
 @throws[realloc, ERANGE] @allow */
static PT_(type) *T_(append)(struct t_(array) *const a, const size_t n) {
	PT_(type) *b;
	if(!(b = T_(buffer)(a, n))) return 0;
	assert(n <= a->capacity && a->size <= a->capacity - n);
	return a->size += n, b;
}

/** Adds `n` un-initialised elements at position `at` in `a`. It will
 invalidate any pointers in `a` if the buffer holds too few elements.
 @param[at] A number smaller than or equal to `a.size`; if `a.size`, this
 function behaves as <fn:<A>array_append>.
 @return A pointer to the start of the new region, where there are `n`
 elements. @throws[realloc, ERANGE] @allow */
static PT_(type) *T_(insert)(struct t_(array) *const a,
	const size_t n, const size_t at) {
	/* Investigate `n` is better than `element`; all the other are element. But
	 also, when would I ever use this? */
	const size_t old_size = a->size;
	PT_(type) *const b = T_(append)(a, n);
	assert(a && at <= old_size);
	if(!b) return 0;
	memmove(a->data + at + n, a->data + at, sizeof *a->data * (old_size - at));
	return a->data + at;
}
/* fixme: All of this is stupid—why would I need it? */

/** @return Adds (push back) one new element of `a`. The buffer space holds at
 least one element, or it may invalidate pointers in `a`.
 @order amortised \O(1) @throws[realloc, ERANGE] @allow */
static PT_(type) *T_(new)(struct t_(array) *const a)
	{ return T_(append)(a, 1); }

/** Shrinks the capacity `a` to the size, freeing unused memory. If the size is
 zero, it will be in an idle state. Invalidates pointers in `a`.
 @return Success. @throws[ERANGE, realloc] (Unlikely) `realloc` error. */
static int T_(shrink)(struct t_(array) *const a) {
	PT_(type) *data;
	size_t c;
	assert(a && a->capacity >= a->size);
	if(!a->data) return assert(!a->size && !a->capacity), 1;
	c = a->size && a->size > BOX_MIN_CAPACITY ? a->size : BOX_MIN_CAPACITY;
	if(!(data = realloc(a->data, sizeof *a->data * c)))
		{ if(!errno) errno = ERANGE; return 0; }
	a->data = data, a->capacity = c;
	return 1;
}

/** Removes `element` from `a`. Do not attempt to remove an element that is not
 in `a`. @order \O(`a.size`). @allow */
static void T_(remove)(struct t_(array) *const a,
	PT_(type) *const element) {
	const size_t n = (size_t)(element - a->data);
	assert(a && element && element >= a->data && element < a->data + a->size);
	memmove(element, element + 1, sizeof *element * (--a->size - n));
}

/** Removes `datum` from `a` and replaces it with the tail. Do not attempt to
 remove an element that is not in `a`. @order \O(1). @allow */
static void T_(lazy_remove)(struct t_(array) *const a,
	PT_(type) *const datum) {
	size_t n = (size_t)(datum - a->data);
	assert(a && datum && datum >= a->data && datum < a->data + a->size);
	if(--a->size != n) memcpy(datum, a->data + a->size, sizeof *datum);
}

/** Sets `a` to be empty. That is, the size of `a` will be zero, but if it was
 previously in an active non-idle state, it continues to be.
 @order \Theta(1) @allow */
static void T_(clear)(struct t_(array) *const a)
	{ assert(a), a->size = 0; }

/** @return The last element or null if `a` is empty. @order \Theta(1) @allow */
static PT_(type) *T_(peek)(const struct t_(array) *const a)
	{ return assert(a), a->size ? a->data + a->size - 1 : 0; }

/** @return Value from the the top of `a` that is removed or null if the array
 is empty. @order \Theta(1) @allow */
static PT_(type) *T_(pop)(struct t_(array) *const a)
	{ return assert(a), a->size ? a->data + --a->size : 0; }

/** Indices [`i0`, `i1`) of `a` will be replaced with a copy of `b`.
 @param[b] Can be null, which acts as empty, but cannot overlap with `a`.
 @return Success. @throws[realloc, ERANGE] @allow */
static int T_(splice)(struct t_(array) *restrict const a,
	const struct t_(array) *restrict const b,
	const size_t i0, const size_t i1) {
	const size_t a_range = i1 - i0, b_range = b ? b->size : 0;
	assert(a && a != b && i0 <= i1 && i1 <= a->size);
	if(a_range < b_range) { /* The output is bigger. */
		const size_t diff = b_range - a_range;
		if(!T_(buffer)(a, diff)) return 0;
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
	T_(begin)(0); T_(end)(0); T_(cursor_exists)(0); T_(cursor_look)(0);
	T_(cursor_next)(0); T_(cursor_back)(0); T_(size)(0); T_(look)(0, 0);
	T_(tell_size)(0, 0);
	t_(array)(); t_(array_)(0); T_(insert)(0, 0, 0); T_(new)(0); T_(shrink)(0);
	T_(remove)(0, 0); T_(lazy_remove)(0, 0); T_(clear)(0); T_(peek)(0);
	T_(pop)(0); T_(append)(0, 0); T_(splice)(0, 0, 0, 0);
	PT_(unused_base_coda)();
}
static void PT_(unused_base_coda)(void) { PT_(unused_base)(); }

#	endif /* Produce code. */

#endif /* Base code. */



#if defined(BOX_TO_STRING) \
	&& !defined(BOX_DECLARE_ONLY) /* <!-- to string trait */
#	include "to_string.h" /** \include */
#	undef BOX_TO_STRING
#	ifndef BOX_TRAIT
#		define BOX_HAS_TO_STRING /* Warning about lack of to_string in tests. */
#	endif
#endif /* to string trait --> */


#if defined(BOX_TEST) && !defined(BOX_TRAIT) \
	&& !defined(BOX_DECLARE_ONLY) /* <!-- test base */
#	include "../test/test_array.h"
#endif /* test base --> */


#if (defined(BOX_COMPARE) || defined(BOX_IS_EQUAL)) \
	&& !defined(BOX_DECLARE_ONLY) /* <!-- compare trait */
#	ifdef BOX_COMPARE
#		define COMPARE BOX_COMPARE
#	else
#		define COMPARE_IS_EQUAL BOX_IS_EQUAL
#	endif
#	include "compare.h" /** \include */
#	ifdef BOX_TEST
#		include "../test/test_array_compare.h"
#	endif
#	ifdef BOX_COMPARE
#		undef BOX_COMPARE
#	else
#		undef BOX_IS_EQUAL
#	endif
#endif /* compare trait --> */


#ifdef BOX_EXPECT_TRAIT
#	undef BOX_EXPECT_TRAIT
#else
#	undef BOX_MINOR_NAME
#	undef BOX_MINOR
#	undef BOX_MAJOR_NAME
#	undef BOX_MAJOR
#	undef BOX_ACCESS
#	undef BOX_CONTIGUOUS
#	undef BOX_NAME
#	undef BOX_TYPE
#	undef BOX_MIN_CAPACITY
#	ifdef BOX_HAS_TO_STRING
#		undef BOX_HAS_TO_STRING
#	endif
#	ifdef BOX_TEST
#		undef BOX_TEST
#	endif
#	ifdef BOX_DECLARE_ONLY
#		undef BOX_DECLARE_ONLY
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
#undef tu_
#undef TU_
#undef PTU_

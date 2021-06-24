/** @license 2016 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Contiguous Dynamic Array (Vector)

 ![Example of array.](../web/array.png)

 <tag:<A>array> is a dynamic array that stores contiguous `ARRAY_TYPE`.
 Resizing may be necessary when increasing the size of the array. This incurs
 amortised cost and any pointers to this memory may become stale.

 @param[ARRAY_NAME, ARRAY_TYPE]
 `<A>` that satisfies `C` naming conventions when mangled and a valid tag-type
 associated therewith; required. `<PA>` is private, whose names are prefixed in
 a manner to avoid collisions.

 @param[ARRAY_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[ARRAY_TO_STRING_NAME, ARRAY_TO_STRING]
 To string trait contained in <to_string.h>; `<Z>` that satisfies `C` naming
 conventions when mangled and function implementing <typedef:<PZ>to_string_fn>.
 There can be multiple to string traits, but only one can omit
 `ARRAY_TO_STRING_NAME`.

 @param[ARRAY_TEST]
 To string trait contained in <../test/array_test.h>; optional unit testing
 framework using `assert`. Can only be defined once _per_ array. Must be
 defined equal to a (random) filler function, satisfying
 <typedef:<PA>action_fn>. Output will be shown with the to string trait in
 which it's defined; provides tests for the base code and all later traits.

 @param[ARRAY_COMPARABLE_NAME, ARRAY_IS_EQUAL, ARRAY_COMPARE]
 Comparable trait; `<C>` that satisfies `C` naming conventions when mangled
 and a function implementing, for `ARRAY_IS_EQUAL` <typedef:<PA>bipredicate_fn>
 that establishes an equivalence relation, or for `ARRAY_COMPARE`
 <typedef:<PA>compare_fn> that establishes a total order. There can be multiple
 comparable traits, but only one can omit `ARRAY_COMPARABLE_NAME`.

 @std C89 */

#include <stdlib.h> /* realloc free */
#include <assert.h> /* assert */
#include <string.h> /* memcpy memmove (strlen) (strerror strcpy memcmp) */
#include <limits.h> /* LONG_MAX */
#include <errno.h>  /* errno */


#if !defined(ARRAY_NAME) || !defined(ARRAY_TYPE)
#error Name ARRAY_NAME or tag type ARRAY_TYPE undefined.
#endif
#if defined(ARRAY_TO_STRING_NAME) || defined(ARRAY_TO_STRING) /* <!-- str */
#ifndef ARRAY_ITERATE
#error ARRAY_ITERATE must be defined for string trait.
#endif
#define ARRAY_TO_STRING_TRAIT 1
#else /* str --><!-- !str */
#define ARRAY_TO_STRING_TRAIT 0
#endif /* !str --> */
#if defined(ARRAY_COMPARABLE_NAME) || defined(ARRAY_COMPARE) \
	|| defined(ARRAY_IS_EQUAL)
#ifndef ARRAY_ITERATE
#error ARRAY_ITERATE must be defined for compare trait.
#endif
#define ARRAY_COMPARABLE_TRAIT 1
#else
#define ARRAY_COMPARABLE_TRAIT 0
#endif
#define ARRAY_TRAITS ARRAY_TO_STRING_TRAIT + ARRAY_COMPARABLE_TRAIT
#if ARRAY_TRAITS > 1
#error Only one trait per include is allowed; use ARRAY_EXPECT_TRAIT.
#endif
#if ARRAY_TRAITS != 0 && (!defined(A_) || !defined(CAT) || !defined(CAT_))
#error A_ or CAT_? not yet defined; use ARRAY_EXPECT_TRAIT?
#endif
#if (ARRAY_TRAITS == 0) && defined(ARRAY_TEST)
#error ARRAY_TEST must be defined in ARRAY_TO_STRING trait.
#endif
#if defined(ARRAY_TO_STRING_NAME) && !defined(ARRAY_TO_STRING)
#error ARRAY_TO_STRING_NAME requires ARRAY_TO_STRING.
#endif
#if defined(ARRAY_COMPARABLE_NAME) \
	&& (!(!defined(ARRAY_COMPARE) ^ !defined(ARRAY_IS_EQUAL)))
#error ARRAY_COMPARABLE_NAME requires ARRAY_COMPARE or ARRAY_IS_EQUAL not both.
#endif


#if ARRAY_TRAITS == 0 /* <!-- base code */


/* <Kernighan and Ritchie, 1988, p. 231>. */
#if defined(A_) || defined(PA_) \
	|| (defined(ARRAY_SUBTYPE) ^ (defined(CAT) || defined(CAT_)))
#error Unexpected P?A_ or CAT_?; possible stray ARRAY_EXPECT_TRAIT?
#endif
#ifndef ARRAY_SUBTYPE /* <!-- !sub-type */
#define CAT_(x, y) x ## _ ## y
#define CAT(x, y) CAT_(x, y)
#endif /* !sub-type --> */
#define A_(thing) CAT(ARRAY_NAME, thing)
#define PA_(thing) CAT(array, A_(thing))


/** A valid tag type set by `ARRAY_TYPE`. */
typedef ARRAY_TYPE PA_(type);

/** Operates by side-effects. */
typedef void (*PA_(action_fn))(PA_(type) *);

/** Returns a boolean given read-only `<A>`. */
typedef int (*PA_(predicate_fn))(const PA_(type) *);

/** Returns a boolean given two read-only `<A>`. */
typedef int (*PA_(bipredicate_fn))(const PA_(type) *, const PA_(type) *);

/** Returns a boolean given two `<A>`. */
typedef int (*PA_(biproject_fn))(PA_(type) *, PA_(type) *);

/** Three-way comparison on a totally order set; returns an integer value less
 then, equal to, greater then zero, if `a < b`, `a == b`, `a > b`,
 respectively. */
typedef int (*PA_(compare_fn))(const PA_(type) *a, const PA_(type) *b);

/** Manages the array field `data` which has `size` elements. The space is
 indexed up to `capacity`, which is at least `size`. To initialize it to an
 idle state, see <fn:<A>array>, `ARRAY_IDLE`, `{0}` (`C99`,) or being `static`.

 ![States.](../web/states.png) */
struct A_(array);
/* !data -> !size, data -> capacity >= min && size <= capacity <= max */
struct A_(array) { PA_(type) *data; size_t size, capacity; };
#ifndef ARRAY_IDLE /* <!-- !zero; `{0}` is `C99`. */
#define ARRAY_IDLE { 0, 0, 0 }
#endif /* !zero --> */
#ifndef ARRAY_MIN_CAPACITY /* <!-- !min; */
#define ARRAY_MIN_CAPACITY 3 /* > 1 */
#endif /* !min --> */

/** Initialises `a` to idle. @order \Theta(1) @allow */
static void A_(array)(struct A_(array) *const a)
	{ assert(a), a->data = 0, a->capacity = a->size = 0; }

/** Destroys `a` and returns it to idle. @allow */
static void A_(array_)(struct A_(array) *const a)
	{ assert(a), free(a->data), A_(array)(a); }

/** @return Converts `i` to an index in `a` from [0, `a.size`]. Negative values
 are implicitly plus `a.size`. @order \Theta(1) @allow */
static size_t A_(array_clip)(const struct A_(array) *const a, const long i) {
	/* `SIZE_MAX` is `C99`; assumes two's-compliment, not many hw-tests. */
	assert(a && (size_t)-1 >= (size_t)LONG_MAX
		&& (unsigned long)((size_t)-1) >= LONG_MAX);
	return i < 0
		? (size_t)-i >= a->size ? 0 : a->size - (size_t)-i
		: (size_t)i > a->size ? a->size : (size_t)i;
}

/** Ensures `min` capacity of `a`. Invalidates pointers in `a`. @param[min] If
 zero, does nothing. @return Success; otherwise, `errno` will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] @allow */
static int A_(array_reserve)(struct A_(array) *const a, const size_t min) {
	size_t c0;
	PA_(type) *data;
	const size_t max_size = (size_t)-1 / sizeof *a->data;
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
	while(c0 < min) {
		size_t c1 = c0 + (c0 >> 1) + (c0 >> 3);
		if(c0 >= c1) { c0 = max_size; break; } /* Unlikely. */
		c0 = c1;
	}
	if(!(data = realloc(a->data, sizeof *a->data * c0)))
		{ if(!errno) errno = ERANGE; return 0; }
	a->data = data, a->capacity = c0;
	return 1;
}

/** The capacity of `a` will be increased to at least `buffer` elements beyond
 the size. Invalidates pointers in `a`.
 @return The start of the buffered space, (the back of the array.) If `a` is
 idle and `buffer` is zero, a null pointer is returned, otherwise null
 indicates an error. @throws[realloc, ERANGE] @allow */
static PA_(type) *A_(array_buffer)(struct A_(array) *const a, const size_t n) {
	assert(a);
	if(a->size > (size_t)-1 - n) { errno = ERANGE; return 0; }
	return A_(array_reserve)(a, a->size + n) && a->data ? a->data + a->size : 0;
}

/** Adds `n` elements to the back of `a`. The buffer holds enough elements or
 it will invalidate pointers in `a`.
 @return A pointer to the elements. If `a` is idle and `n` is zero, a null
 pointer will be returned, otherwise null indicates an error.
 @throws[realloc, ERANGE] @allow */
static PA_(type) *A_(array_append)(struct A_(array) *const a, const size_t n) {
	PA_(type) *const buffer = A_(array_buffer)(a, n);
	assert(a);
	if(!buffer) return 0;
	assert(n <= a->capacity && a->size <= a->capacity - n);
	return a->size += n, buffer;
}

/** Adds `n` un-initialised elements at position `at` in `a`. The buffer holds
 enough elements or it will invalidate pointers in `a`.
 @param[at] A number smaller than or equal to `a.size`; if `a.size`, this
 function behaves as <fn:<A>array_append>.
 @return A pointer to the start of the new region, where there are `n`
 elements. @throws[realloc, ERANGE] @allow */
static PA_(type) *A_(array_append_at)(struct A_(array) *const a,
	const size_t n, const size_t at) {
	const size_t old_size = a->size;
	PA_(type) *const buffer = A_(array_append)(a, n);
	assert(a && at <= old_size);
	if(!buffer) return 0;
	memmove(a->data + at + n, a->data + at, sizeof a->data * (old_size - at));
	return a->data + at;
}

/** @return Adds (append, push back) one new element of `a`. The buffer holds 
 an element or it will invalidate pointers in `a`.
 @order amortised \O(1) @throws[realloc, ERANGE] */
static PA_(type) *A_(array_new)(struct A_(array) *const a)
	{ return A_(array_append)(a, 1); }

/** Shrinks the capacity `a` to the size, freeing unused memory. If the size is
 zero, it will be in an idle state. Invalidates pointers in `a`.
 @return Success. @throws[ERANGE, realloc] Unlikely `realloc` error. */
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

/** Removes `datum` from `a`. @order \O(`a.size`). @allow */
static void A_(array_remove)(struct A_(array) *const a,
	PA_(type) *const datum) {
	const size_t n = (size_t)(datum - a->data);
	assert(a && datum && datum >= a->data && datum < a->data + a->size);
	memmove(datum, datum + 1, sizeof *datum * (--a->size - n));
}

/** Removes `datum` from `a` and replaces it with the tail.
 @order \O(1). @allow */
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

/*--- Consider having a contiguous trait that all these can go into? It doesn't
 really matter that it takes no arguments and it is the only one?
 Use the iterator? ---*/

/** @return The last element or null if `a` is empty. @order \Theta(1) @allow */
static PA_(type) *A_(array_peek)(const struct A_(array) *const a)
	{ return assert(a), a->size ? a->data + a->size - 1 : 0; }

/** @return Value from the the top of `a` that is removed or null if the array
 is empty. @order \Theta(1) @allow */
static PA_(type) *A_(array_pop)(struct A_(array) *const a)
	{ return assert(a), a->size ? a->data + --a->size : 0; }

/** `a` indices [`i0`, `i1`) will be replaced with a copy of `b`.
 @param[b] Can be null, which acts as empty.
 @return Success. @throws[realloc, ERANGE] */
static int A_(array_splice)(struct A_(array) *const a, const size_t i0,
	const size_t i1, const struct A_(array) *const b) {
	const size_t a_range = i1 - i0, b_range = b ? b->size : 0;
	assert(a && a != b && i0 <= i1 && i1 <= a->size);
	if(a_range < b_range) { /* The output is bigger. */
		const size_t diff = b_range - a_range;
		/*if(a->size > (size_t)-1 - diff) return errno = ERANGE, 0;*/
		if(!A_(array_buffer)(a, diff)) return 0;
		/*if(!A_(array_reserve)(a, a->size + diff)) return 0;*/
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

/** Copies `b`, which can be null, to the back of `a`.
 @return Success. @throws[realloc, ERANGE] */
static int A_(array_copy)(struct A_(array) *const a,
	const struct A_(array) *const b)
	{ return A_(array_splice)(a, a->size, a->size, b); }

/** For all elements of `b`, calls `copy`, and if true, lazily copies the
 elements to `a`. `a` and `b` can not be the same but `b` can be null.
 @order \O(`b.size` \times `copy`) @throws[ERANGE, realloc] @allow */
static int A_(array_copy_if)(struct A_(array) *const a,
	const PA_(predicate_fn) copy, const struct A_(array) *const b) {
	PA_(type) *i, *fresh;
	const PA_(type) *end, *rise = 0;
	size_t add;
	int difcpy = 0;
	assert(a && copy && a != b);
	if(!b) return 1;
	for(i = b->data, end = i + b->size; i < end; i++) {
		if(!(!!rise ^ (difcpy = copy(i)))) continue; /* Not falling/rising. */
		if(difcpy) { /* Rising edge. */
			assert(!rise);
			rise = i;
		} else { /* Falling edge. */
			assert(rise && !difcpy && rise < i);
			if(!(fresh = A_(array_append)(a, add = (size_t)(i - rise))))
				return 0;
			memcpy(fresh, rise, sizeof *fresh * add);
			rise = 0;
		}
	}
	if(rise) { /* Delayed copy. */
		assert(!difcpy && rise < i);
		if(!(fresh = A_(array_append)(a, add = (size_t)(i - rise))))
			return 0;
		memcpy(fresh, rise, sizeof *fresh * add);
	}
	return 1;
}

/** For all elements of `a`, calls `keep`, and if false, lazy deletes that
 item, calling `destruct` if not-null.
 @order \O(`a.size` \times `keep` \times `destruct`) @allow */
static void A_(array_keep_if)(struct A_(array) *const a,
	const PA_(predicate_fn) keep, const PA_(action_fn) destruct) {
	PA_(type) *erase = 0, *t;
	const PA_(type) *retain = 0, *end;
	int keep0 = 1, keep1 = 0;
	assert(a && keep);
	for(t = a->data, end = a->data + a->size; t < end; keep0 = keep1, t++) {
		if(!(keep1 = !!keep(t)) && destruct) destruct(t);
		if(!(keep0 ^ keep1)) continue; /* Not a falling/rising edge. */
		if(keep1) { /* Rising edge. */
			assert(erase && !retain);
			retain = t;
		} else if(erase) { /* Falling edge. */
			size_t n = (size_t)(t - retain);
			assert(erase < retain && retain < t);
			memmove(erase, retain, n * sizeof *t);
			erase += n;
			retain = 0;
		} else { /* Falling edge, (first time only.) */
			erase = t;
		}
	}
	if(!erase) return; /* All elements were kept. */
	if(keep1) { /* Delayed move when the iteration ended; repeat. */
		size_t n = (size_t)(t - retain);
		assert(retain && erase < retain && retain < t);
		memmove(erase, retain, n * sizeof *t);
		erase += n;
	}
	/* Adjust the size. */
	assert((size_t)(erase - a->data) <= a->size);
	a->size = (size_t)(erase - a->data);
}

/** Removes at either end of `a` of things that `predicate` returns true.
 @order \O(`a.size` \times `predicate`) @allow */
static void A_(array_trim)(struct A_(array) *const a,
	const PA_(predicate_fn) predicate) {
	size_t i;
	assert(a && predicate);
	while(a->size && predicate(a->data + a->size - 1)) a->size--;
	for(i = 0; i < a->size && predicate(a->data + i); i++);
	if(!i) return;
	assert(i < a->size);
	memmove(a->data, a->data + i, sizeof *a->data * i), a->size -= i;
}

/** Iterates through `a` and calls `action` on all the elements. The topology
 of the list should not change while in this function.
 @order \O(`a.size` \times `action`) @allow */
static void A_(array_each)(struct A_(array) *const a,
	const PA_(action_fn) action) {
	PA_(type) *i, *i_end;
	assert(a && action);
	for(i = a->data, i_end = i + a->size; i < i_end; i++) action(i);
}

/** Iterates through `a` and calls `action` on all the elements for which
 `predicate` returns true. The topology of the list should not change while in
 this function. @order \O(`a.size` \times `predicate` \times `action`) @allow */
static void A_(array_if_each)(struct A_(array) *const a,
	const PA_(predicate_fn) predicate, const PA_(action_fn) action) {
	PA_(type) *i, *i_end;
	assert(a && predicate && action);
	for(i = a->data, i_end = i + a->size; i < i_end; i++)
		if(predicate(i)) action(i);
}

/** Iterates through `a` and calls `predicate` until it returns true.
 @return The first `predicate` that returned true, or, if the statement is
 false on all, null. @order \O(`a.size` \times `predicate`) @allow */
static PA_(type) *A_(array_any)(const struct A_(array) *const a,
	const PA_(predicate_fn) predicate) {
	PA_(type) *i, *i_end;
	if(!a || !predicate) return 0;
	for(i = a->data, i_end = i + a->size; i < i_end; i++)
		if(predicate(i)) return i;
	return 0;
}

#ifdef ARRAY_ITERATE /* <!-- iterate */

/* Iterators. */
#define ARRAY_NATURAL_(thing) CAT(A_(array_natural), thing)
#define ARRAY_REVERSE_(thing) CAT(A_(array_reverse), thing)

/** Contains all iteration parameters. */
struct PA_(iterator);
struct PA_(iterator) { const struct A_(array) *a; size_t i; };

/** Loads `a` into `it`. @implements begin */
static void PA_(begin)(struct PA_(iterator) *const it,
	const struct A_(array) *const a) { assert(it && a), it->a = a, it->i = 0; }

/** Advances `it`. @implements next */
static const PA_(type) *PA_(next)(struct PA_(iterator) *const it) {
	return assert(it && it->a), it->i < it->a->size ? it->a->data + it->i++ : 0;
}

#define ITERATE struct PA_(iterator)
#define ITERATE_BOX struct A_(array)
#define ITERATE_TYPE PA_(type)
#define ITERATE_BEGIN PA_(begin)
#define ITERATE_NEXT PA_(next)
#include "iterate.h" /** \include */

#endif /* iterate --> */


static void PA_(unused_base_coda)(void);
static void PA_(unused_base)(void) {
	A_(array_)(0); A_(array_clip)(0, 0); A_(array_append_at)(0, 0, 0);
	A_(array_new)(0); A_(array_shrink)(0); A_(array_remove)(0, 0);
	A_(array_lazy_remove)(0, 0); A_(array_clear)(0); A_(array_peek)(0);
	A_(array_pop)(0); A_(array_splice)(0, 0, 0, 0); A_(array_copy)(0, 0);
	A_(array_keep_if)(0, 0, 0); A_(array_copy_if)(0, 0, 0);
	A_(array_trim)(0, 0); A_(array_each)(0, 0); A_(array_if_each)(0, 0, 0);
	A_(array_any)(0, 0);
#ifdef ARRAY_ITERATE
	PA_(begin)(0, 0); PA_(next)(0);
#endif
	PA_(unused_base_coda)();
}
static void PA_(unused_base_coda)(void) { PA_(unused_base)(); }


#elif defined(ARRAY_TO_STRING) /* base code --><!-- to string trait */


#ifdef ARRAY_TO_STRING_NAME /* <!-- name */
#define Z_(thing) CAT(A_(array), CAT(ARRAY_TO_STRING_NAME, thing))
#else /* name --><!-- !name */
#define Z_(thing) CAT(A_(array), thing)
#endif /* !name --> */
#define TO_STRING ARRAY_TO_STRING
#include "to_string.h" /** \include */

#if !defined(ARRAY_TEST_BASE) && defined(ARRAY_TEST) /* <!-- test */
#define ARRAY_TEST_BASE /* Only one instance of base tests. */
#include "../test/test_array.h" /** \include */
#endif /* test --> */

#undef Z_
#undef ARRAY_TO_STRING
#ifdef ARRAY_TO_STRING_NAME
#undef ARRAY_TO_STRING_NAME
#endif


#else /* to string trait --><!-- comparable trait */


#ifdef ARRAY_COMPARABLE_NAME /* <!-- name */
#define PTC_(thing) CAT(PA_(thing), ARRAY_COMPARABLE_NAME)
#define T_C_(thing1, thing2) CAT(A_(thing1), CAT(ARRAY_COMPARABLE_NAME, thing2))
#else /* name --><!-- !name */
#define PTC_(thing) CAT(PA_(thing), anonymous)
#define T_C_(thing1, thing2) CAT(A_(thing1), thing2)
#endif /* !name --> */

#ifdef ARRAY_COMPARE /* <!-- compare */

/* Check that `ARRAY_COMPARE` is a function implementing
 <typedef:<PA>compare>. */
static const PA_(compare_fn) PTC_(compare) = (ARRAY_COMPARE);

/** Lexicographically compares `a` to `b`, which both can be null.
 @return { `a < b`: negative, `a == b`: zero, `a > b`: positive }.
 @order \O(`a.size`) @allow */
static int T_C_(array, compare)(const struct A_(array) *const a,
	const struct A_(array) *const b) {
	PA_(type) *ia, *ib, *end;
	int diff;
	/* Null counts as `-\infty`. */
	if(!a) return b ? -1 : 0;
	else if(!b) return 1;
	if(a->size > b->size) {
		for(ia = a->data, ib = b->data, end = ib + b->size; ib < end;
			ia++, ib++) if((diff = PTC_(compare)(ia, ib))) return diff;
		return 1;
	} else {
		for(ia = a->data, ib = b->data, end = ia + a->size; ia < end;
			ia++, ib++) if((diff = PTC_(compare)(ia, ib))) return diff;
		return -(a->size != b->size);
	}
}

/** `a` should be partitioned true/false with less-then `value`.
 @return The first index of `a` that is not less than `value`.
 @order \O(log `a.size`) @allow */
static size_t T_C_(array, lower_bound)(const struct A_(array) *const a,
	const PA_(type) *const value) {
	size_t low = 0, high = a->size, mid;
	assert(a && value);
	while(low < high) if(PTC_(compare)(value, a->data
		+ (mid = low + ((high - low) >> 1))) <= 0) high = mid;
		else low = mid + 1;
	return low;
}

/** `a` should be partitioned false/true with greater-than or equals `value`.
 @return The first index of `a` that is greater than `value`.
 @order \O(log `a.size`) @allow */
static size_t T_C_(array, upper_bound)(const struct A_(array) *const a,
	const PA_(type) *const value) {
	size_t low = 0, high = a->size, mid;
	assert(a && value);
	while(low < high) if(PTC_(compare)(value, a->data
		+ (mid = low + ((high - low) >> 1))) >= 0) low = mid + 1;
		else high = mid;
	return low;
}

/** Copies `datum` at the lower bound of a sorted `a`.
 @return Success. @order \O(`a.size`) @throws[realloc, ERANGE] */
static int T_C_(array, insert)(struct A_(array) *const a,
	const PA_(type) *const datum) {
	size_t bound;
	assert(a && datum);
	bound = T_C_(array, lower_bound)(a, datum);
	if(!A_(array_new)(a)) return 0;
	memmove(a->data + bound + 1, a->data + bound,
		sizeof *a->data * (a->size - bound - 1));
	memcpy(a->data + bound, datum, sizeof *datum);
	return 1;
}

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PTC_(vcompar)(const void *const a, const void *const b)
	{ return PTC_(compare)(a, b); }

/** Sorts `a` by `qsort` on `ARRAY_COMPARE`.
 @order \O(`a.size` \log `a.size`) @allow */
static void T_C_(array, sort)(struct A_(array) *const a)
	{ assert(a), qsort(a->data, a->size, sizeof *a->data, PTC_(vcompar)); }

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PTC_(vrevers)(const void *const a, const void *const b)
	{ return PTC_(compare)(b, a); }

/** Sorts `a` in reverse by `qsort` on `ARRAY_COMPARE`.
 @order \O(`a.size` \log `a.size`) @allow */
static void T_C_(array, reverse)(struct A_(array) *const a)
	{ assert(a), qsort(a->data, a->size, sizeof *a->data, PTC_(vrevers)); }

/** !compare(`a`, `b`) == equals(`a`, `b`) for not `ARRAY_IS_EQUAL`.
 @implements <PA>bipredicate */
static int PTC_(is_equal)(const void *const a, const void *const b)
	{ return !PTC_(compare)(a, b); }

#else /* compare --><!-- is equal */

/* Check that `ARRAY_IS_EQUAL` is a function implementing
 <typedef:<PA>bipredicate>. */
static const PA_(bipredicate) PTC_(is_equal) = (ARRAY_IS_EQUAL);

#endif /* is equal --> */

/** @return If `a` piecewise equals `b`, which both can be null.
 @order \O(`size`) */
static int T_C_(array, is_equal)(const struct A_(array) *const a,
	const struct A_(array) *const b) {
	PA_(type) *ia, *ib, *end;
	if(!a) return !b;
	if(!b || a->size != b->size) return 0;
	for(ia = a->data, ib = b->data, end = ia + a->size; ia < end; ia++, ib++)
		if(!PTC_(is_equal)(a, b)) return 0;
	return 1;
}

/** Removes consecutive duplicate elements in `a`.
 @param[merge] Controls surjection. Called with duplicate elements, if false
 `(x, y)->(x)`, if true `(x,y)->(y)`. More complex functions, `(x, y)->(x+y)`
 can be simulated by mixing the two in the value returned. Can be null: behaves
 like false. @order \O(`a.size` \times `merge`) @allow */
static void T_C_(array, merge_unique)(struct A_(array) *const a,
	const PA_(biproject_fn) merge) {
	size_t target, from, cursor, choice, next, move;
	const size_t last = a->size;
	int is_first, is_last;
	assert(a);
	for(target = from = cursor = 0; cursor < last; cursor += next) {
		/* Bijective `[from, cursor)` is moved lazily. */
		for(choice = 0, next = 1; cursor + next < last && PTC_(is_equal)(a->data
			+ cursor + choice, a->data + cursor + next); next++)
			if(merge && merge(a->data + choice, a->data + next)) choice = next;
		if(next == 1) continue;
		/* Must move injective `cursor + choice \in [cursor, cursor + next)`. */
		is_first = !choice;
		is_last  = (choice == next - 1);
		move = cursor - from + (size_t)is_first;
		memmove(a->data + target, a->data + from, sizeof *a->data * move),
		target += move;
		if(!is_first && !is_last) memcpy(a->data + target,
			a->data + cursor + choice, sizeof *a->data), target++;
		from = cursor + next - (size_t)is_last;
	}
	/* Last differed move. */
	move = last - from;
	memmove(a->data + target, a->data + from, sizeof *a->data * move),
	target += move, assert(a->size >= target);
	a->size = target;
}

/** Removes consecutive duplicate elements in `a`. @order \O(`a.size`) @allow */
static void T_C_(array, unique)(struct A_(array) *const a)
	{ T_C_(array, merge_unique)(a, 0); }

static void PTC_(unused_contrast_coda)(void);
static void PTC_(unused_contrast)(void) {
#ifdef ARRAY_COMPARE /* <!-- compare */
	T_C_(array, compare)(0, 0); T_C_(array, lower_bound)(0, 0);
	T_C_(array, upper_bound)(0, 0); T_C_(array, insert)(0, 0);
	T_C_(array, sort)(0); T_C_(array, reverse)(0);
#endif /* compare --> */
	T_C_(array, is_equal)(0, 0); T_C_(array, unique)(0);
	PTC_(unused_contrast_coda)();
}
static void PTC_(unused_contrast_coda)(void) { PTC_(unused_contrast)(); }

#if defined(ARRAY_TEST_BASE) && defined(ARRAY_TEST) /* <!-- test */
#include "../test/test_array.h"
#endif /* test --> */

#undef PTC_
#undef T_C_
#ifdef ARRAY_COMPARE
#undef ARRAY_COMPARE
#endif
#ifdef ARRAY_IS_EQUAL
#undef ARRAY_IS_EQUAL
#endif
#ifdef ARRAY_COMPARABLE_NAME
#undef ARRAY_COMPARABLE_NAME
#endif


#endif /* traits --> */


#ifdef ARRAY_EXPECT_TRAIT /* <!-- trait */
#undef ARRAY_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifndef ARRAY_SUBTYPE /* <!-- !sub-type */
#undef CAT
#undef CAT_
#else /* !sub-type --><!-- sub-type */
#undef ARRAY_SUBTYPE
#endif /* sub-type --> */
#undef ...
#undef A_
#undef PA_
#undef ARRAY_NAME
#undef ARRAY_TYPE
#ifdef ARRAY_TEST
#undef ARRAY_TEST
#endif
#ifdef ARRAY_TEST_BASE
#undef ARRAY_TEST_BASE
#endif
#endif /* !trait --> */

#undef ARRAY_TO_STRING_TRAIT
#undef ARRAY_COMPARABLE_TRAIT
#undef ARRAY_TRAITS

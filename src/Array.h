/** @license 2016 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Parameterised Contiguous Dynamic Array (Vector)

 ![Example of Array](../web/array.png)

 <tag:<T>Array> is a dynamic array that stores contiguous data. To ensure that
 the capacity is greater then or equal to the size, resizing may be necessary
 and incurs amortised cost.

 `<T>Array` is not synchronised. Errors are returned with `errno`. The
 parameters are preprocessor macros, and are all undefined at the end of the
 file for convenience. `assert.h` is included in this file; to stop the
 debug assertions, use `#define NDEBUG` before `assert.h`.

 @param[ARRAY_NAME, ARRAY_TYPE]
 `<T>` that satisfies `C` naming conventions when mangled and a valid tag-type
 associated therewith; required. `<PT>` is private, whose names are prefixed in
 a manner to avoid collisions; any should be re-defined prior to use elsewhere.

 @param[ARRAY_UNFINISHED]
 Do not un-define variables for including again in an interface.

 @param[ARRAY_TO_STRING_NAME, ARRAY_TO_STRING]
 To string interface contained in <ToString.h>; `<A>` that satisfies `C` naming
 conventions when mangled and function implementing <typedef:<PT>ToString>.
 There can be multiple to string interfaces, but only one can omit
 `ARRAY_TO_STRING_NAME`.

 @param[ARRAY_TEST]
 To string interface optional unit testing framework using `assert`; contained
 in <../test/ArrayTest.h>. Can only be defined once per `Array`. Must be
 defined equal to a (random) filler function, satisfying <typedef:<PT>Action>.

 @param[ARRAY_CONTRAST_NAME, ARRAY_COMPARE, ARRAY_IS_EQUAL]
 Compare interface; `<C>` that satiscfies `C` naming conventions when mangled
 and a function implementing <typedef:<PT>Compare> or <typedef:<PT>Bipredicate>
 that satisfies an equality ...fixme There can
 be multiple contrast interfaces, but only one can omit `ARRAY_CONTRAST_NAME`.

 @std C89
 @cf [Heap](https://github.com/neil-edelman/Heap)
 @cf [List](https://github.com/neil-edelman/List)
 @cf [Orcish](https://github.com/neil-edelman/Orcish)
 @cf [Pool](https://github.com/neil-edelman/Pool)
 @cf [Set](https://github.com/neil-edelman/Set)
 @cf [Trie](https://github.com/neil-edelman/Trie) */

#include <stdlib.h> /* realloc free */
#include <assert.h> /* assert */
#include <string.h> /* memcpy memmove (strlen) (strerror strcpy memcmp) */
#include <errno.h>  /* errno */

/* Check defines. */
#ifndef ARRAY_NAME
#error Name ARRAY_NAME undefined.
#endif
#ifndef ARRAY_TYPE
#error Tag type ARRAY_TYPE undefined.
#endif
#define ARRAY_INTERFACES (defined(ARRAY_TO_STRING_NAME) \
	|| defined(ARRAY_TO_STRING)) + (defined(ARRAY_CONTRAST_NAME) \
	|| defined(ARRAY_COMPARE) || defined(ARRAY_IS_EQUAL))
#if ARRAY_INTERFACES > 1
#error Only one interface per include is allowed; use ARRAY_UNFINISHED.
#endif
#if (ARRAY_INTERFACES == 0) && defined(ARRAY_TEST)
#error ARRAY_TEST must be defined in ARRAY_TO_STRING interface.
#endif
#if defined(ARRAY_TO_STRING_NAME) && !defined(ARRAY_TO_STRING)
#error ARRAY_TO_STRING_NAME requires ARRAY_TO_STRING.
#endif
#if defined(ARRAY_CONTRAST_NAME) \
	&& !defined(ARRAY_COMPARE) && !defined(ARRAY_IS_EQUAL)
#error ARRAY_CONTRAST_NAME requires ARRAY_COMPARE or ARRAY_IS_EQUAL.
#endif


#if ARRAY_INTERFACES == 0 /* <!-- base code */


/* <Kernighan and Ritchie, 1988, p. 231>. */
#if defined(T) || defined(T_) || defined(PT_)
#error P?T_? cannot be defined; possible stray ARRAY_UNFINISHED?
#endif
#ifndef ARRAY_CHILD /* <!-- !sub-type */
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#elif !defined(CAT) || !defined(PCAT) /* !sub-type --><!-- !cat */
#error ARRAY_CHILD defined but CAT is not.
#endif /* !cat --> */
#define T_(thing) CAT(ARRAY_NAME, thing)
#define PT_(thing) PCAT(array, PCAT(ARRAY_NAME, thing))

/** A valid tag type set by `ARRAY_TYPE`. This becomes `T`. */
typedef ARRAY_TYPE PT_(Type);
#define T PT_(Type)

/** Operates by side-effects. */
typedef void (*PT_(Action))(T *);

/** Returns a boolean given read-only `<T>`. */
typedef int (*PT_(Predicate))(const T *);

/** Returns a boolean given two read-only `<T>`. */
typedef int (*PT_(Bipredicate))(const T *, const T *);

/** Returns a boolean given two `<T>`. */
typedef int (*PT_(Biproject))(T *, T *);

/** Returns an integer value greater, less, or equal zero fixme... */
typedef int (*PT_(Compare))(const T *, const T *);

/** Responsible for turning the first argument into a 12-`char` null-terminated
 output string. Used for `ARRAY_TO_STRING`. */
typedef void (*PT_(ToString))(const T *, char (*)[12]);

/** Manages the array field `data`, which is indexed up to `size`. When
 modifying the topology of this array, it may change memory location to fit;
 any pointers to this memory may become stale. To initialise it to an idle
 state, see <fn:<T>Array>, `ARRAY_IDLE`, `{0}` (`C99`,) or being `static`.

 ![States.](../web/states.png) */
struct T_(Array);
/* !data -> !size, data -> capacity >= min && size <= capacity <= max */
struct T_(Array) { T *data; size_t size, capacity; };
/* `{0}` is `C99`. */
#ifndef ARRAY_IDLE /* <!-- !zero */
#define ARRAY_IDLE { 0, 0, 0 }
#endif /* !zero --> */

/** Contains all iteration parameters in one. */
struct PT_(Iterator) { const struct T_(Array) *a; size_t i; };

/** Initialises `a` to idle. */
static void PT_(array)(struct T_(Array) *const a)
	{ assert(a), a->data = 0, a->capacity = a->size = 0; }

/** Destroys `a` and returns it to idle. */
static void PT_(array_)(struct T_(Array) *const a)
	{ assert(a), free(a->data), PT_(array)(a); }

/** Ensures `min_capacity` of `a`.
 @param[min_capacity] If zero, does nothing.
 @param[update_ptr] Must be in the array or null, it updates this value.
 @return Success; otherwise, `errno` will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] */
static int PT_(update_reserve)(struct T_(Array) *const a,
	const size_t min_capacity, T **const update_ptr) {
	size_t c0;
	T *data;
	const size_t max_size = (size_t)-1 / sizeof *a->data;
	assert(a);
	if(a->data) {
		if(min_capacity <= a->capacity) return 1;
		c0 = a->capacity;
		if(c0 < 8) c0 = 8;
	} else { /* Idle. */
		if(!min_capacity) return 1;
		c0 = 8;
	}
	if(min_capacity > max_size) return errno = ERANGE, 0;
	/* `c_n = a1.625^n`, approximation golden ratio `\phi ~ 1.618`. Still,
	 Fibonacci increases slightly more because added term with negative. */
	while(c0 < min_capacity) {
		size_t c1 = c0 + (c0 >> 1) + (c0 >> 3);
		if(c0 >= c1) { c0 = max_size; break; } /* Overflow; very unlikely. */
		c0 = c1;
	}
	if(!(data = realloc(a->data, sizeof *a->data * c0)))
		{ if(!errno) errno = ERANGE; return 0; }
	if(update_ptr && a->data != data)
		*update_ptr = data + (*update_ptr - a->data); /* Not strict ISO. */
	a->data = data, a->capacity = c0;
	return 1;
}

/** Call `reserve_update` with `a`, `min_capacity`, and no `update_ptr`; which
 is what usually happens. */
static int PT_(reserve)(struct T_(Array) *const a, const size_t min_capacity)
	{ return PT_(update_reserve)(a, min_capacity, 0); }

/** Shrinks `a` to the size. */
static int PT_(shrink)(struct T_(Array) *const a) {
	T *data;
	assert(a && a->capacity >= a->size);
	if(!a->data) return assert(!a->size), 1;
	if(!a->size) return PT_(array_)(a), 1;
	if(!(data = realloc(a->data, sizeof *a->data * a->size)))
		{ if(!errno) errno = ERANGE; return 0; } /* Unlikely. */
	a->data = data;
	a->capacity = a->size;
	return 1;
}

/** In `a`, converts `anchor` and `range` à-la-Python and stores them in the
 pointers `p0` and `p1` _st_ `*p0, *p1 \in [0, a.size], *p0 <= *p1`.
 @param[anchor] An element in the array or null to indicate past the end.
 @return Success. @throws[ERANGE] `anchor` is not null and not in `a`. `range`
 is greater then +/-65534. `size_t` overflow. */
static int PT_(range)(const struct T_(Array) *const a, const T *anchor,
	const long range, size_t *const p0, size_t *const p1) {
	size_t i0, i1;
	assert(a && p0 && p1);
	if((anchor && (anchor < a->data || anchor >= a->data + a->size))
		|| range > 65534l || range < -65534l) return errno = ERANGE, 0;
	i0 = anchor ? (size_t)(anchor - a->data) : a->size;
	if(range < 0) {
		i1 = (size_t)(-range) > a->size ? 0 : a->size - (size_t)(-range) + 1;
		if(i0 > i1) i1 = i0;
	} else {
		i1 = i0 + (size_t)range;
		if(i0 > i1) return errno = ERANGE, 0;
		if(i1 > a->size) i1 = a->size;
	}
	*p0 = i0, *p1 = i1;
	return 1;
}

/** `a` indices [`i0`, `i1`) will be replaced with `b`. */
static int PT_(replace)(struct T_(Array) *const a, const size_t i0,
	const size_t i1, const struct T_(Array) *const b) {
	const size_t a_range = i1 - i0, b_range = b ? b->size : 0;
	assert(a && a != b && i0 <= i1 && i1 <= a->size);
	if(a_range < b_range) { /* The output is bigger. */
		const size_t diff = b_range - a_range;
		if(a->size > (size_t)-1 - diff) return errno = ERANGE, 0;
			if(!PT_(reserve)(a, a->size + diff)) return 0;
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

/** Returns a new un-initialized datum of `a` and updates `update_ptr`. */
static T *PT_(update_new)(struct T_(Array) *const a, T **const update_ptr) {
	assert(a);
	return PT_(update_reserve)(a, a->size + 1, update_ptr)
		? a->data + a->size++ : 0;
}

/** Returns a new un-initialized datum of `a`. */
static T *PT_(new)(struct T_(Array) *const a) {
	assert(a);
	return PT_(reserve)(a, a->size + 1) ? a->data + a->size++ : 0;
}

#ifndef ARRAY_CHILD /* <!-- !sub-type */

/** Initialises `a` to be idle. @order \Theta(1) @allow */
static void T_(Array)(struct T_(Array) *const a) { if(a) PT_(array)(a); }

/** Returns `a` to the idle state where it takes no dynamic memory.
 @param[a] If null, does nothing. @order \Theta(1) @allow */
static void T_(Array_)(struct T_(Array) *const a) { if(a) PT_(array_)(a); }

/** Removes `datum` from `a`.
 @param[a, datum] If null, returns false.
 @return Success, otherwise `errno` will be set for valid input.
 @throws[EDOM] `datum` is not part of `a`. @order \O(n). @allow */
static int T_(ArrayRemove)(struct T_(Array) *const a, T *const datum) {
	size_t n;
	if(!a || !datum) return 0;
	if(datum < a->data
		|| (n = datum - a->data) >= a->size) return errno = EDOM, 0;
	memmove(datum, datum + 1, sizeof *datum * (--a->size - n));
	return 1;
}

/** Removes `datum` from `a` and replaces it with the tail.
 @param[a, datum] If null, returns false.
 @return Success, otherwise `errno` will be set for valid input.
 @throws[EDOM] `datum` is not part of `a`. @order \O(1). @allow */
static int T_(ArrayLazyRemove)(struct T_(Array) *const a, T *const datum) {
	size_t n;
	if(!a || !datum) return 0;
	if(datum < a->data
	   || (n = datum - a->data) >= a->size) return errno = EDOM, 0;
	if(--a->size != n) memcpy(datum, a->data + a->size, sizeof *datum);
	return 1;
}

/** Sets `a` to be empty. That is, the size of `a` will be zero, but if it was
 previously in an active non-idle state, it continues to be.
 @param[a] If null, does nothing. @order \Theta(1) @allow */
static void T_(ArrayClear)(struct T_(Array) *const a)
	{ if(a) a->size = 0; }

/** @param[a] If null, returns null.
 @return The last element or null if the a is empty.
 @order \Theta(1) @allow */
static T *T_(ArrayPeek)(const struct T_(Array) *const a)
	{ return a && a->size ? a->data + a->size - 1 : 0; }

/** The same value as <fn:<T>ArrayPeek>.
 @param[a] If null, returns null.
 @return Value from the the top of the `a` that is removed or null if the
 stack is empty.
 @order \Theta(1)
 @allow */
static T *T_(ArrayPop)(struct T_(Array) *const a)
	{ return a && a->size ? a->data + --a->size : 0; }

/** @param[a] If is null, returns null.
 @return A new, un-initialised, element at the back of `a`, or null and `errno`
 will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 error and doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] @order Amortised \O(1). @allow */
static T *T_(ArrayNew)(struct T_(Array) *const a)
	{ return a ? PT_(new)(a) : 0; }

/** @param[a] If null, returns null.
 @param[update_ptr] Pointer to update on memory move if it is within the memory
 region that was changed to accommodate new space.
 @return A new, un-initialised, element at the back of `a`, or null and `errno`
 will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 error and doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] @order Amortised \O(1). @allow */
static T *T_(ArrayUpdateNew)(struct T_(Array) *const a,
	T **const update_ptr) { return a ? PT_(update_new)(a, update_ptr) : 0; }

/** Ensures that `a` is `reserve` capacity beyond the elements in the array.
 @param[a] If null, returns null.
 @return The previous end of `a`, where are `reserve` elements, or null
 and `errno` will be set. Writing on this memory space is safe on success, up
 to `reserve` elements, but one will have to increase the size, (see
 <fn:<T>ArrayBuffer>.)
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 error and doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] @allow */
static T *T_(ArrayReserve)(struct T_(Array) *const a, const size_t reserve) {
	if(!a) return 0;
	if(!reserve) return a->data ? a->data + a->size : 0;
	if(a->size > (size_t)-1 - reserve) { errno = ERANGE; return 0; }
	if(!PT_(reserve)(a, a->size + reserve)) return 0;
	assert(a->data);
	return a->data + a->size;
}

/** Adds `add` elements to `a`.
 @param[a] If null, returns null.
 @param[add] If zero, returns null.
 @return The start of a new sub-array of `add` elements at the previous end of
 `a`, or null and `errno` will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 error and doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html). If
 <fn:<T>ArrayReserve> has been successful in reserving at least `add` elements,
 one is guaranteed success.
 @throws[realloc] @order Amortised \O(`add`). @allow */
static T *T_(ArrayBuffer)(struct T_(Array) *const a, const size_t add) {
	size_t prev_size;
	if(!a || !add) return 0;
	if(a->size > (size_t)-1 - add) { errno = ERANGE; return 0; } /* Unlikely. */
	if(!PT_(reserve)(a, a->size + add)) return 0;
	prev_size = a->size;
	a->size += add;
	return a->data + prev_size;
}

/** Shrinks the capacity `a` to the size, freeing unsed memory. If the size is
 zero, it will be in an idle state.
 @return Success. @throws[ERANGE, realloc] Unlikely `realloc` error. */
static int T_(ArrayShrink)(struct T_(Array) *const a)
	{ return a ? PT_(shrink)(a) : 0; }

/** Iterates through `a` and calls `action` on all the elements. The topology
 of the list should not change while in this function.
 
 @fixme All these functions can go into Sequetial?
 
 @param[a, action] If null, does nothing.
 @order \O(`size` \times `action`) @allow */
static void T_(ArrayEach)(struct T_(Array) *const a,
	const PT_(Action) action) {
	T *t, *end;
	if(!a || !action) return;
	for(t = a->data, end = t + a->size; t < end; t++) action(t);
}

/** Iterates through `a` and calls `action` on all the elements for which
 `predicate` returns true. The topology of the list can not change while in
 this function.
 @param[a, predicate, action] If null, does nothing.
 @order \O(`size` \times `action`) @allow */
static void T_(ArrayIfEach)(struct T_(Array) *const a,
	const PT_(Predicate) predicate, const PT_(Action) action) {
	T *t, *end;
	if(!a || !action || !predicate) return;
	for(t = a->data, end = t + a->size; t < end; t++)
		if(predicate(t)) action(t);
}

/** Iterates through `a` and calls `predicate` until it returns true.
 @param[a, predicate] If null, returns null.
 @return The first `predicate` that returned true, or, if the statement is
 false on all, null. @order \O(`size` \times `predicate`) @allow */
static T *T_(ArrayAny)(const struct T_(Array) *const a,
	const PT_(Predicate) predicate) {
	T *t, *end;
	if(!a || !predicate) return 0;
	for(t = a->data, end = t + a->size; t < end; t++)
		if(predicate(t)) return t;
	return 0;
}

/** For all elements of `a`, calls `keep`, and for each element, if the return
 value is false, lazy deletes that item, calling `destruct` if not-null.
 @param[a, keep] If null, does nothing.
 @order \O(`size`) @allow */
static void T_(ArrayKeepIf)(struct T_(Array) *const a,
	const PT_(Predicate) keep, const PT_(Action) destruct) {
	T *erase = 0, *t;
	const T *retain = 0, *end;
	int keep0 = 1, keep1 = 0;
	if(!a || !keep) return;
	for(t = a->data, end = a->data + a->size; t < end; keep0 = keep1, t++) {
		if(!(keep1 = !!keep(t)) && destruct) destruct(t);
		if(!(keep0 ^ keep1)) continue; /* Not a falling/rising edge. */
		if(keep1) { /* Rising edge. */
			assert(erase && !retain);
			retain = t;
		} else if(erase) { /* Falling edge. */
			size_t n = t - retain;
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
		size_t n = t - retain;
		assert(retain && erase < retain && retain < t);
		memmove(erase, retain, n * sizeof *t);
		erase += n;
	}
	/* Adjust the size. */
	assert((size_t)(erase - a->data) <= a->size);
	a->size = erase - a->data;
}

/** Removes at either end of `a` of things that `predicate` returns true.
 @param[a, predicate] If null, does nothing.
 @order \O(`size`)
 @allow */
static void T_(ArrayTrim)(struct T_(Array) *const a,
	const PT_(Predicate) predicate) {
	size_t i;
	if(!a || !predicate) return;
	while(a->size && predicate(a->data + a->size - 1)) a->size--;
	for(i = 0; i < a->size && predicate(a->data + i); i++);
	if(!i) return;
	assert(i < a->size);
	memmove(a->data, a->data + i, sizeof *a->data * i), a->size -= i;
}

/** In `a`, replaces the elements from `anchor` up to `range` with a copy of
 `b`.
 @param[a] If null, returns zero.
 @param[anchor] Beginning of the replaced value, inclusive. If null, appends to
 the end.
 @param[range] How many replaced values in the original; negative values are
 implicitly plus the length of the array; clamped at the minimum and maximum.
 @param[b] The replacement array. If null, deletes without replacing. It is
 more efficient than individual <fn:<T>ArrayRemove> to delete several
 consecutive values.
 @return Success.
 @throws[EDOM] `a` and `b` are not null and the same.
 @throws[ERANGE] `anchor` is not null and not in `a`.
 @throws[ERANGE] `range` is greater then 65535 or smaller then -65534.
 @throws[ERANGE] `b` would cause the array to overflow.
 @throws[realloc]
 @order \Theta(`b.size`) if the elements have the same size, otherwise,
 amortised \O(`a.size` + `b.size`).
 @allow */
static int T_(ArraySplice)(struct T_(Array) *const a, const T *anchor,
	const long range, const struct T_(Array) *const b) {
	size_t i0, i1;
	if(!a) return 0;
	if(a == b) return errno = EDOM, 0;
	if(!PT_(range)(a, anchor, range, &i0, &i1)) return 0;
	return PT_(replace)(a, i0, i1, b);
}

/** In `a`, replaces the elements from indices `i0` (inclusive) to `i1`
 (exclusive) with a copy of `b`.
 @param[a] If null, returns zero.
 @param[i0, i1] The replacement indices, `[i0, i1)`, such that
 `0 <= i0 <= i1 <= a.size`.
 @param[b] The replacement array. If null, deletes without replacing.
 @return Success.
 @throws[EDOM] `a` and `b` are not null and the same.
 @throws[EDOM] `i0` or `i1` are out-of-bounds or `i0 > i1`.
 @throws[ERANGE] `b` would cause the array to overflow.
 @throws[realloc]
 @order \Theta(`b.size`) if the elements have the same size, otherwise,
 amortised \O(`a.size` + `b.size`).
 @allow */
static int T_(ArrayIndexSplice)(struct T_(Array) *const a, const size_t i0,
	const size_t i1, const struct T_(Array) *const b) {
	if(!a) return 0;
	if(a == b || i0 > i1 || i1 > a->size) return errno = EDOM, 0;
	return PT_(replace)(a, i0, i1, b);
}

#endif /* !sub-type --> */

static void PT_(unused_base_coda)(void);
static void PT_(unused_base)(void) {
	PT_(array)(0), PT_(array_)(0); PT_(update_reserve)(0, 0, 0);
	PT_(reserve)(0, 0); PT_(shrink)(0); PT_(range)(0, 0, 0, 0, 0);
	PT_(replace)(0, 0, 0, 0); PT_(update_new)(0, 0); PT_(new)(0);
#ifndef ARRAY_CHILD /* <!-- !sub-type */
	T_(Array_)(0); T_(Array)(0); T_(ArrayRemove)(0, 0);
	T_(ArrayLazyRemove)(0, 0); T_(ArrayClear)(0); T_(ArrayPeek)(0);
	T_(ArrayPop)(0); T_(ArrayNew)(0); T_(ArrayUpdateNew)(0, 0);
	T_(ArrayReserve)(0, 0); T_(ArrayBuffer)(0, 0); T_(ArrayShrink)(0);
	T_(ArrayEach)(0, 0); T_(ArrayIfEach)(0, 0, 0); T_(ArrayAny)(0, 0);
	T_(ArrayKeepIf)(0, 0, 0); T_(ArrayTrim)(0, 0); T_(ArraySplice)(0, 0, 0, 0);
	T_(ArrayIndexSplice)(0, 0, 0, 0);
#endif /* !sub-type --> */
	PT_(unused_base_coda)();
}
static void PT_(unused_base_coda)(void) { PT_(unused_base)(); }


#elif defined(ARRAY_TO_STRING) /* base code --><!-- to string interface */


#if !defined(T) || !defined(T_) || !defined(PT_) || !defined(CAT) \
	|| !defined(CAT_) || !defined(PCAT) || !defined(PCAT_)
#error P?T_? or P?CAT_? not yet defined in to string interface; include array?
#endif

#ifdef ARRAY_TO_STRING_NAME /* <!-- name */
#define PTA_(thing) PCAT(PT_(thing), ARRAY_TO_STRING_NAME)
#define T_A_(thing1, thing2) CAT(T_(thing1), CAT(ARRAY_TO_STRING_NAME, thing2))
#else /* name --><!-- !name */
#define PTA_(thing) PCAT(PT_(thing), anonymous)
#define T_A_(thing1, thing2) CAT(T_(thing1), thing2)
#endif /* !name --> */

/* Check that `ARRAY_TO_STRING` is a function implementing
 <typedef:<PT>ToString>. */
static const PT_(ToString) PTA_(to_str12) = (ARRAY_TO_STRING);

/** Writes `it` to `str` and advances or returns false.
 @implements <AI>NextToString */
static int PTA_(next_to_str12)(struct PT_(Iterator) *const it,
	char (*const str)[12]) {
	assert(it && it->a && str);
	if(it->i >= it->a->size) return 0;
	PTA_(to_str12)(it->a->data + it->i++, str);
	return 1;
}

/** @return If `it` contains a not-null pool. */
static int PTA_(is_valid)(const struct PT_(Iterator) *const it)
	{ assert(it); return !!it->a; }

#define AI_ PTA_
#define TO_STRING_ITERATOR struct PT_(Iterator)
#define TO_STRING_NEXT &PTA_(next_to_str12)
#define TO_STRING_IS_VALID &PTA_(is_valid)
#include "ToString.h"

/** @return Prints `a`. */
static const char *PTA_(to_string)(const struct T_(Array) *const a) {
	struct PT_(Iterator) it = { 0, 0 };
	it.a = a; /* Can be null. */
	return PTA_(iterator_to_string)(&it, '(', ')'); /* In ToString. */
}

#ifndef ARRAY_CHILD /* <!-- !sub-type */

/** @return Print the contents of `a` in a static string buffer with the
 limitations of `ToString.h`. @order \Theta(1) @allow */
static const char *T_A_(Array, ToString)(const struct T_(Array) *const a)
	{ return PTA_(to_string)(a); /* Can be null. */ }

#endif /* !sub-type --> */

static void PTA_(unused_to_string_coda)(void);
static void PTA_(unused_to_string)(void) {
	PTA_(to_string)(0);
#ifndef ARRAY_CHILD /* <!-- !sub-type */
	T_A_(Array, ToString)(0);
#endif /* !sub-type --> */
	PTA_(unused_to_string_coda)();
}
static void PTA_(unused_to_string_coda)(void) { PTA_(unused_to_string)(); }

#if !defined(ARRAY_TEST_BASE) && defined(ARRAY_TEST) /* <!-- test */
#define ARRAY_TEST_BASE /* Only one instance of base tests. */
#include "../test/TestArray.h"
#endif /* test --> */

#undef PTA_
#undef T_A_
#undef ARRAY_TO_STRING
#ifdef ARRAY_TO_STRING_NAME
#undef ARRAY_TO_STRING_NAME
#endif


#else /* to string interface --><!-- contrast interface */


#if !defined(T) || !defined(T_) || !defined(PT_) || !defined(CAT)
	|| !defined(CAT_) || !defined(PCAT) || !defined(PCAT_)
#error P?T_? or P?CAT_? not yet defined in contrast interface; include array?
#endif

#ifdef ARRAY_CONTRAST_NAME /* <!-- name */
#define PTC_(thing) PCAT(PT_(thing), ARRAY_CONTRAST_NAME)
#define T_C_(thing1, thing2) CAT(T_(thing1), CAT(ARRAY_CONTRAST_NAME, thing2))
#else /* name --><!-- !name */
#define PTC_(thing) PCAT(PT_(thing), anonymous)
#define T_C_(thing1, thing2) CAT(T_(thing1), thing2)
#endif /* !name --> */

#ifdef ARRAY_COMPARE /* <!-- compare */

/* Check that `ARRAY_COMPARE` is a function implementing
 <typedef:<PT>Bipredicate>. */
static const PT_(Bipredicate) PTC_(compare) = (ARRAY_COMPARE);

/** !compare(`a`, `b`) == equals(`a`, `b`) */
static int PTC_(is_equal)(const void *const a, const void *const b)
	{ return !PTC_(compare)(a, b); }

/** Wrapper with void `a` and `b`. @implements qsort */
static int PTC_(compar)(const void *const a, const void *const b)
	{ return PTC_(compare)(a, b); }

/** Wrapper with void `a` and `b`. @implements qsort */
static int PTC_(revers)(const void *const a, const void *const b)
	{ return PTC_(compare)(b, a); }

/** Loosely `C++` `lower_bound`. @param[a] Array.
 @return The first index of `a` that is not less then `value`. */
static size_t PTC_(lower_bound)(const struct T_(Array) *const a,
	const T *const value) {
	size_t low = 0, high = a->size, mid;
	assert(a && value);
	while(low < high)
		if(PTC_(compare)(value, a->data + (mid = low + ((high - low) >> 1)))
		<= 0) high = mid; else low = mid + 1;
	return low;
}

/** Loosely `C++` `upper_bound`. @param[a] Array;
 @return The first index of `a` that is greater then `value`. */
static size_t PTC_(upper_bound)(const struct T_(Array) *const a,
	const T *const value) {
	size_t low = 0, high = a->size, mid;
	assert(a && value);
	while(low < high)
		if(PTC_(compare)(value, a->data + (mid = low + ((high - low) >> 1)))
		>= 0) low = mid + 1; else high = mid;
	return low;
}

/** Inserts `datum` in `a` at the lower bound. @return Success. */
static int PTC_(insert)(struct T_(Array) *const a, const T *const datum) {
	size_t bound;
	assert(a && datum);
	bound = PTC_(lower_bound)(a, datum);
	if(!PT_(new)(a)) return 0;
	memmove(a->data + bound + 1, a->data + bound,
		sizeof *a->data * (a->size - bound - 1));
	memcpy(a->data + bound, datum, sizeof *datum);
	return 1;
}

#else /* compare --><!-- is equal */

/* Check that `ARRAY_COMPARE` is a function implementing
 <typedef:<PT>Bipredicate>. */
static const PT_(Bipredicate) PTC_(is_equal) = (ARRAY_IS_EQUAL);

#endif /* is equal --> */

/** Calls `<PTC>is_equal` for each consecutive pair of elements in `a` and, if
 true, surjects two one according to `merge`. Loosely based on `C++` `unique`.
 @param[merge] If null, discards all but the first.
 @order \O(`a.size`) */
static void PTC_(compactify)(struct T_(Array) *const a,
	const PT_(Biproject) merge) {
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
		move = cursor - from + is_first;
		memmove(a->data + target, a->data + from, sizeof *a->data * move),
		target += move;
		if(!is_first && !is_last) memcpy(a->data + target,
			a->data + cursor + choice, sizeof *a->data), target++;
		from = cursor + next - is_last;
	}
	/* Last differed move. */
	move = last - from;
	memmove(a->data + target, a->data + from, sizeof *a->data * move),
	target += move, assert(a->size >= target);
	a->size = target;
}

#ifndef ARRAY_CHILD /* <!-- !sub-type */

#ifdef ARRAY_COMPARE /* <!-- compare */

/** Sorts `a` by `qsort`. @allow */
static void T_C_(Array, Sort)(struct T_(Array) *const a)
	{ if(a) qsort(a->data, a->size, sizeof *a->data, PTC_(compar)); }

/** Sorts `a` in reverse by `qsort`. @allow */
static void T_C_(Array, Reverse)(struct T_(Array) *const a)
	{ if(a) qsort(a->data, a->size, sizeof *a->data, PTC_(revers)); }

/* fixme: static int T_C_(Array, Compare)() */

/** `a` is a random-access array which should be partitioned true/false with
 less-then `value`.
 @param[a] If null, returns zero.
 @return The first index of `a` that is not less then `value`.
 @order \O(log `size`) @allow */
static size_t T_C_(Array, LowerBound)(struct T_(Array) *const a,
	const T *const value)
	{ return a && value ? PTC_(lower_bound)(a, value) : 0; }

/** `a` is a random-access array which should be partitioned false/true with
 greater-than or equals `value`.
 @param[a] If null, returns zero.
 @return The first index of `a` that is greater then `value`.
 @order \O(log `size`) @allow */
static size_t T_C_(Array, UpperBound)(struct T_(Array) *const a,
	const T *const value)
	{ return a && value ? PTC_(upper_bound)(a, value) : 0; }

/** Inserts a copy of `datum` in `a` at the lower bound.
 @param[a, datum] If null, does nothing.
 @return Success. @throws[ERANGE] Tried allocating more then can fit in
 `size_t` or `realloc` error and doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] @order Amortised \O(1). @allow */
static int T_C_(Array, Insert)(struct T_(Array) *const a,
	const T *const datum) { return a && datum ? PTC_(insert)(a, datum) : 0; }

#endif /* compare --> */

/* fixme: static int T_C_(Array, IsEqual)() */

/** Interfaces `ARRAY_COMPARE` or `ARRAY_IS_EQUAL`. For each consecutive and
 equal pair of elements in `a`, surjects to one according to `merge`.
 @param[a] If null, does nothing.
 @param[merge] Can be null, in which case, all duplicate entries are erased.
 @order \O(`a.size`) @allow */
static void T_C_(Array, Compactify)(struct T_(Array) *const a,
	const PT_(Biproject) merge) { if(a) PTC_(compactify)(a, merge); }

#endif /* !sub-type --> */

static void PTC_(unused_contrast_coda)(void);
static void PTC_(unused_contrast)(void) {
#ifdef ARRAY_COMPARE
	PTC_(lower_bound)(0, 0); PTC_(upper_bound)(0, 0); PTC_(insert)(0, 0);
#endif
	PTC_(compactify)(0, 0);
#ifndef ARRAY_CHILD /* <!-- !sub-type */
#ifdef ARRAY_COMPARE /* <!-- compare */
	T_C_(Array, Sort)(0); T_C_(Array, Reverse)(0);
	T_C_(Array, LowerBound)(0, 0); T_C_(Array, UpperBound)(0, 0);
	T_C_(Array, Insert)(0, 0);
#endif /* compare --> */
	T_C_(Array, Compactify)(0, 0);
#endif /* !sub-type --> */
	PTC_(unused_contrast_coda)();
}
static void PTC_(unused_contrast_coda)(void) { PTC_(unused_contrast)(); }

#if defined(ARRAY_TEST_BASE) && defined(ARRAY_TEST) /* <!-- test */
#include "../test/TestArray.h" /** \include */
#endif /* test --> */

#undef PTC_
#undef T_C_
#ifdef ARRAY_COMPARE
#undef ARRAY_COMPARE
#endif
#ifdef ARRAY_IS_EQUAL
#undef ARRAY_IS_EQUAL
#endif
#ifdef ARRAY_CONTRAST_NAME
#undef ARRAY_CONTRAST_NAME
#endif


#endif /* interfaces --> */


#ifdef ARRAY_UNFINISHED /* <!-- unfinish */
#undef ARRAY_UNFINISHED
#else /* unfinish --><!-- finish */
#ifndef ARRAY_CHILD /* <!-- !sub-type */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#endif /* !sub-type --> */
#undef T
#undef T_
#undef PT_
#undef ARRAY_NAME
#undef ARRAY_TYPE
#ifdef ARRAY_TEST
#undef ARRAY_TEST
#endif
#ifdef ARRAY_TEST_BASE
#undef ARRAY_TEST_BASE
#endif
#ifdef ARRAY_CHILD
#undef ARRAY_CHILD
#endif
#endif /* finish --> */

#undef ARRAY_INTERFACES

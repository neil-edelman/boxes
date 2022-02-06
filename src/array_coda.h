/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Array coda

 This defines an optional set of functions that is nice, but not necessary,
 for any child of `array` not providing additional constraints.

 @param[ARRAY_CODA_TYPE]
 Type of array.

 @param[ARRAY_CODA_BOX_TO_C, ARRAY_CODA_BOX_TO]
 Function picking out the array satisfying <typedef:<PAC>box_to_array_c> and
 <typedef:<PAC>box_to_array>.

 @param[AC_]
 A one-argument macro producing a name that is responsible for the name of the
 functions. Should be something like `AC_(x) -> foo_widget_x`. The caller is
 responsible for undefining `AC_`.

 @std C89 */

#if !defined(BOX_) || !defined(BOX_CONTAINER) || !defined(BOX_CONTENTS) \
	|| !defined(ARRAY_CODA_BOX_TO_C) || !defined(ARRAY_CODA_BOX_TO) \
	|| !defined(AC_)
#error Unexpected preprocessor symbols.
#endif

#ifndef ARRAY_CODA_H /* <!-- idempotent */
#define ARRAY_CODA_H
#include <limits.h>
#if defined(ARRAY_CODA_CAT_) || defined(ARRAY_CODA_CAT) || defined(PAC_)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define ARRAY_CODA_CAT_(n, m) n ## _ ## m
#define ARRAY_CODA_CAT(n, m) ARRAY_CODA_CAT_(n, m)
#define PAC_(n) ARRAY_CODA_CAT(array_coda, AC_(n))
#endif /* idempotent --> */

/** <array_coda.h>: an alias to the box. */
typedef BOX_CONTAINER PAC_(box);

/** <array_coda.h>: an alias to the individual type contained in the box. */
typedef BOX_CONTENTS PAC_(type);

/** <array_coda.h>: Operates by side-effects on <typedef:<PAC>type>. */
typedef void (*PAC_(action_fn))(PAC_(type) *);

/** <array_coda.h>: Returns a boolean given read-only <typedef:<PAC>type>. */
typedef int (*PAC_(predicate_fn))(const PAC_(type) *);

/* Array type. */
typedef ARRAY_CODA_TYPE PAC_(array);

/* Returns a constant array member of constant box. */
typedef const PAC_(array) *(*PAC_(box_to_array_c))(const PAC_(box) *);
static PAC_(box_to_array_c) PAC_(b2a_c) = (ARRAY_CODA_BOX_TO_C);

/* Returns an array member of box. */
typedef PAC_(array) *(*PAC_(box_to_array))(PAC_(box) *);
static PAC_(box_to_array) PAC_(b2a) = (ARRAY_CODA_BOX_TO);

/** <array_coda.h> @param[x] A valid entry or null to start from the last.
 @return The previous valid entry from `box` (which could be null) or null if
 this was the first. @allow */
static PAC_(type) *AC_(previous)(const PAC_(box) *const box,
	const PAC_(type) *const x) {
	const PAC_(array) *a;
	size_t i;
	if(!box || !(a = PAC_(b2a_c)(box))->data) return 0;
	if(!x) return a->size ? a->data + a->size - 1 : 0;
	return (i = (size_t)(x - a->data)) ? a->data + i - 1 : 0;
}

/** <array_coda.h> @param[x] A valid entry or null to start from the first.
 @return The next valid entry from `box` (which could be null) or null if this
 was the last. @allow */
static PAC_(type) *AC_(next)(const PAC_(box) *const box,
	const PAC_(type) *const x) {
	const PAC_(array) *a;
	size_t i;
	if(!box || !(a = PAC_(b2a_c)(box))->data) return 0;
	if(!x) return a->size ? a->data + 0 : 0;
	return (i = (size_t)(x - a->data) + 1) < a->size ? a->data + i : 0;
}

/** <array_coda.h> @return Converts `i` to an index in `box` from
 [0, `box.size`]. Negative values are wrapped. @order \Theta(1) @allow */
static size_t AC_(clip)(const PAC_(box) *const box, const long i) {
	const PAC_(array) *const a = PAC_(b2a_c)(box);
	/* `SIZE_MAX` is `C99`. This is not guaranteed at all, but is common? */
	assert(box && a && ~((size_t)0) >= (size_t)LONG_MAX
		&& (unsigned long)~((size_t)0) >= LONG_MAX);
	return i < 0
		? (size_t)-i >= a->size ? 0 : a->size - (size_t)-i
		: (size_t)i > a->size ? a->size : (size_t)i;
}

/** <array_coda.h>: For all elements of `b`, calls `copy`, and if true, lazily
 copies the elements to `a`. `a` and `b` can not be the same but `b` can be
 null, (in which case, it does nothing.)
 @order \O(`b.size` \times `copy`) @throws[ERANGE, realloc] @allow */
static int AC_(copy_if)(PAC_(box) *const a, const PAC_(predicate_fn) copy,
	const PAC_(box) *const b) {
	PAC_(array) *const aa = PAC_(b2a)(a);
	const PAC_(array) *const bb = b ? PAC_(b2a_c)(b) : 0;
	PAC_(type) *i, *fresh;
	const PAC_(type) *end, *rise = 0;
	size_t add;
	int difcpy = 0;
	assert(a && aa && !(!b ^ !bb) && copy && a != b && aa != bb);
	if(!b) return 1;
	for(i = bb->data, end = i + bb->size; i < end; i++) {
		if(!(!!rise ^ (difcpy = copy(i)))) continue; /* Not falling/rising. */
		if(difcpy) { /* Rising edge. */
			assert(!rise);
			rise = i;
		} else { /* Falling edge. */
			assert(rise && !difcpy && rise < i);
			if(!(fresh = BOX_(append)(a, add = (size_t)(i - rise)))) return 0;
			memcpy(fresh, rise, sizeof *fresh * add);
			rise = 0;
		}
	}
	if(rise) { /* Delayed copy. */
		assert(!difcpy && rise < i);
		if(!(fresh = BOX_(append)(a, add = (size_t)(i - rise)))) return 0;
		memcpy(fresh, rise, sizeof *fresh * add);
	}
	return 1;
}

/** <array_coda.h>: For all elements of `box`, calls `keep`, and if false, lazy
 deletes that item, calling `destruct` (if not-null).
 @order \O(`a.size` \times `keep` \times `destruct`) @allow */
static void AC_(keep_if)(PAC_(box) *const box,
	const PAC_(predicate_fn) keep, const PAC_(action_fn) destruct) {
	PAC_(array) *const a = PAC_(b2a)(box);
	PAC_(type) *erase = 0, *t;
	const PAC_(type) *retain = 0, *end;
	int keep0 = 1, keep1 = 0;
	assert(box && a && keep);
	for(t = a->data, end = t + a->size; t < end; keep0 = keep1, t++) {
		if(!(keep1 = !!keep(t)) && destruct) destruct(t);
		if(!(keep0 ^ keep1)) continue; /* Not a falling/rising edge. */
		if(keep1) { /* Rising edge. */
			assert(erase && !retain);
			retain = t;
		} else if(erase) { /* Falling edge. */
			size_t n = (size_t)(t - retain);
			assert(erase < retain && retain < t);
			memmove(erase, retain, sizeof *t * n);
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
		memmove(erase, retain, sizeof *t * n);
		erase += n;
	}
	/* Adjust the size. */
	assert((size_t)(erase - a->data) <= a->size);
	a->size = (size_t)(erase - a->data);
}

/** <array_coda.h>: Removes at either end of `box` of things that `predicate`
 returns true. @order \O(`box.size` \times `predicate`) @allow */
static void AC_(trim)(PAC_(box) *const box,
	const PAC_(predicate_fn) predicate) {
	PAC_(array) *const a = PAC_(b2a)(box);
	size_t i;
	assert(box && a && predicate);
	while(a->size && predicate(a->data + a->size - 1)) a->size--;
	for(i = 0; i < a->size && predicate(a->data + i); i++);
	if(!i) return;
	assert(i < a->size);
	memmove(a->data, a->data + i, sizeof *a->data * i), a->size -= i;
}

/** <array_coda.h>: Iterates through `box` and calls `action` on all the
 elements. The topology of the list should not change while in this function.
 @order \O(`box.size` \times `action`) @allow */
static void AC_(each)(PAC_(box) *const box, const PAC_(action_fn) action) {
	PAC_(array) *const a = PAC_(b2a)(box);
	PAC_(type) *i, *end;
	assert(box && a && action);
	for(i = a->data, end = i + a->size; i < end; i++) action(i);
}

/** <array_coda.h>: Iterates through `box` and calls `action` on all the
 elements for which `predicate` returns true. The topology of the list should
 not change while in this function.
 @order \O(`box.size` \times `predicate` \times `action`) @allow */
static void AC_(if_each)(PAC_(box) *const box,
	const PAC_(predicate_fn) predicate, const PAC_(action_fn) action) {
	PAC_(array) *const a = PAC_(b2a)(box);
	PAC_(type) *i, *end;
	assert(box && a && predicate && action);
	for(i = a->data, end = i + a->size; i < end; i++)
		if(predicate(i)) action(i);
}

/** <array_coda.h>: Iterates through `box` and calls `predicate` until it
 returns true.
 @return The first `predicate` that returned true, or, if the statement is
 false on all, null. @order \O(`box.size` \times `predicate`) @allow */
static const PAC_(type) *AC_(any)(const PAC_(box) *const box,
	const PAC_(predicate_fn) predicate) {
	const PAC_(array) *const a = PAC_(b2a_c)(box);
	PAC_(type) *i, *end;
	assert(box && a && predicate);
	for(i = a->data, end = i + a->size; i < end; i++)
		if(predicate(i)) return i;
	return 0;
}

static void PAC_(unused_function_coda)(void);
static void PAC_(unused_function)(void)
	{ AC_(previous)(0, 0); AC_(next)(0, 0); AC_(clip)(0, 0);
	AC_(copy_if)(0, 0, 0); AC_(keep_if)(0, 0, 0); AC_(trim)(0, 0);
	AC_(each)(0, 0); AC_(if_each)(0, 0, 0); AC_(any)(0, 0);
	PAC_(unused_function_coda)(); }
static void PAC_(unused_function_coda)(void) { PAC_(unused_function)(); }

#undef ARRAY_CODA_BOX_TO_C
#undef ARRAY_CODA_BOX_TO

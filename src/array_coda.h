/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Array coda

 This defines an optional set of functions that is nice, for any child of
 `array` not providing additional constraints. (Thus, `array`?)

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

#if !defined(BOX_) || !defined(BOX) || !defined(BOX_ENUM) \
	|| !defined(ARRAY_CODA_TYPE) || !defined(ARRAY_CODA_BOX_TO_C) \
	|| !defined(ARRAY_CODA_BOX_TO) || !defined(AC_) \
	|| defined(BOX_IS_EQUAL) && defined(BOX_COMPARE)
#error Unexpected preprocessor symbols.
#endif

#ifndef ARRAY_CODA_H /* <!-- idempotent */
#define ARRAY_CODA_H
#include <limits.h>
#ifdef PAC_
#error Unexpected defines.
#endif
#define PAC_(n) ARRAY_CAT(array_coda, AC_(n))
#endif /* idempotent --> */

#ifndef ARRAY_CODA_ONCE /* <!-- once */
#define ARRAY_CODA_ONCE
/** <src/array_coda.h>: an alias to the box. */
typedef BOX PAC_(box);
/** <src/array_coda.h>: an alias to the individual type contained in the box. */
typedef BOX_ENUM PAC_(enum);
/* Downcasting. */
typedef ARRAY_CODA_TYPE PAC_(array);
typedef const PAC_(array) *(*PAC_(box_to_array_c))(const PAC_(box) *);
static PAC_(box_to_array_c) PAC_(b2a_c) = (ARRAY_CODA_BOX_TO_C);
typedef PAC_(array) *(*PAC_(box_to_array))(PAC_(box) *);
static PAC_(box_to_array) PAC_(b2a) = (ARRAY_CODA_BOX_TO);
#endif /* once --> */


#if !defined(BOX_IS_EQUAL) && !defined(BOX_COMPARE) /* <!-- functions */


/** <src/array_coda.h>: Operates by side-effects on <typedef:<PAC>type>. */
typedef void (*PAC_(action_fn))(PAC_(enum));

/** <src/array_coda.h>: Returns a boolean given read-only <typedef:<PAC>type>. */
typedef int (*PAC_(predicate_fn))(const PAC_(enum));

/** <src/array_coda.h> @param[x] A valid entry or null to start from the last.
 @return The previous valid entry from `box` (which could be null) or null if
 this was the first. @allow */
static PAC_(enum) AC_(previous)(const PAC_(box) *const box,
	const PAC_(enum) x) {
	const PAC_(array) *a;
	size_t i;
	if(!box || !(a = PAC_(b2a_c)(box))->data) return 0;
	if(!x) return a->size ? a->data + a->size - 1 : 0;
	return (i = (size_t)(x - a->data)) ? a->data + i - 1 : 0;
}

/** <src/array_coda.h> @param[x] A valid entry or null to start from the first.
 @return The next valid entry from `box` (which could be null) or null if this
 was the last. @allow */
static PAC_(enum) AC_(next)(const PAC_(box) *const box, const PAC_(enum) x) {
	const PAC_(array) *a;
	size_t i;
	if(!box || !(a = PAC_(b2a_c)(box))->data) return 0;
	if(!x) return a->size ? a->data + 0 : 0;
	return (i = (size_t)(x - a->data) + 1) < a->size ? a->data + i : 0;
}

/** <src/array_coda.h> @return Converts `i` to an index in `box` from
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

/** <src/array_coda.h>: For all elements of `b`, calls `copy`, and if true, lazily
 copies the elements to `a`. `a` and `b` can not be the same but `b` can be
 null, (in which case, it does nothing.)
 @order \O(`b.size` \times `copy`) @throws[ERANGE, realloc] @allow */
static int AC_(copy_if)(PAC_(box) *const a, const PAC_(predicate_fn) copy,
	const PAC_(box) *const b) {
	PAC_(array) *const aa = PAC_(b2a)(a);
	const PAC_(array) *const bb = b ? PAC_(b2a_c)(b) : 0;
	PAC_(enum) i, fresh, end, rise = 0;
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

/** <src/array_coda.h>: For all elements of `box`, calls `keep`, and if false, lazy
 deletes that item, calling `destruct` (if not-null).
 @order \O(`a.size` \times `keep` \times `destruct`) @allow */
static void AC_(keep_if)(PAC_(box) *const box,
	const PAC_(predicate_fn) keep, const PAC_(action_fn) destruct) {
	PAC_(array) *const a = PAC_(b2a)(box);
	PAC_(enum) erase = 0, t, retain = 0, end;
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

/** <src/array_coda.h>: Removes at either end of `box` of things that `predicate`
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

/** <src/array_coda.h>: Iterates through `box` and calls `action` on all the
 elements. The topology of the list should not change while in this function.
 @order \O(`box.size` \times `action`) @allow */
static void AC_(each)(PAC_(box) *const box, const PAC_(action_fn) action) {
	PAC_(array) *const a = PAC_(b2a)(box);
	PAC_(enum) i, end;
	assert(box && a && action);
	for(i = a->data, end = i + a->size; i < end; i++) action(i);
}

/** <src/array_coda.h>: Iterates through `box` and calls `action` on all the
 elements for which `predicate` returns true. The topology of the list should
 not change while in this function.
 @order \O(`box.size` \times `predicate` \times `action`) @allow */
static void AC_(if_each)(PAC_(box) *const box,
	const PAC_(predicate_fn) predicate, const PAC_(action_fn) action) {
	PAC_(array) *const a = PAC_(b2a)(box);
	PAC_(enum) i, end;
	assert(box && a && predicate && action);
	for(i = a->data, end = i + a->size; i < end; i++)
		if(predicate(i)) action(i);
}

/** <src/array_coda.h>: Iterates through `box` and calls `predicate` until it
 returns true.
 @return The first `predicate` that returned true, or, if the statement is
 false on all, null. @order \O(`box.size` \times `predicate`) @allow */
static PAC_(enum) AC_(any)(const PAC_(box) *const box,
	const PAC_(predicate_fn) predicate) {
	const PAC_(array) *const a = PAC_(b2a_c)(box);
	PAC_(enum) i, end;
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

#else /* functions --><!-- compare/is equal */

#ifndef ARRAY_CODA_COMPARE_ONCE /* <!-- once */
#define ARRAY_CODA_COMPARE_ONCE
/** <src/array_coda.h>: Returns a boolean given two read-only <typedef:<PAC>type>. */
typedef int (*PAC_(bipredicate_fn))(const PAC_(enum), const PAC_(enum));
/** <src/array_coda.h>: Three-way comparison on a totally order set of
 <typedef:<PAC>type>; returns an integer value less then, equal to, greater
 then zero, if `a < b`, `a == b`, `a > b`, respectively. */
typedef int (*PAC_(compare_fn))(const PAC_(enum) a, const PAC_(enum) b);
/** <src/array_coda.h>: Returns a boolean given two <typedef:<PAC>type>. */
typedef int (*PAC_(biaction_fn))(PAC_(enum), PAC_(enum));
#endif /* once --> */

#ifdef ARRAY_CODA_NAME
#define ACC_(n) AC_(ARRAY_CAT(ARRAY_CODA_NAME, n))
#else /* name --><!-- !name */
#define ACC_(n) AC_(n)
#endif /* !name --> */
#define PACC_(n) ARRAY_CAT(array_coda, ACC_(n))

#ifdef BOX_COMPARE /* <!-- compare */

/* Check that `BOX_COMPARE` is a function implementing
 <typedef:<PAC>compare_fn>. */
static const PAC_(compare_fn) PACC_(compare) = (BOX_COMPARE);

/** <src/array_coda.h>: Lexicographically compares `a` to `b`. Both can be null,
 with null values before everything. @return `a < b`: negative; `a == b`: zero;
 `a > b`: positive. @order \O(`a.size`) @allow */
static int ACC_(compare)(const PAC_(box) *const a, const PAC_(box) *const b) {
	const PAC_(array) *aa, *bb;
	PAC_(enum) *ad, *bd, *end;
	int diff;
	/* Null counts as `-\infty`. */
	if(!a) return b ? -1 : 0;
	else if(!b) return 1;
	aa = PAC_(b2a_c)(a), bb = PAC_(b2a_c)(b), assert(aa && bb);
	if(aa->size > bb->size) {
		for(ad = aa->data, bd = bb->data, end = bd + bb->size; bd < end;
			ad++, bd++) if((diff = PACC_(compare)(ad, bd))) return diff;
		return 1;
	} else {
		for(ad = a->data, bd = b->data, end = ad + a->size; ad < end;
			ad++, bd++) if((diff = PACC_(compare)(ad, bd))) return diff;
		return -(aa->size != bb->size);
	}
}

/** <src/array_coda.h>: `box` should be partitioned true/false with less-then
 `value`. @return The first index of `a` that is not less than `value`.
 @order \O(log `a.size`) @allow */
static size_t ACC_(lower_bound)(const PAC_(box) *const box,
	const PAC_(enum) *const value) {
	const PAC_(array) *a = PAC_(b2a_c)(box);
	size_t low = 0, high = a->size, mid;
	assert(box && a && value);
	while(low < high)
		if(PACC_(compare)(value, a->data + (mid = low + (high - low) / 2)) <= 0)
			high = mid;
		else
			low = mid + 1;
	return low;
}

/** <src/array_coda.h>: `box` should be partitioned false/true with greater-than or
 equal-to <typedef:<PAC>type> `value`. @return The first index of `box` that is
 greater than `value`. @order \O(log `a.size`) @allow */
static size_t ACC_(upper_bound)(const PAC_(box) *const box,
	const PAC_(enum) *const value) {
	const PAC_(array) *a = PAC_(b2a_c)(box);
	size_t low = 0, high = a->size, mid;
	assert(box && a && value);
	while(low < high) if(PACC_(compare)(value, a->data
		+ (mid = low + ((high - low) >> 1))) >= 0) low = mid + 1;
		else high = mid;
	return low;
}

/** <src/array_coda.h>: Copies `value` at the upper bound of a sorted `box`.
 @return Success. @order \O(`a.size`) @throws[realloc, ERANGE] @allow */
static int ACC_(insert_after)(PAC_(box) *const box,
	const PAC_(enum) *const value) {
	PAC_(array) *a = PAC_(b2a)(box);
	size_t bound;
	assert(box && a && value);
	bound = ACC_(upper_bound)(a, value);
	if(!A_(array_new)(a)) return 0; /* @fixme Reference to array. */
	memmove(a->data + bound + 1, a->data + bound,
		sizeof *a->data * (a->size - bound - 1));
	memcpy(a->data + bound, value, sizeof *value);
	return 1;
}

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PACC_(vcompar)(const void *const a, const void *const b)
	{ return PACC_(compare)(a, b); }

/** <src/array_coda.h>: Sorts `box` by `qsort`.
 @order \O(`a.size` \log `box.size`) @allow */
static void ACC_(sort)(PAC_(box) *const box) {
	const PAC_(array) *a = PAC_(b2a_c)(box);
	assert(box && a);
	qsort(a->data, a->size, sizeof *a->data, &PACC_(vcompar));
}

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PACC_(vrevers)(const void *const a, const void *const b)
	{ return PACC_(compare)(b, a); }

/** <src/array_coda.h>: Sorts `box` in reverse by `qsort`.
 @order \O(`a.size` \log `a.size`) @allow */
static void ACC_(reverse)(PAC_(box) *const box) {
	const PAC_(array) *a = PAC_(b2a_c)(box);
	assert(box && a);
	qsort(a->data, a->size, sizeof *a->data, &PACC_(vrevers));
}

/** !compare(`a`, `b`) == equals(`a`, `b`).
 @implements <typedef:<PAC>bipredicate_fn> */
static int PACC_(is_equal)(const PAC_(enum) *const a, const PAC_(enum) *const b)
	{ return !PACC_(compare)(a, b); }

#else /* compare --><!-- is equal */

/* Check that `BOX_IS_EQUAL` is a function implementing
 <typedef:<PAC>bipredicate_fn>. */
static const PAC_(bipredicate_fn) PACC_(is_equal) = (BOX_IS_EQUAL);

#endif /* is equal --> */

/** <src/array_coda.h> @return If `a` piecewise equals `b`, which both can be null.
 @order \O(`size`) @allow */
static int ACC_(is_equal)(const PAC_(box) *const a, const PAC_(box) *const b)
{
	const PAC_(array) *aa, *bb;
	const PAC_(enum) *ad, *bd, *end;
	if(!a) return !b;
	if(!b) return 0;
	aa = PAC_(b2a_c)(a), bb = PAC_(b2a_c)(a), assert(aa && bb);
	if(aa->size != bb->size) return 0;
	for(ad = aa->data, bd = bb->data, end = ad + aa->size; ad < end; ad++, bd++)
		if(!PACC_(is_equal)(ad, bd)) return 0;
	return 1;
}

/** <src/array_coda.h>: Removes consecutive duplicate elements in `box`.
 @param[merge] Controls surjection. Called with duplicate elements, if false
 `(x, y)->(x)`, if true `(x,y)->(y)`. More complex functions, `(x, y)->(x+y)`
 can be simulated by mixing the two in the value returned. Can be null: behaves
 like false. @order \O(`a.size` \times `merge`) @allow */
static void ACC_(unique_merge)(PAC_(box) *const box,
	const PAC_(biaction_fn) merge) {
	PAC_(array) *a = PAC_(b2a)(box);
	size_t target, from, cursor, choice, next, move;
	const size_t last = a->size;
	int is_first, is_last;
	assert(box && a);
	for(target = from = cursor = 0; cursor < last; cursor += next) {
		/* Bijective `[from, cursor)` is moved lazily. */
		for(choice = 0, next = 1; cursor + next < last && PAC_(is_equal)(a->data
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

/** <src/array_coda.h>: Removes consecutive duplicate elements in `a`.
 @order \O(`a.size`) @allow */
static void ACC_(unique)(PAC_(box) *const a) { ACC_(unique_merge)(a, 0); }

static void PACC_(unused_compare_coda)(void);
static void PACC_(unused_compare)(void) {
#ifdef BOX_COMPARE /* <!-- compare */
	ACC_(compare)(0, 0); ACC_(lower_bound)(0, 0); ACC_(upper_bound)(0, 0);
	ACC_(insert_after)(0, 0); ACC_(sort)(0); ACC_(reverse)(0);
#endif /* compare --> */
	ACC_(is_equal)(0, 0); ACC_(unique_merge)(0, 0); ACC_(unique)(0);
	PACC_(unused_compare_coda)(); }
static void PACC_(unused_compare_coda)(void) { PACC_(unused_compare)(); }

#ifdef BOX_COMPARE
#undef BOX_COMPARE
#endif
#ifdef BOX_IS_EQUAL
#undef BOX_IS_EQUAL
#endif
#ifdef BOX_COMPARE_NAME
#undef BOX_COMPARE_NAME
#endif
/*#undef AC_C_
#undef PACC_ Need for tests. */

#endif /* compare/is equal --> */

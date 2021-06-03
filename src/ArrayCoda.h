/* 2016 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT). Intended for use under
 `array.h`, this file has much more features that usually are not needed. */

/**
 @param[ARRAY_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[ARRAY_TO_STRING_NAME, ARRAY_TO_STRING]
 To string trait contained in <ToString.h>; `<A>` that satisfies `C` naming
 conventions when mangled and function implementing <typedef:<PA>to_string_fn>.
 There can be multiple to string traits, but only one can omit
 `ARRAY_TO_STRING_NAME`.

 @param[ARRAY_TEST]
 To string trait contained in <../test/ArrayTest.h>; optional unit testing
 framework using `assert`. Can only be defined once _per_ array. Must be
 defined equal to a (random) filler function, satisfying
 <typedef:<PT>action_fn>. Output will be shown with the to string trait in
 which it's defined; provides tests for the base code and all later traits.

 @param[ARRAY_COMPARABLE_NAME, ARRAY_IS_EQUAL, ARRAY_COMPARE]
 Comparable trait; `<C>` that satisfies `C` naming conventions when mangled
 and a function implementing, for `ARRAY_IS_EQUAL` <typedef:<PT>bipredicate_fn>
 that establishes an equivalence relation, or for `ARRAY_COMPARE`
 <typedef:<PT>compare_fn> that establishes a total order. There can be multiple
 comparable traits, but only one can omit `ARRAY_COMPARABLE_NAME`. */

#if !defined(ARRAY_NAME) || !defined(ARRAY_TYPE) || !defined(ARRAY_CODA) \
	|| !defined(CAT) || !defined(CAT_) || !defined(T_) || !defined(PT_)
#error Include `array_coda.h` from within `array.h`.
#endif
#if defined(ARRAY_TO_STRING_NAME) || defined(ARRAY_TO_STRING)
#define ARRAY_TO_STRING_TRAIT 1
#else
#define ARRAY_TO_STRING_TRAIT 0
#endif
#if defined(ARRAY_COMPARABLE_NAME) || defined(ARRAY_COMPARE) \
	|| defined(ARRAY_IS_EQUAL)
#define ARRAY_COMPARABLE_TRAIT 1
#else
#define ARRAY_COMPARABLE_TRAIT 0
#endif
#define ARRAY_TRAITS ARRAY_TO_STRING_TRAIT + ARRAY_COMPARABLE_TRAIT
#if ARRAY_TRAITS > 1
#error Only one trait per include is allowed; use ARRAY_EXPECT_TRAIT.
#endif
#if ARRAY_TRAITS != 0 && (!defined(T_) || !defined(CAT) || !defined(CAT_))
#error T_ or CAT_? not yet defined; use ARRAY_EXPECT_TRAIT?
#endif
#if (ARRAY_TRAITS == 0) && defined(ARRAY_TEST)
#error ARRAY_TEST must be defined in ARRAY_TO_STRING trait.
#endif
#if defined(ARRAY_TO_STRING_NAME) && !defined(ARRAY_TO_STRING)
#error ARRAY_TO_STRING_NAME requires ARRAY_TO_STRING.
#endif
#if defined(ARRAY_COMPARABLE_NAME) \
	&& ((!defined(ARRAY_COMPARE) && !defined(ARRAY_IS_EQUAL)) || \
	(defined(ARRAY_COMPARE) && defined(ARRAY_IS_EQUAL)))
#error ARRAY_COMPARABLE_NAME requires ARRAY_COMPARE or ARRAY_IS_EQUAL not both.
#endif


#if ARRAY_TRAITS == 0 /* <!-- base code */

/** Operates by side-effects. */
typedef void (*PT_(action_fn))(PT_(type) *);

/** Returns a boolean given read-only `<T>`. */
typedef int (*PT_(predicate_fn))(const PT_(type) *);

/** Returns a boolean given two read-only `<T>`. */
typedef int (*PT_(bipredicate_fn))(const PT_(type) *, const PT_(type) *);

/** Returns a boolean given two `<T>`. */
typedef int (*PT_(biproject_fn))(PT_(type) *, PT_(type) *);

/** Three-way comparison on a totally order set; returns an integer value less
 then, equal to, greater then zero, if `a < b`, `a == b`, `a > b`,
 respectively. */
typedef int (*PT_(compare_fn))(const PT_(type) *a, const PT_(type) *b);

/** @return Converts `i` to an index in `a` from [0, `a.size`]. Negative values
 are implicitly plus `a.size`. @order \Theta(1) @allow */
static size_t T_(array_clip)(const struct T_(array) *const a, const long i) {
	/* `SIZE_MAX` is `C99`; assumes two's-compliment, not many hw-tests. */
	assert(a && (size_t)-1 >= (size_t)LONG_MAX
		&& (unsigned long)((size_t)-1) >= LONG_MAX);
	return i < 0
		? (size_t)-i >= a->size ? 0 : a->size + i
		: (size_t)i > a->size ? a->size : (size_t)i;
}

/** `a` indices [`i0`, `i1`) will be replaced with a copy of `b`.
 @param[b] Can be null, which acts as empty.
 @return Success. @throws[realloc, ERANGE] */
static int T_(array_splice)(struct T_(array) *const a, const size_t i0,
	const size_t i1, const struct T_(array) *const b) {
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

/** Copies `b`, which can be null, to the back of `a`.
 @return Success. @throws[realloc, ERANGE] */
static int T_(array_copy)(struct T_(array) *const a,
	const struct T_(array) *const b)
	{ return T_(array_splice)(a, a->size, a->size, b); }

/** For all elements of `b`, calls `copy`, and if true, lazily copies the
 elements to `a`. `a` and `b` can not be the same but `b` can be null.
 @order \O(`b.size` \times `copy`) @throws[ERANGE, realloc] @allow */
static int T_(array_copy_if)(struct T_(array) *const a,
	const PT_(predicate_fn) copy, const struct T_(array) *const b) {
	PT_(type) *i, *fresh;
	const PT_(type) *end, *rise = 0;
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
			if(!(fresh = T_(array_append)(a, add = i - rise))) return 0;
			memcpy(fresh, rise, sizeof *fresh * add);
			rise = 0;
		}
	}
	if(rise) { /* Delayed copy. */
		assert(!difcpy && rise < i);
		if(!(fresh = T_(array_append)(a, add = i - rise))) return 0;
		memcpy(fresh, rise, sizeof *fresh * add);
	}
	return 1;
}

/** For all elements of `a`, calls `keep`, and if false, lazy deletes that
 item, calling `destruct` if not-null.
 @order \O(`a.size` \times `keep` \times `destruct`) @allow */
static void T_(array_keep_if)(struct T_(array) *const a,
	const PT_(predicate_fn) keep, const PT_(action_fn) destruct) {
	PT_(type) *erase = 0, *t;
	const PT_(type) *retain = 0, *end;
	int keep0 = 1, keep1 = 0;
	assert(a && keep);
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
 @order \O(`a.size` \times `predicate`) @allow */
static void T_(array_trim)(struct T_(array) *const a,
	const PT_(predicate_fn) predicate) {
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
static void T_(array_each)(struct T_(array) *const a,
	const PT_(action_fn) action) {
	PT_(type) *i, *i_end;
	assert(a && action);
	for(i = a->data, i_end = i + a->size; i < i_end; i++) action(i);
}

/** Iterates through `a` and calls `action` on all the elements for which
 `predicate` returns true. The topology of the list should not change while in
 this function. @order \O(`a.size` \times `predicate` \times `action`) @allow */
static void T_(array_if_each)(struct T_(array) *const a,
	const PT_(predicate_fn) predicate, const PT_(action_fn) action) {
	PT_(type) *i, *i_end;
	assert(a && predicate && action);
	for(i = a->data, i_end = i + a->size; i < i_end; i++)
		if(predicate(i)) action(i);
}

/** Iterates through `a` and calls `predicate` until it returns true.
 @return The first `predicate` that returned true, or, if the statement is
 false on all, null. @order \O(`a.size` \times `predicate`) @allow */
static PT_(type) *T_(array_any)(const struct T_(array) *const a,
	const PT_(predicate_fn) predicate) {
	PT_(type) *i, *i_end;
	if(!a || !predicate) return 0;
	for(i = a->data, i_end = i + a->size; i < i_end; i++)
		if(predicate(i)) return i;
	return 0;
}

/** Contains all iteration parameters. */
struct PT_(iterator);
struct PT_(iterator) { const struct T_(array) *a; size_t i; };

/** Loads `a` into `it`. @implements begin */
static void PT_(begin)(struct PT_(iterator) *const it,
	const struct T_(array) *const a) { assert(it && a), it->a = a, it->i = 0; }

/** Advances `it`. @implements next */
static const PT_(type) *PT_(next)(struct PT_(iterator) *const it) {
	assert(it && it->a);
	return it->i < it->a->size ? it->a->data + it->i++ : 0;
}

#if defined(ITERATE) || defined(ITERATE_BOX) || defined(ITERATE_TYPE) \
	|| defined(ITERATE_BEGIN) || defined(ITERATE_NEXT)
#error Unexpected ITERATE*.
#endif

#define ITERATE struct PT_(iterator)
#define ITERATE_BOX struct T_(array)
#define ITERATE_TYPE PT_(type)
#define ITERATE_BEGIN PT_(begin)
#define ITERATE_NEXT PT_(next)

static void PT_(unused_coda_coda)(void);
static void PT_(unused_coda)(void) {
	T_(array_)(0); T_(array_clip)(0, 0); T_(array_append_at)(0, 0, 0);
	T_(array_new)(0); T_(array_shrink)(0); T_(array_remove)(0, 0);
	T_(array_lazy_remove)(0, 0); T_(array_clear)(0); T_(array_peek)(0);
	T_(array_pop)(0); T_(array_splice)(0, 0, 0, 0); T_(array_copy)(0, 0);
	T_(array_keep_if)(0, 0, 0); T_(array_copy_if)(0, 0, 0);
	T_(array_trim)(0, 0); T_(array_each)(0, 0); T_(array_if_each)(0, 0, 0);
	T_(array_any)(0, 0); PT_(begin)(0, 0); PT_(next)(0);
	PT_(unused_base_coda)();
}
static void PT_(unused_coda_coda)(void) { PT_(unused_coda)(); }


#elif defined(ARRAY_TO_STRING) /* base code --><!-- to string trait */


#ifdef ARRAY_TO_STRING_NAME /* <!-- name */
#define A_(thing) CAT(T_(array), CAT(ARRAY_TO_STRING_NAME, thing))
#else /* name --><!-- !name */
#define A_(thing) CAT(T_(array), thing)
#endif /* !name --> */
#define TO_STRING ARRAY_TO_STRING
#include "ToString.h" /** \include */

#if !defined(ARRAY_TEST_BASE) && defined(ARRAY_TEST) /* <!-- test */
#define ARRAY_TEST_BASE /* Only one instance of base tests. */
#include "../test/TestArray.h" /** \include */
#endif /* test --> */

#undef A_
#undef ARRAY_TO_STRING
#ifdef ARRAY_TO_STRING_NAME
#undef ARRAY_TO_STRING_NAME
#endif


#else /* to string trait --><!-- comparable trait */


#ifdef ARRAY_COMPARABLE_NAME /* <!-- name */
#define PTC_(thing) CAT(PT_(thing), ARRAY_COMPARABLE_NAME)
#define T_C_(thing1, thing2) CAT(T_(thing1), CAT(ARRAY_COMPARABLE_NAME, thing2))
#else /* name --><!-- !name */
#define PTC_(thing) CAT(PT_(thing), anonymous)
#define T_C_(thing1, thing2) CAT(T_(thing1), thing2)
#endif /* !name --> */

#ifdef ARRAY_COMPARE /* <!-- compare */

/* Check that `ARRAY_COMPARE` is a function implementing
 <typedef:<PT>compare>. */
static const PT_(compare_fn) PTC_(compare) = (ARRAY_COMPARE);

/** Lexagraphically compares `a` to `b`, which both can be null.
 @return { `a < b`: negative, `a == b`: zero, `a > b`: positive }.
 @order \O(`a.size`) @allow */
static int T_C_(array, compare)(const struct T_(array) *const a,
	const struct T_(array) *const b) {
	PT_(type) *ia, *ib, *end;
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
static size_t T_C_(array, lower_bound)(const struct T_(array) *const a,
	const PT_(type) *const value) {
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
static size_t T_C_(array, upper_bound)(const struct T_(array) *const a,
	const PT_(type) *const value) {
	size_t low = 0, high = a->size, mid;
	assert(a && value);
	while(low < high) if(PTC_(compare)(value, a->data
		+ (mid = low + ((high - low) >> 1))) >= 0) low = mid + 1;
		else high = mid;
	return low;
}

/** Copies `datum` at the lower bound of a sorted `a`.
 @return Success. @order \O(`a.size`) @throws[realloc, ERANGE] */
static int T_C_(array, insert)(struct T_(array) *const a,
	const PT_(type) *const datum) {
	size_t bound;
	assert(a && datum);
	bound = T_C_(array, lower_bound)(a, datum);
	if(!T_(array_new)(a)) return 0;
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
static void T_C_(array, sort)(struct T_(array) *const a)
	{ assert(a), qsort(a->data, a->size, sizeof *a->data, PTC_(vcompar)); }

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PTC_(vrevers)(const void *const a, const void *const b)
	{ return PTC_(compare)(b, a); }

/** Sorts `a` in reverse by `qsort` on `ARRAY_COMPARE`.
 @order \O(`a.size` \log `a.size`) @allow */
static void T_C_(array, reverse)(struct T_(array) *const a)
	{ assert(a), qsort(a->data, a->size, sizeof *a->data, PTC_(vrevers)); }

/** !compare(`a`, `b`) == equals(`a`, `b`) for not `ARRAY_IS_EQUAL`.
 @implements <PT>bipredicate */
static int PTC_(is_equal)(const void *const a, const void *const b)
	{ return !PTC_(compare)(a, b); }

#else /* compare --><!-- is equal */

/* Check that `ARRAY_IS_EQUAL` is a function implementing
 <typedef:<PT>bipredicate>. */
static const PT_(bipredicate) PTC_(is_equal) = (ARRAY_IS_EQUAL);

#endif /* is equal --> */

/** @return If `a` piecewise equals `b`, which both can be null.
 @order \O(`size`) */
static int T_C_(array, is_equal)(const struct T_(array) *const a,
	const struct T_(array) *const b) {
	PT_(type) *ia, *ib, *end;
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
static void T_C_(array, merge_unique)(struct T_(array) *const a,
	const PT_(biproject_fn) merge) {
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

/** Removes consecutive duplicate elements in `a`. @order \O(`a.size`) @allow */
static void T_C_(array, unique)(struct T_(array) *const a)
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
#include "../test/TestArray.h"
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
#undef ITERATE
#undef ITERATE_BOX
#undef ITERATE_TYPE
#undef ITERATE_BEGIN
#undef ITERATE_NEXT
#endif /* !trait --> */

#undef ARRAY_TO_STRING_TRAIT
#undef ARRAY_COMPARABLE_TRAIT
#undef ARRAY_TRAITS

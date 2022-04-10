/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Compare trait

 Interface (defined by `BOX_`, `BOX`, and `BOX_CURSOR`):

 \* `int <BOX>is_cursor(<PCMP>cursor_c)`;
 \* `struct <BOX>iterator <BOX>begin(const <PCMP>box *)`;
 \* `<PCMP>cursor <BOX>next(struct <BOX>iterator *)`;
 \* `size_t <BOX>size(const <PCMP>box *)`;
 \* `<PCMP>cursor <BOX>append(<PCMP>box *, size_t)`.

 @param[CMP_(n)]
 A one-argument macro producing a name that is responsible for the name of the
 functions. Should be something like `CMP_(x) -> foo_widget_x`. The caller is
 responsible for undefining `CMP_`.

 @param[COMPARE_IS_EQUAL, COMPARE]
 Function implementing <typedef:<PCMP>is_equal_fn> or
 <typedef:<PCMP>compare_fn>. One is required, (but not two.)

 @std C89 */

#if !defined(BOX_) || !defined(BOX) || !defined(BOX_CURSOR) || !defined(CMP_) \
	|| !(defined(COMPARE_IS_EQUAL) ^ defined(COMPARE))
#error Unexpected preprocessor symbols.
#endif

#ifndef COMPARE_H /* <!-- idempotent */
#define COMPARE_H
#include <stddef.h>
#include <limits.h>
#if defined(TO_STRING_CAT_) || defined(TO_STRING_CAT) || defined(PCMP_)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define COMPARE_CAT_(n, m) n ## _ ## m
#define COMPARE_CAT(n, m) COMPARE_CAT_(n, m)
#define PCMP_(n) COMPARE_CAT(compare, CMP_(n))
#endif /* idempotent --> */

#ifndef COMPARE_ONCE /* <!-- once */
#define COMPARE_ONCE
/** <src/compare.h>: Returns a boolean given two read-only elements. */
typedef int (*PCMP_(bipredicate_fn))(const PCMP_(cursor), const PCMP_(cursor));
/** <src/compare.h>: Three-way comparison on a totally order set; returns an
 integer value less than, equal to, greater than zero, if `a < b`, `a == b`,
 `a > b`, respectively. */
typedef int (*PCMP_(compare_fn))(const PCMP_(cursor) a, const PCMP_(cursor) b);
/** <src/compare.h>: Returns a boolean given two modifiable arguments. */
typedef int (*PCMP_(biaction_fn))(PCMP_(cursor), PCMP_(cursor));
typedef BOX PCMP_(box);
typedef BOX_CURSOR PCMP_(cursor);
typedef const BOX_CURSOR PCMP_(cursor_c);
#endif /* once --> */


#ifdef COMPARE /* <!-- compare */


/* Check that `BOX_COMPARE` is a function implementing
 <typedef:<PCMP>compare_fn>. */
static const PCMP_(compare_fn) PCMP_(compare) = (BOX_COMPARE);

/** <src/compare.h>: Lexicographically compares `a` to `b`. Both can be null,
 with null values before everything. @return `a < b`: negative; `a == b`: zero;
 `a > b`: positive. @order \O(`|a|` & `|b|`) @allow */
static int CMP_(compare)(const PCMP_(box) *const a, const PCMP_(box) *const b) {
	struct BOX_(iterator) ia = begin(a);
	PCMP_(cursor_c) *ad, *bd, *end;
	int diff;
	/* Null counts as `-\infty`. */
	if(!a) return b ? -1 : 0;
	else if(!b) return 1;

	if(BOX_(size)(a) > BOX_(size)(b)) {
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
static size_t ACC_(lower_bound)(const PCMP_(box) *const box,
	const PCMP_(enum) *const value) {
	const PCMP_(array) *a = PCMP_(b2a_c)(box);
	size_t low = 0, high = a->size, mid;
	assert(box && a && value);
	while(low < high)
		if(PACC_(compare)(value, a->data + (mid = low + (high - low) / 2)) <= 0)
			high = mid;
		else
			low = mid + 1;
	return low;
}

/** <src/array_coda.h>: `box` should be partitioned false/true with
 greater-than or equal-to <typedef:<PAC>enum> `value`. @return The first index
 of `box` that is greater than `value`. @order \O(log `a.size`) @allow */
static size_t ACC_(upper_bound)(const PCMP_(box) *const box,
	const PCMP_(enum) *const value) {
	const PCMP_(array) *a = PCMP_(b2a_c)(box);
	size_t low = 0, high = a->size, mid;
	assert(box && a && value);
	while(low < high) if(PACC_(compare)(value, a->data
		+ (mid = low + ((high - low) >> 1))) >= 0) low = mid + 1;
		else high = mid;
	return low;
}

/** <src/array_coda.h>: Copies `value` at the upper bound of a sorted `box`.
 @return Success. @order \O(`a.size`) @throws[realloc, ERANGE] @allow */
static int ACC_(insert_after)(PCMP_(box) *const box,
	const PCMP_(enum) *const value) {
	PCMP_(array) *a = PCMP_(b2a)(box);
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
static void ACC_(sort)(PCMP_(box) *const box) {
	const PCMP_(array) *a = PCMP_(b2a_c)(box);
	assert(box && a);
	qsort(a->data, a->size, sizeof *a->data, &PACC_(vcompar));
}

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PACC_(vrevers)(const void *const a, const void *const b)
	{ return PACC_(compare)(b, a); }

/** <src/array_coda.h>: Sorts `box` in reverse by `qsort`.
 @order \O(`a.size` \log `a.size`) @allow */
static void ACC_(reverse)(PCMP_(box) *const box) {
	const PCMP_(array) *a = PCMP_(b2a_c)(box);
	assert(box && a);
	qsort(a->data, a->size, sizeof *a->data, &PACC_(vrevers));
}

/** !compare(`a`, `b`) == equals(`a`, `b`).
 @implements <typedef:<PAC>bipredicate_fn> */
static int PACC_(is_equal)(const PCMP_(enum) *const a, const PCMP_(enum) *const b)
	{ return !PACC_(compare)(a, b); }

#else /* compare --><!-- is equal */

/* Check that `BOX_IS_EQUAL` is a function implementing
 <typedef:<PAC>bipredicate_fn>. */
static const PCMP_(bipredicate_fn) PACC_(is_equal) = (BOX_IS_EQUAL);

#endif /* is equal --> */

/** <src/array_coda.h> @return If `a` piecewise equals `b`, which both can be null.
 @order \O(`size`) @allow */
static int ACC_(is_equal)(const PCMP_(box) *const a, const PCMP_(box) *const b)
{
	const PCMP_(array) *aa, *bb;
	const PCMP_(enum) *ad, *bd, *end;
	if(!a) return !b;
	if(!b) return 0;
	aa = PCMP_(b2a_c)(a), bb = PCMP_(b2a_c)(a), assert(aa && bb);
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
static void ACC_(unique_merge)(PCMP_(box) *const box,
	const PCMP_(biaction_fn) merge) {
	PCMP_(array) *a = PCMP_(b2a)(box);
	size_t target, from, cursor, choice, next, move;
	const size_t last = a->size;
	int is_first, is_last;
	assert(box && a);
	for(target = from = cursor = 0; cursor < last; cursor += next) {
		/* Bijective `[from, cursor)` is moved lazily. */
		for(choice = 0, next = 1; cursor + next < last && PCMP_(is_equal)(a->data
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
static void ACC_(unique)(PCMP_(box) *const a) { ACC_(unique_merge)(a, 0); }

static void PACC_(unused_compare_coda)(void);
static void PACC_(unused_compare)(void) {
#ifdef BOX_COMPARE /* <!-- compare */
	ACC_(compare)(0, 0); ACC_(lower_bound)(0, 0); ACC_(upper_bound)(0, 0);
	ACC_(insert_after)(0, 0); ACC_(sort)(0); ACC_(reverse)(0);
#endif /* compare --> */
	ACC_(is_equal)(0, 0); ACC_(unique_merge)(0, 0); ACC_(unique)(0);
	PACC_(unused_compare_coda)(); }
static void PACC_(unused_compare_coda)(void) { PACC_(unused_compare)(); }

#endif /*0*/

#ifdef BOX_COMPARE
#undef BOX_COMPARE
#endif
#ifdef BOX_IS_EQUAL
#undef BOX_IS_EQUAL
#endif
#ifdef BOX_COMPARE_NAME
#undef BOX_COMPARE_NAME
#endif
/*#undef CMP_C_
#undef PACC_ Need for tests. */

#endif /* compare/is equal --> */

/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Compare trait

 Interface (defined by `BOX_`, `BOX`, and `BOX_CURSOR`):

 \* `int <BOX>is_cursor(<PCMP>cursor_c)`;
 \* (maybe) `size_t <BOX>size(const <PCMP>box *)`;
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

 @param[COMPARE_QSORT]
 A contiguous array of elements, and a pointer <typedef:<BOX>cursor>, allows
 `qsort`, otherwise, we have to do merge sort, which is stable, but more slow.

 @std C89 */

#if !defined(BOX_) || !defined(BOX) || !defined(BOX_FORWARD) \
	|| !defined(CMP_) || !(defined(COMPARE_IS_EQUAL) ^ defined(COMPARE))
#error Unexpected preprocessor symbols.
#endif

#ifndef COMPARE_H /* <!-- idempotent */
#define COMPARE_H
#include <stddef.h>
#include <limits.h>
#if defined(COMPARE_CAT_) || defined(COMPARE_CAT) || defined(PCMP_)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define COMPARE_CAT_(n, m) n ## _ ## m
#define COMPARE_CAT(n, m) COMPARE_CAT_(n, m)
#define PCMP_(n) COMPARE_CAT(compare, CMP_(n))
#endif /* idempotent --> */

typedef BOX PCMP_(box);
typedef BOX_FORWARD PCMP_(element);
typedef const BOX_FORWARD PCMP_(element_c);

/** <src/compare.h>: Returns a boolean given two read-only elements. */
typedef int (*PCMP_(bipredicate_fn))(const PCMP_(element), const PCMP_(element));
/** <src/compare.h>: Three-way comparison on a totally order set; returns an
 integer value less than, equal to, greater than zero, if `a < b`, `a == b`,
 `a > b`, respectively. */
typedef int (*PCMP_(compare_fn))(const PCMP_(element_c) a,
	const PCMP_(element_c) b);
/** <src/compare.h>: Returns a boolean given two modifiable arguments. */
typedef int (*PCMP_(biaction_fn))(PCMP_(element), PCMP_(element));


#ifdef COMPARE /* <!-- compare */


/* Check that `BOX_COMPARE` is a function implementing
 <typedef:<PCMP>compare_fn>. */
static const PCMP_(compare_fn) PCMP_(compare) = (COMPARE);

/** <src/compare.h>: Lexicographically compares `a` to `b`. Both can be null,
 with null values before everything. @return `a < b`: negative; `a == b`: zero;
 `a > b`: positive. @order \O(`|a|` & `|b|`) @allow */
static int CMP_(compare)(const PCMP_(box) *const a, const PCMP_(box) *const b) {
	struct BOX_(forward) ia = BOX_(forward_begin)(a),
		ib = BOX_(forward_begin)(b);
	for( ; ; ) {
		const PCMP_(element_c) x = BOX_(forward_next)(&ia),
			y = BOX_(forward_next)(&ib);
		int diff;
		if(!BOX_(is_element)(x)) return BOX_(is_element)(y) ? -1 : 0;
		else if(!BOX_(is_element)(y)) return 1;
		if(diff = PCMP_(compare)(x, y)) return diff;
	}
}

#ifdef BOX_ACCESS /* <!-- access */

/** <src/compare.h>: `box` should be partitioned true/false with less-then
 `value`. @return The first index of `a` that is not less than `cursor`.
 @order \O(log `a.size`) @allow */
static size_t CMP_(lower_bound)(const PCMP_(box) *const box,
	const PCMP_(element_c) cursor) {
	size_t low = 0, high = BOX_(size)(box), mid;
	while(low < high)
		if(PCMP_(compare)(cursor, BOX_(at)(box,
			mid = low + (high - low) / 2)) <= 0) high = mid;
		else low = mid + 1;
	return low;
}

/** <src/compare.h>: `box` should be partitioned false/true with
 greater-than or equal-to <typedef:<PAC>enum> `value`. @return The first index
 of `box` that is greater than `cursor`. @order \O(log `a.size`) @allow */
static size_t CMP_(upper_bound)(const PCMP_(box) *const box,
	const PCMP_(element_c) cursor) {
	size_t low = 0, high = BOX_(size)(box), mid;
	while(low < high)
		if(PCMP_(compare)(cursor, BOX_(at)(box,
			mid = low + (high - low) / 2)) >= 0) low = mid + 1;
		else high = mid;
	return low;
}

#if 0
/** <src/compare.h>: Copies `value` at the upper bound of a sorted `box`.
 @return Success. @order \O(`a.size`) @throws[realloc, ERANGE] @allow
 Don't know how we would do this... */
static int ACC_(insert_after)(PCMP_(box) *const box,
	const PCMP_(element) *const value) {
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
#endif

#endif

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PCMP_(vcompar)(const void *const a, const void *const b)
	{ return PCMP_(compare)(a, b); }

/** <src/compare.h>: Sorts `box` by `qsort`. @fixme: must be contiguous!
 (We have no such non-contiguous boxes that need sorting . . . )
 @order \O(|`a`| \log |`b`|) @allow */
static void CMP_(sort)(PCMP_(box) *const box) {
	const size_t size = BOX_(size)(box);
	struct BOX_(iterator) it;
	PCMP_(element) first;
	if(!size) return;
	it = BOX_(begin)(box);
	first = BOX_(next)(&it);
	if(!BOX_(is_element)(first)) return; /* That was weird. */
	/* FIXME: sizeof is a problem, and more generally this will not work if it's
	 not contiguous; have a parameter that chooses between `qsort` and merge
	 sort (not merged into this code.) */
	qsort(first, size, sizeof *first, &PCMP_(vcompar));
}

#if 0

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PACC_(vrevers)(const void *const a, const void *const b)
	{ return PACC_(compare)(b, a); }

/** <src/compare.h>: Sorts `box` in reverse by `qsort`.
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

#endif /* --------------------------------------> */

#else /* compare --><!-- is equal */

/* Check that `BOX_IS_EQUAL` is a function implementing
 <typedef:<PAC>bipredicate_fn>. */
/*static const PCMP_(bipredicate_fn) PACC_(is_equal) = (BOX_IS_EQUAL);*/

#endif /* is equal --> */

#if 0 /* <!-----------------------------------------*/
/** <src/compare.h> @return If `a` piecewise equals `b`, which both can be null.
 @order \O(`size`) @allow */
static int CMP_(is_equal)(const PCMP_(box) *const a,
	const PCMP_(box) *const b) {
	struct BOX_(iterator) ia = BOX_(begin)(a), ib = BOX_(begin)(b);
	for( ; ; ) {
		const PCMP_(element_c) x = BOX_(next)(&ia), y = BOX_(next)(&ib);
		if(!BOX_(is_cursor)(x)) return !BOX_(is_cursor)(y);
		else if(!BOX_(is_cursor)(y)) return 0;
		if(!PCMP_(is_equal)(x, y)) return 0;
	}
	return 1;
}

/** <src/compare.h>: Removes consecutive duplicate elements in `box`.
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

/** <src/compare.h>: Removes consecutive duplicate elements in `a`.
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

#endif /* ---------------------------------------------> */
/* compare/is equal --> */

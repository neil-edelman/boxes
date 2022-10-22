/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Compare trait

 Interface defined by box.

 @param[COMPARE_IS_EQUAL, COMPARE]
 Function implementing <typedef:<PCMP>is_equal_fn> or
 <typedef:<PCMP>compare_fn>. One is required, (but not two.)

 @std C89 */

#if !defined(BOX_TYPE) || !defined(BOX_CONTENT) || !defined(BOX_) \
	|| !defined(BOX_MAJOR_NAME) || !defined(BOX_MINOR_NAME) \
	|| defined(STR_) || defined(STRLABEL_)
#error Unexpected preprocessor symbols.
#endif
#if !defined(BOX_) || !defined(BOX) || !defined(BOX_CONTENT_C) \
	|| !defined(CMP_) || !(defined(COMPARE_IS_EQUAL) ^ defined(COMPARE)) \
	|| defined(BOX_CONTIGUOUS) && !defined(BOX_CONTENT)
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
typedef BOX_CONTENT_C PCMP_(element_c);
#ifdef BOX_CONTENT
typedef BOX_CONTENT PCMP_(element);
#endif

/** <src/compare.h>: Returns a boolean given two read-only elements. */
typedef int (*PCMP_(bipredicate_fn))(const PCMP_(element_c),
	const PCMP_(element_c));
/** <src/compare.h>: Three-way comparison on a totally order set; returns an
 integer value less than, equal to, greater than zero, if `a < b`, `a == b`,
 `a > b`, respectively. */
typedef int (*PCMP_(compare_fn))(const PCMP_(element_c) restrict a,
	const PCMP_(element_c) restrict b);
#ifdef BOX_CONTENT
/** <src/compare.h>, `BOX_CONTENT`: Returns a boolean given two modifiable
 arguments. */
typedef int (*PCMP_(biaction_fn))(PCMP_(element) restrict,
	PCMP_(element) restrict);
#endif

#ifdef COMPARE /* <!-- compare: <typedef:<PCMP>compare_fn>. */

/* Check that `BOX_COMPARE` is a function implementing
 <typedef:<PCMP>compare_fn>. */
static const PCMP_(compare_fn) PCMP_(compare) = (COMPARE);

/** <src/compare.h>, `COMPARE`: Lexicographically compares `a` to `b`. Both can
 be null, with null values before everything.
 @return `a < b`: negative; `a == b`: zero; `a > b`: positive.
 @order \O(`|a|` & `|b|`) @allow */
static int CMP_(compare)(const PCMP_(box) *restrict const a,
	const PCMP_(box) *restrict const b) {
	struct BOX_(forward) ia = BOX_(forward)(a),
		ib = BOX_(forward)(b);
	for( ; ; ) {
		const PCMP_(element_c) x = BOX_(next_c)(&ia),
			y = BOX_(next_c)(&ib);
		int diff;
		if(!BOX_(is_element_c)(x)) return BOX_(is_element_c)(y) ? -1 : 0;
		else if(!BOX_(is_element_c)(y)) return 1;
		if(diff = PCMP_(compare)(x, y)) return diff;
	}
}

#ifdef BOX_ACCESS /* <!-- access: size, at. */

/** <src/compare.h>, `COMPARE`, `BOX_ACCESS`: `box` should be partitioned
 true/false with less-then `element`. @return The first index of `a` that is
 not less than `cursor`. @order \O(log `a.size`) @allow */
static size_t CMP_(lower_bound)(const PCMP_(box) *const box,
	const PCMP_(element_c) element) {
	size_t low = 0, high = BOX_(size)(box), mid;
	while(low < high)
		if(PCMP_(compare)(element,
			(const PCMP_(element_c))BOX_(at)(box,
			mid = low + (high - low) / 2)) <= 0) high = mid;
		else low = mid + 1;
	return low;
}

/** <src/compare.h>, `COMPARE`, `BOX_ACCESS`: `box` should be partitioned
 false/true with greater-than or equal-to `element`.
 @return The first index of `box` that is greater than `element`.
 @order \O(log |`box`|) @allow */
static size_t CMP_(upper_bound)(const PCMP_(box) *const box,
	const PCMP_(element_c) element) {
	size_t low = 0, high = BOX_(size)(box), mid;
	while(low < high)
		if(PCMP_(compare)(element, (const PCMP_(element_c))BOX_(at)(box,
			mid = low + (high - low) / 2)) >= 0) low = mid + 1;
		else high = mid;
	return low;
}

#ifdef BOX_CONTIGUOUS /* <!-- contiguous: element is pointer to array. */

/** <src/compare.h>, `COMPARE`, `BOX_CONTIGUOUS`: Copies `element` at the upper
 bound of a sorted `box`.
 @return Success. @order \O(`a.size`) @throws[realloc, ERANGE] @allow */
static int CMP_(insert_after)(PCMP_(box) *const box,
	const PCMP_(element_c) element) {
	size_t bound;
	assert(box && element);
	bound = CMP_(upper_bound)(box, element);
	if(!BOX_(append)(box, 1)) return 0;
	memmove(BOX_(at)(box, bound + 1), BOX_(at)(box, bound),
		sizeof *element * (BOX_(size)(box) - bound - 1));
	memcpy(BOX_(at)(box, bound), element, sizeof *element);
	return 1;
}

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PCMP_(vcompar)(const void *restrict const a,
	const void *restrict const b) { return PCMP_(compare)(a, b); }

/** <src/compare.h>, `COMPARE`, `BOX_CONTIGUOUS`: Sorts `box` by `qsort`,
 (which has a high-context-switching cost, but is easy.)
 @order \O(|`box`| \log |`box`|) @allow */
static void CMP_(sort)(PCMP_(box) *const box) {
	const size_t size = BOX_(size)(box);
	PCMP_(element) first;
	if(!size) return;
	first = BOX_(at)(box, 0);
	if(!BOX_(is_element)(first)) return; /* That was weird. */
	qsort(first, size, sizeof *first, &PCMP_(vcompar));
}

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PCMP_(vrevers)(const void *restrict const a,
	const void *restrict const b) { return PCMP_(compare)(b, a); }

/** <src/compare.h>, `COMPARE`, `BOX_CONTIGUOUS`: Sorts `box` in reverse by
 `qsort`. @order \O(|`box`| \log |`box`|) @allow */
static void CMP_(reverse)(PCMP_(box) *const box) {
	const size_t size = BOX_(size)(box);
	PCMP_(element) first;
	if(!size) return;
	first = BOX_(at)(box, 0);
	if(!BOX_(is_element)(first)) return; /* That was weird. */
	qsort(first, size, sizeof *first, &PCMP_(vrevers));
}

#endif /* contiguous --> */

#endif /* access --> */

/** !compare(`a`, `b`) == equals(`a`, `b`).
 (This makes `COMPARE` encompass `COMPARE_IS_EQUAL`.)
 @implements <typedef:<PCMP>bipredicate_fn> */
static int PCMP_(is_equal)(const PCMP_(element_c) restrict a,
	const PCMP_(element_c) restrict b) { return !PCMP_(compare)(a, b); }

#else /* compare --><!-- is equal */

/* Check that `COMPARE_IS_EQUAL` is a function implementing
 <typedef:<PCMP>bipredicate_fn>. */
static const PCMP_(bipredicate_fn) PCMP_(is_equal) = (COMPARE_IS_EQUAL);

#endif /* is equal --> */

/** <src/compare.h> @return If `a` piecewise equals `b`,
 which both can be null. @order \O(|`a`| & |`b`|) @allow */
static int CMP_(is_equal)(const PCMP_(box) *restrict const a,
	const PCMP_(box) *restrict const b) {
	struct BOX_(forward) ia = BOX_(forward)(a),
		ib = BOX_(forward)(b);
	for( ; ; ) {
		const PCMP_(element_c) x = BOX_(next_c)(&ia),
			y = BOX_(next_c)(&ib);
		if(!BOX_(is_element_c)(x)) return !BOX_(is_element_c)(y);
		else if(!BOX_(is_element_c)(y)) return 0;
		if(!PCMP_(is_equal)(x, y)) return 0;
	}
	return 1;
}

#ifdef BOX_CONTIGUOUS /* <!-- contiguous: (array, pointer), size, at,
 tell_size. */

/** <src/compare.h>, `BOX_CONTIGUOUS`: Removes consecutive duplicate elements
 in `box` lazily.
 @param[merge] Controls surjection. Called with duplicate elements, if false
 `(x, y)->(x)`, if true `(x,y)->(y)`. More complex functions, `(x, y)->(x+y)`
 can be simulated by mixing the two in the value returned. Can be null: behaves
 like false, always deleting the second element.
 @order \O(|`box`|) \times \O(`merge`) @allow */
static void CMP_(unique_merge)(PCMP_(box) *const box,
	const PCMP_(biaction_fn) merge) {
	size_t target, from, cursor, choice, next, move;
	const size_t last = BOX_(size)(box);
	int is_first, is_last;
	PCMP_(element) dst, src;
	assert(box);
	for(target = from = cursor = 0; cursor < last; cursor += next) {
		/* Bijective `[from, cursor)` is moved lazily. */
		for(choice = 0, next = 1; cursor + next < last; next++) {
			const PCMP_(element) a = BOX_(at)(box, cursor + choice),
				b = BOX_(at)(box, cursor + next);
			if(!PCMP_(is_equal)((const PCMP_(element_c))a,
				(const PCMP_(element_c))b)) break;
			if(merge && merge(a, b)) choice = next;
		}
		if(next == 1) continue;
		/* Must move injective `cursor + choice \in [cursor, cursor + next)`. */
		is_first = !choice;
		is_last  = (choice == next - 1);
		move = cursor - from + (size_t)is_first;
		dst = BOX_(at)(box, target), src = BOX_(at)(box, from);
		memmove(dst, src, sizeof *src * move), target += move;
		if(!is_first && !is_last) dst = BOX_(at)(box, target),
			src = BOX_(at)(box, cursor + choice),
			memcpy(dst, src, sizeof *src), target++;
		from = cursor + next - (size_t)is_last;
	}
	/* Last differed move. */
	move = last - from;
	dst = BOX_(at)(box, target), src = BOX_(at)(box, from),
		memmove(dst, src, sizeof *src * move),
		target += move, assert(target <= last);
	BOX_(tell_size)(box, target);
}

/** <src/compare.h>, `BOX_CONTIGUOUS`: Removes consecutive duplicate elements
 in `box`. @order \O(|`box`|) @allow */
static void CMP_(unique)(PCMP_(box) *const box) { CMP_(unique_merge)(box, 0); }

#endif /* contiguous --> */

static void PCMP_(unused_compare_coda)(void);
static void PCMP_(unused_compare)(void) {
#ifdef COMPARE /* <!-- compare */
	CMP_(compare)(0, 0);
#ifdef BOX_ACCESS
	CMP_(lower_bound)(0, 0); CMP_(upper_bound)(0, 0);
#ifdef BOX_CONTIGUOUS
	CMP_(insert_after)(0, 0); CMP_(sort)(0); CMP_(reverse)(0);
#endif
#endif
#endif /* compare --> */
	CMP_(is_equal)(0, 0);
#ifdef BOX_CONTIGUOUS
	CMP_(unique_merge)(0, 0); CMP_(unique)(0);
#endif
	PCMP_(unused_compare_coda)();
}
static void PCMP_(unused_compare_coda)(void) { PCMP_(unused_compare)(); }

#ifdef COMPARE
#undef COMPARE
#endif
#ifdef COMPARE_IS_EQUAL
#undef COMPARE_IS_EQUAL
#endif
#ifdef BOX_COMPARE_NAME
#undef BOX_COMPARE_NAME
#endif
/*#undef CMP_
#undef PCMP_ Need for tests. */

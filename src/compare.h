/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Compare trait

 Interface minimum: `BOX_`, `BOX`, `BOX_CONTENT`.

 @param[CMP_(n)]
 A one-argument macro producing a name that is responsible for the name of the
 functions. Should be something like `CMP_(x) -> foo_widget_x`. The caller is
 responsible for undefining `CMP_`.

 @param[COMPARE_IS_EQUAL, COMPARE]
 Function implementing <typedef:<PCMP>is_equal_fn> or
 <typedef:<PCMP>compare_fn>. One is required, (but not two.)

 @std C89 */

/* `BOX_CONTENT`: is_content, forward, forward_begin, forward_next. */
#if !defined(BOX_) || !defined(BOX) || !defined(BOX_CONTENT) \
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
enum compare_operation {
	CMP_SUBTRACTION_AB = 1,
	CMP_SUBTRACTION_BA = 2, CMP_A,
	CMP_INTERSECTION   = 4, CMP_B, CMP_C, CMP_D,
	CMP_DEFAULT_A      = 8, CMP_E, CMP_F, CMP_G, CMP_H, CMP_I, CMP_J, CMP_K,
	CMP_DEFAULT_B      = 16, CMP_L, CMP_M, CMP_N, CMP_O, CMP_P, CMP_Q, CMP_R,
		CMP_S, CMP_T, CMP_U, CMP_V, CMP_W, CMP_X, CMP_Y, CMP_Z
};
#endif /* idempotent --> */

typedef BOX PCMP_(box);
typedef BOX_CONTENT PCMP_(element);
typedef const BOX_CONTENT PCMP_(element_c);

/** <src/compare.h>: Returns a boolean given two read-only elements. */
typedef int (*PCMP_(bipredicate_fn))(const PCMP_(element_c),
	const PCMP_(element_c));
/** <src/compare.h>: Three-way comparison on a totally order set; returns an
 integer value less than, equal to, greater than zero, if `a < b`, `a == b`,
 `a > b`, respectively. */
typedef int (*PCMP_(compare_fn))(const PCMP_(element_c) restrict a,
	const PCMP_(element_c) restrict b);
/** <src/compare.h>: Returns a boolean given two modifiable arguments. */
typedef int (*PCMP_(biaction_fn))(PCMP_(element) restrict,
	PCMP_(element) restrict);

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
	struct BOX_(forward) ia = BOX_(forward_begin)(a),
		ib = BOX_(forward_begin)(b);
	for( ; ; ) {
		const PCMP_(element_c) x = BOX_(forward_next)(&ia),
			y = BOX_(forward_next)(&ib);
		int diff;
		if(!BOX_(is_content)(x)) return BOX_(is_content)(y) ? -1 : 0;
		else if(!BOX_(is_content)(y)) return 1;
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
		if(PCMP_(compare)(element, BOX_(at)(box,
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
		if(PCMP_(compare)(element, BOX_(at)(box,
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
	if(!BOX_(is_content)(first)) return; /* That was weird. */
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
	if(!BOX_(is_content)(first)) return; /* That was weird. */
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
	struct BOX_(forward) ia = BOX_(forward_begin)(a),
		ib = BOX_(forward_begin)(b);
	for( ; ; ) {
		const PCMP_(element_c) x = BOX_(forward_next)(&ia),
			y = BOX_(forward_next)(&ib);
		if(!BOX_(is_content)(x)) return !BOX_(is_content)(y);
		else if(!BOX_(is_content)(y)) return 0;
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
			if(!PCMP_(is_equal)(a, b)) break;
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

#ifdef BOX_RANGE /* <!-- range */

typedef BOX_RANGE PCMP_(range);
typedef PCMP_(range) (*PCMP_(range_fn))(PCMP_(box) *,
	const PCMP_(element_c) low, const PCMP_(element_c) high);
typedef void (*PA_(move_fn))(PCMP_(box) *, const PCMP_(range));

/** `abox` `op` `bbox` -> `result`. Prefers `abox` to `bbox` when equal.
 Either could be null. @order \O(|`abox`| + |`bbox`|) */
static int PCMP_(boolean)(PCMP_(box) *restrict const abox,
	PCMP_(box) *restrict const bbox, PCMP_(box) *restrict const result,
	const enum compare_operation op) {
	struct BOX_(iterator) ait = BOX_(begin)(abox),
		bit = BOX_(begin)(bbox);
	PCMP_(element) temp, a = BOX_(next)(&ait), b = BOX_(next)(&bit);
	char astr[12], bstr[12];
	assert((!result || (result != abox && result != bbox))
		&& (!abox || (abox != bbox)));
	if(BOX_(is_content)(a) && BOX_(is_content)(b)) {
#ifdef COMPARE
		int cmp = PCMP_(compare)(a, b);
#else
		int cmp = PCMP_(is_equal)(a, b);
#endif
		PA_(to_string)(a, &astr), PA_(to_string)(b, &bstr);
		if(
#ifdef COMPARE
		cmp < 0
#else
		   !cmp
#endif
		) {
			temp = a, a = BOX_(next)(&ait);
			if(op & CMP_SUBTRACTION_AB) {
				printf("%s: %s -> %s\n", astr, orcify(abox), orcify(result));
				/*PL_(remove)(temp);
				if(result) PL_(push)(result, temp);*/
			}
		}
#ifdef COMPARE
		else if(cmp > 0) {
			temp = b, b = BOX_(next)(&bit);
			if(op & CMP_SUBTRACTION_BA) {
				printf("%s: %s -> %s\n", bstr, orcify(bbox), orcify(result));
				/*PL_(remove)(temp);
				if(result) PL_(push)(result, temp);*/
			}
		}
#endif
		else {
			temp = a, a = BOX_(next)(&ait), b = BOX_(next)(&bit);
			if(op & CMP_INTERSECTION) {
				printf("%s: %s,%s -> %s\n",
					bstr, orcify(abox), orcify(bbox), orcify(result));
				/*PL_(remove)(temp);
				if(result) PL_(push)(result, temp);*/
			}
		}
	}
	if(abox && op & CMP_DEFAULT_A) {
		while((temp = a, a = BOX_(next)(&ait))) {
			PA_(to_string)(temp, &astr);
			printf("%s: %s -> %s\n", astr, orcify(abox), orcify(result));
			/*PL_(remove)(temp);
			if(result) PL_(push)(result, temp);*/
		}
	}
	if(bbox && op & CMP_DEFAULT_B) {
		while((temp = b, b = BOX_(next)(&bit))) {
			PA_(to_string)(temp, &bstr);
			printf("%s: %s -> %s\n", bstr, orcify(bbox), orcify(result));
			/*PL_(remove)(temp);
			if(result) PL_(push)(result, temp);*/
		}
	}
	return 1;
}
/** Subtracts `a` from `b`, as sequential sorted individual elements, and moves
 it to `result`. All elements are removed from `a`. All parameters must be
 unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, a:D)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static int CMP_(subtraction_to)(PCMP_(box) *restrict const a,
	PCMP_(box) *restrict const b, PCMP_(box) *restrict const result)
	{ return PCMP_(boolean)(a, b, result, CMP_SUBTRACTION_AB | CMP_DEFAULT_A); }
/** Moves the union of `a` and `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. All parameters must be unique or
 can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, a:B, b:C, a:D)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static int CMP_(union_to)(PCMP_(box) *restrict const a,
	PCMP_(box) *restrict const b, PCMP_(box) *restrict const result)
	{ return PCMP_(boolean)(a, b, result, CMP_SUBTRACTION_AB
	| CMP_SUBTRACTION_BA | CMP_INTERSECTION | CMP_DEFAULT_A | CMP_DEFAULT_B); }
#if 0
/** Moves the intersection of `a` and `b` as sequential sorted individual
 elements to `result`. Equal elements are moved from `a`. All parameters must
 be unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:B)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void LC_(intersection_to)(struct L_(list) *const a,
	struct L_(list) *const b, struct L_(list) *const result) {
	PLC_(boolean)(a, b, LIST_INTERSECTION, result);
}
/** Moves `a` exclusive-or `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. All parameters must be unique or
 can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, b:C, a:D)` would be moved to `result`.
 @order O(|`a`| + |`b`|) @allow */
static void LC_(xor_to)(struct L_(list) *const a, struct L_(list) *const b,
	struct L_(list) *const result) {
	PLC_(boolean)(a, b, LIST_SUBTRACTION_AB | LIST_SUBTRACTION_BA
		| LIST_DEFAULT_A | LIST_DEFAULT_B, result);
}
#endif

#endif /* range --> */

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

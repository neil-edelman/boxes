/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Compare trait

 Interface defined by box.

 @param[COMPARE_IS_EQUAL, COMPARE]
 Function implementing <typedef:<pT>bipredicate_fn> or
 <typedef:<pT>compare_fn>. One is required, but not two.

 @std C89 */

#define BOX_ALL /* Sanity check. */
#include "box.h"

#include <stddef.h>
#include <limits.h>

#ifndef COMPARE_H
#	define COMPARE_H
/** <../../src/compare.h>: The type of the required `<tr>compare`. Three-way
 comparison on a totally order set; returns an integer value less than, equal
 to, greater than zero, if `a < b`, `a == b`, `a > b`, respectively. */
typedef int (*pT_(compare_fn))(const pT_(type) *restrict a,
	const pT_(type) *restrict b);
/** <../../src/compare.h>: The type of the required `<tr>is_equal`. Returns a
 symmetric boolean given two read-only elements. */
typedef int (*pT_(bipredicate_fn))(pT_(type) *restrict, pT_(type) *restrict);
/** <../../src/compare.h>: Returns a boolean given two modifiable arguments. */
typedef int (*pT_(biaction_fn))(pT_(type) *restrict,
	pT_(type) *restrict);
#endif
#ifdef BOX_NON_STATIC
#	ifdef COMPARE
int TR_(compare)(const pT_(box) *restrict, const pT_(box) *restrict);
#		ifdef BOX_ACCESS
size_t TR_(lower_bound)(const pT_(box) *, const pT_(type) *);
size_t TR_(upper_bound)(const pT_(box) *, const pT_(type) *);
#			ifdef BOX_CONTIGUOUS
int TR_(insert_after)(pT_(box) *, const pT_(type) *);
void TR_(sort)(pT_(box) *);
void TR_(reverse)(pT_(box) *);
#			endif
#		endif
#	endif
int TR_(is_equal)(const pT_(box) *restrict, const pT_(box) *restrict);
#	ifdef BOX_CONTIGUOUS
void TR_(unique_merge)(pT_(box) *, const pT_(biaction_fn));
void TR_(unique)(pT_(box) *);
#	endif
#endif
#ifndef BOX_DECLARE_ONLY

#	define BOX_PUBLIC_OVERRIDE
#	include "box.h"

#	ifdef COMPARE /* <!-- compare: <typedef:<pT>compare_fn>. */

/** <../../src/compare.h>, `COMPARE`: Lexicographically compares `a` to `b`. Both can
 be null, with null values before everything.
 @return `a < b`: negative; `a == b`: zero; `a > b`: positive.
 @order \O(`|a|` & `|b|`) @allow */
static int TR_(compare)(const pT_(box) *restrict const a,
	const pT_(box) *restrict const b) {
	union { const pT_(box) *readonly; pT_(box) *promise; } sly_a, sly_b;
	struct T_(cursor) i, j;
	if(!a) return b ? 1 : 0;
	if(!b) return -1;
	for(sly_a.readonly = a, sly_b.readonly = b,
		i = T_(begin)(sly_a.promise), j = T_(begin)(sly_b.promise); ;
		T_(next)(&i), T_(next)(&j)) {
		int diff;
		if(!T_(exists)(&i)) return T_(exists)(&j) ? -1 : 0;
		else if(!T_(exists)(&j)) return 1;
		/* Must have this function declared.
		 "Discards qualifiers in nested pointer types" sometimes. Cast. */
		if(diff = tr_(compare)((const void *)T_(entry)(&i),
			(const void *)T_(entry)(&j))) return diff;
	}
}

#		ifdef BOX_ACCESS /* <!-- access: size, at. */

/** <../../src/compare.h>, `COMPARE`, `BOX_ACCESS`: `box` should be partitioned
 true/false with less-then `element`. @return The first index of `a` that is
 not less than `cursor`. @order \O(log `a.size`) @allow */
static size_t TR_(lower_bound)(const pT_(box) *const box,
	const pT_(type) *const element) {
	size_t low = 0, high = T_(size)(box), mid;
	while(low < high)
		if(TR_(compare)((const void *)element, (const void *)
			T_(data_at)(box, mid = low + (high - low) / 2)) <= 0) high = mid;
		else low = mid + 1;
	return low;
}

/** <../../src/compare.h>, `COMPARE`, `BOX_ACCESS`: `box` should be partitioned
 false/true with greater-than or equal-to `element`.
 @return The first index of `box` that is greater than `element`.
 @order \O(log |`box`|) @allow */
static size_t TR_(upper_bound)(const pT_(box) *const box,
	const pT_(type) *const element) {
	size_t low = 0, high = T_(size)(box), mid;
	while(low < high)
		if(TR_(compare)((const void *)element,
			(const void *)T_(data_at)(box, mid = low + (high - low) / 2)) >= 0)
			low = mid + 1;
		else high = mid;
	return low;
}

#			ifdef BOX_CONTIGUOUS /* <!-- contiguous: array. */

/** <../../src/compare.h>, `COMPARE`, `BOX_CONTIGUOUS`: Copies `element` at the upper
 bound of a sorted `box`.
 @return Success. @order \O(`a.size`) @throws[realloc, ERANGE] @allow */
static int TR_(insert_after)(pT_(box) *const box,
	const pT_(type) *const element) {
	size_t bound;
	assert(box && element);
	bound = TR_(upper_bound)(box, element);
	if(!T_(append)(box, 1)) return 0;
	memmove(T_(data_at)(box, bound + 1), T_(data_at)(box, bound),
		sizeof *element * (T_(size)(box) - bound - 1));
	memcpy(T_(data_at)(box, bound), element, sizeof *element);
	return 1;
}

#				define BOX_PRIVATE_AGAIN
#				include "../src/box.h"
/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int pTR_(vcompar)(const void *restrict const a,
	const void *restrict const b) { return tr_(compare)(a, b); }
/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int pTR_(vrevers)(const void *restrict const a,
	const void *restrict const b) { return tr_(compare)(b, a); }
#				define BOX_PUBLIC_OVERRIDE
#				include "box.h"

/** <../../src/compare.h>, `COMPARE`, `BOX_CONTIGUOUS`: Sorts `box` by `qsort`,
 (which has a high-context-switching cost, but is easy.)
 @order \O(|`box`| \log |`box`|) @allow */
static void TR_(sort)(pT_(box) *const box) {
	const size_t size = T_(size)(box);
	pT_(type) *first;
	if(!size) return;
	first = T_(data_at)(box, 0);
	/*if(!BOX_(is_element)(first)) return;*/ /* That was weird. */
	qsort(first, size, sizeof *first, &pTR_(vcompar));
}

/** <../../src/compare.h>, `COMPARE`, `BOX_CONTIGUOUS`: Sorts `box` in reverse by
 `qsort`. @order \O(|`box`| \log |`box`|) @allow */
static void TR_(reverse)(pT_(box) *const box) {
	const size_t size = T_(size)(box);
	pT_(type) *first;
	if(!size) return;
	first = T_(data_at)(box, 0);
	/*if(!BOX_(is_element)(first)) return;*/ /* That was weird. */
	qsort(first, size, sizeof *first, &pTR_(vrevers));
}

#			endif /* contiguous --> */

#		endif /* access --> */

#		define BOX_PRIVATE_AGAIN
#		include "../src/box.h"
/** !compare(`a`, `b`) == equals(`a`, `b`).
 (This makes `COMPARE` encompass `COMPARE_IS_EQUAL`.) However, it can not
 collide with another function!
 @implements <typedef:<pT>bipredicate_fn> */
static int tr_(is_equal)(const pT_(type) *const restrict a,
	const pT_(type) *const restrict b) {
	/* "Discards qualifiers in nested pointer types" sometimes. Cast. */
	return !tr_(compare)((const void *)a, (const void *)b);
}
#		define BOX_PUBLIC_OVERRIDE
#		include "box.h"

#	endif /* compare --> */

/** <../../src/compare.h> @return If `a` piecewise equals `b`,
 which both can be null. @order \O(|`a`| & |`b`|) @allow */
static int TR_(is_equal)(const pT_(box) *restrict const a,
	const pT_(box) *restrict const b) {
	union { const pT_(box) *readonly; pT_(box) *promise; } sly_a, sly_b;
	struct T_(cursor) i, j;
	if(!a) return !b /*|| !b->size <- Null is less than empty? Easier. */;
	if(!b) return 0;
	for(sly_a.readonly = a, sly_b.readonly = b,
		i = T_(begin)(sly_a.promise), j = T_(begin)(sly_b.promise); ;
		T_(next)(&i), T_(next)(&j)) {
		if(!T_(exists)(&i)) return !T_(exists)(&j);
		else if(!T_(exists)(&j)) return 0 /* fixme: a > b? */;
		if(!tr_(is_equal)(T_(entry)(&i), T_(entry)(&j)))
			return 0;
	}
	return 1;
}

#	ifdef BOX_CONTIGUOUS /* <!-- contiguous: (array, pointer), size, at,
 tell_size. */

/** <../../src/compare.h>, `BOX_CONTIGUOUS`: Removes consecutive duplicate elements
 in `box` lazily.
 @param[merge] Controls surjection. Called with duplicate elements, if false
 `(x, y)->(x)`, if true `(x,y)->(y)`. More complex functions, `(x, y)->(x+y)`
 can be simulated by mixing the two in the value returned. Can be null: behaves
 like false, always deleting the second element.
 @order \O(|`box`|) \times \O(`merge`) @allow */
static void TR_(unique_merge)(pT_(box) *const box,
	const pT_(biaction_fn) merge) {
	size_t target, from, cursor, choice, next, move;
	const size_t last = T_(size)(box);
	int is_first, is_last;
	pT_(type) *dst, *src;
	assert(box);
	for(target = from = cursor = 0; cursor < last; cursor += next) {
		/* Bijective `[from, cursor)` is moved lazily. */
		for(choice = 0, next = 1; cursor + next < last; next++) {
			/*const*/ pT_(type) *a = T_(data_at)(box, cursor + choice),
				*b = T_(data_at)(box, cursor + next);
			if(!t_(is_equal)(a, b)) break;
			if(merge && merge(a, b)) choice = next;
		}
		if(next == 1) continue;
		/* Must move injective `cursor + choice \in [cursor, cursor + next)`. */
		is_first = !choice;
		is_last  = (choice == next - 1);
		move = cursor - from + (size_t)is_first;
		dst = T_(data_at)(box, target), src = T_(data_at)(box, from);
		memmove(dst, src, sizeof *src * move), target += move;
		if(!is_first && !is_last) dst = T_(data_at)(box, target),
			src = T_(data_at)(box, cursor + choice),
			memcpy(dst, src, sizeof *src), target++;
		from = cursor + next - (size_t)is_last;
	}
	/* Last differed move. */
	move = last - from;
	dst = T_(data_at)(box, target), src = T_(data_at)(box, from),
		memmove(dst, src, sizeof *src * move),
		target += move, assert(target <= last);
	T_(tell_size)(box, target);
}

/** <../../src/compare.h>, `BOX_CONTIGUOUS`: Removes consecutive duplicate elements
 in `box`. @order \O(|`box`|) @allow */
static void TR_(unique)(pT_(box) *const box) { TR_(unique_merge)(box, 0); }

#	endif /* contiguous --> */

#	define BOX_PRIVATE_AGAIN
#	include "box.h"

static void pTR_(unused_compare_coda)(void);
static void pTR_(unused_compare)(void) {
#	ifdef COMPARE /* <!-- compare */
	TR_(compare)(0, 0);
#		ifdef BOX_ACCESS
	TR_(lower_bound)(0, 0); TR_(upper_bound)(0, 0);
#			ifdef BOX_CONTIGUOUS
	TR_(insert_after)(0, 0); TR_(sort)(0); TR_(reverse)(0);
#			endif
#		endif
	tr_(is_equal)(0, 0);
#	endif /* compare --> */
	TR_(is_equal)(0, 0);
#	ifdef BOX_CONTIGUOUS
	TR_(unique_merge)(0, 0); TR_(unique)(0);
#	endif
	pTR_(unused_compare_coda)();
}
static void pTR_(unused_compare_coda)(void) { pTR_(unused_compare)(); }

#endif /* Produce code. */

#ifdef COMPARE
#	undef COMPARE
#endif
#ifdef COMPARE_IS_EQUAL
#	undef COMPARE_IS_EQUAL
#endif

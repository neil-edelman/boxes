/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Compare trait

 Interface defined by box.

 @param[COMPARE_IS_EQUAL, COMPARE]
 Function implementing <typedef:<PCMP>is_equal_fn> or
 <typedef:<PCMP>compare_fn>. One is required, (but not two.)

 @std C89 */

/* fixme: I don't like it. Maybe break it up into smaller files. Seems like
 private and public is not really needed. Should be much simpler. 2024-11-25. */

#if !defined(BOX_H) || !defined(BOX_CAT) || !defined(TU_) || !defined(PTU_) \
	|| !defined(BOX_MAJOR_NAME) || !defined(BOX_MAJOR) \
	|| !defined(BOX_NAME) || !defined(BOX_MINOR) \
	|| !(defined(COMPARE_IS_EQUAL) ^ defined(COMPARE))
#	error Unexpected preprocessor symbols.
#endif

#include <stddef.h>
#include <limits.h>


/** <src/compare.h>: Returns a boolean given two read-only elements. */
typedef int (*PTU_(bipredicate_fn))(PT_(type) *restrict, PT_(type) *restrict);
/** <src/compare.h>: Three-way comparison on a totally order set; returns an
 integer value less than, equal to, greater than zero, if `a < b`, `a == b`,
 `a > b`, respectively. */
typedef int (*PTU_(compare_fn))(const PTU_(type) *restrict a,
	const PTU_(type) *restrict b);
/** <src/compare.h>: Returns a boolean given two modifiable arguments. */
typedef int (*PTU_(biaction_fn))(PT_(type) *restrict,
	PT_(type) *restrict);

#ifdef COMPARE /* <!-- compare: <typedef:<PTU>compare_fn>. */

/** <src/compare.h>, `COMPARE`: Lexicographically compares `a` to `b`. Both can
 be null, with null values before everything.
 @return `a < b`: negative; `a == b`: zero; `a > b`: positive.
 @order \O(`|a|` & `|b|`) @allow */
static int BOXTU_(compare)(const PT_(box) *restrict const a,
	const PT_(box) *restrict const b) {
	struct PT_(iterator) ia, ib;
	if(!a) return b ? 1 : 0;
	if(!b) return -1;
	{ /* We do not modify, but the compiler doesn't know that. */
		const PT_(box) *const rm_restrict = a;
		PT_(box) *promise_box;
		memcpy(&promise_box, &rm_restrict, sizeof a);
		ia = PT_(iterator)(promise_box);
	} {
		const PT_(box) *const rm_restrict = b;
		PT_(box) *promise_box;
		memcpy(&promise_box, &rm_restrict, sizeof b);
		ib = PT_(iterator)(promise_box);
	}
	for( ; ; ) {
		int diff;
		if(!PT_(next)(&ia)) return PT_(next)(&ib) ? -1 : 0;
		else if(!PT_(next)(&ib)) return 1;
		/* Must have this function declared. */
		if(diff = TU_(compare)((void *)PT_(element)(&ia),
			(void *)PT_(element)(&ib))) return diff;
	}
}

#	ifdef BOX_ACCESS /* <!-- access: size, at. */

/** <src/compare.h>, `COMPARE`, `BOX_ACCESS`: `box` should be partitioned
 true/false with less-then `element`. @return The first index of `a` that is
 not less than `cursor`. @order \O(log `a.size`) @allow */
static size_t BOXTU_(lower_bound)(const PT_(box) *const box,
	const PT_(type) *const element) {
	size_t low = 0, high = PT_(size)(box), mid;
	while(low < high)
		if(TU_(compare)((const void *)element, (const void *)
			PT_(at)(box, mid = low + (high - low) / 2)) <= 0) high = mid;
		else low = mid + 1;
	return low;
}

/** <src/compare.h>, `COMPARE`, `BOX_ACCESS`: `box` should be partitioned
 false/true with greater-than or equal-to `element`.
 @return The first index of `box` that is greater than `element`.
 @order \O(log |`box`|) @allow */
static size_t BOXTU_(upper_bound)(const PT_(box) *const box,
	const PT_(type) *const element) {
	size_t low = 0, high = PT_(size)(box), mid;
	while(low < high)
		if(TU_(compare)((const void *)element,
			(const void *)PT_(at)(box, mid = low + (high - low) / 2)) >= 0)
			low = mid + 1;
		else high = mid;
	return low;
}

#		ifdef BOX_CONTIGUOUS /* <!-- contiguous: element is pointer to array. */

/** <src/compare.h>, `COMPARE`, `BOX_CONTIGUOUS`: Copies `element` at the upper
 bound of a sorted `box`.
 @return Success. @order \O(`a.size`) @throws[realloc, ERANGE] @allow */
static int BOXTU_(insert_after)(PT_(box) *const box,
	const PT_(type) *const element) {
	size_t bound;
	assert(box && element);
	bound = BOXTU_(upper_bound)(box, element); /* hmm */
	if(!BOXTU_(append)(box, 1)) return 0; /* hmm */
	memmove(PT_(at)(box, bound + 1), PT_(at)(box, bound),
		sizeof *element * (PT_(size)(box) - bound - 1));
	memcpy(PT_(at)(box, bound), element, sizeof *element);
	return 1;
}

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PTU_(vcompar)(const void *restrict const a,
	const void *restrict const b) { return TU_(compare)(a, b); }

/** <src/compare.h>, `COMPARE`, `BOX_CONTIGUOUS`: Sorts `box` by `qsort`,
 (which has a high-context-switching cost, but is easy.)
 @order \O(|`box`| \log |`box`|) @allow */
static void BOXTU_(sort)(PT_(box) *const box) {
	const size_t size = PT_(size)(box);
	PT_(type) *first;
	if(!size) return;
	first = PT_(at)(box, 0);
	/*if(!BOX_(is_element)(first)) return;*/ /* That was weird. */
	qsort(first, size, sizeof *first, &PTU_(vcompar));
}

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PTU_(vrevers)(const void *restrict const a,
	const void *restrict const b) { return TU_(compare)(b, a); }

/** <src/compare.h>, `COMPARE`, `BOX_CONTIGUOUS`: Sorts `box` in reverse by
 `qsort`. @order \O(|`box`| \log |`box`|) @allow */
static void BOXTU_(reverse)(PT_(box) *const box) {
	const size_t size = PT_(size)(box);
	PT_(type) *first;
	if(!size) return;
	first = PT_(at)(box, 0);
	/*if(!BOX_(is_element)(first)) return;*/ /* That was weird. */
	qsort(first, size, sizeof *first, &PTU_(vrevers));
}

#		endif /* contiguous --> */

#	endif /* access --> */

/** !compare(`a`, `b`) == equals(`a`, `b`).
 (This makes `COMPARE` encompass `COMPARE_IS_EQUAL`.) However, it can not
 collide with another function!
 @implements <typedef:<PTU>bipredicate_fn> */
static int TU_(is_equal)(const PT_(type) *const restrict a,
	const PT_(type) *const restrict b) {
	return !TU_(compare)((const void *)a, (const void *)b);
}

#endif /* compare --> */

/** <src/compare.h> @return If `a` piecewise equals `b`,
 which both can be null. @order \O(|`a`| & |`b`|) @allow */
static int BOXTU_(is_equal)(const PT_(box) *restrict const a,
	const PT_(box) *restrict const b) {
	struct PT_(iterator) ia, ib;
	if(!a) return !b /*|| !b->size <- Null is less than empty? Easier. */;
	if(!b) return 0;
	{ /* We do not modify, but the compiler doesn't know that. */
		const PT_(box) *const rm_restrict = a;
		PT_(box) *promise_box;
		memcpy(&promise_box, &rm_restrict, sizeof a);
		ia = PT_(iterator)(promise_box);
	} {
		const PT_(box) *const rm_restrict = b;
		PT_(box) *promise_box;
		memcpy(&promise_box, &rm_restrict, sizeof b);
		ib = PT_(iterator)(promise_box);
	}
	for( ; ; ) {
		if(!PT_(next)(&ia)) return !PT_(next)(&ib);
		else if(!PT_(next)(&ib)) return 0;
		if(!TU_(is_equal)(PT_(element)(&ia), PT_(element)(&ib)))
			return 0;
	}
	return 1;
}

#ifdef BOX_CONTIGUOUS /* <!-- contiguous: (array, pointer), size, at,
 tell_size [?]. */

/** <src/compare.h>, `BOX_CONTIGUOUS`: Removes consecutive duplicate elements
 in `box` lazily.
 @param[merge] Controls surjection. Called with duplicate elements, if false
 `(x, y)->(x)`, if true `(x,y)->(y)`. More complex functions, `(x, y)->(x+y)`
 can be simulated by mixing the two in the value returned. Can be null: behaves
 like false, always deleting the second element.
 @order \O(|`box`|) \times \O(`merge`) @allow */
static void BOXTU_(unique_merge)(PT_(box) *const box,
	const PTU_(biaction_fn) merge) {
	size_t target, from, cursor, choice, next, move;
	const size_t last = PT_(size)(box);
	int is_first, is_last;
	PT_(type) *dst, *src;
	assert(box);
	for(target = from = cursor = 0; cursor < last; cursor += next) {
		/* Bijective `[from, cursor)` is moved lazily. */
		for(choice = 0, next = 1; cursor + next < last; next++) {
			/*const*/ PT_(type) *a = PTU_(at)(box, cursor + choice),
				*b = PTU_(at)(box, cursor + next);
			if(!TU_(is_equal)(a, b)) break;
			if(merge && merge(a, b)) choice = next;
		}
		if(next == 1) continue;
		/* Must move injective `cursor + choice \in [cursor, cursor + next)`. */
		is_first = !choice;
		is_last  = (choice == next - 1);
		move = cursor - from + (size_t)is_first;
		dst = PTU_(at)(box, target), src = PTU_(at)(box, from);
		memmove(dst, src, sizeof *src * move), target += move;
		if(!is_first && !is_last) dst = PT_(at)(box, target),
			src = PT_(at)(box, cursor + choice),
			memcpy(dst, src, sizeof *src), target++;
		from = cursor + next - (size_t)is_last;
	}
	/* Last differed move. */
	move = last - from;
	dst = PT_(at)(box, target), src = PT_(at)(box, from),
		memmove(dst, src, sizeof *src * move),
		target += move, assert(target <= last);
	PT_(tell_size)(box, target);
}

/** <src/compare.h>, `BOX_CONTIGUOUS`: Removes consecutive duplicate elements
 in `box`. @order \O(|`box`|) @allow */
static void BOXTU_(unique)(PT_(box) *const box)
	{ BOXTU_(unique_merge)(box, 0); }

#endif /* contiguous --> */

static void PTU_(unused_compare_coda)(void);
static void PTU_(unused_compare)(void) {
#ifdef COMPARE /* <!-- compare */
	BOXTU_(compare)(0, 0);
#ifdef BOX_ACCESS
	BOXTU_(lower_bound)(0, 0); BOXTU_(upper_bound)(0, 0);
#ifdef BOX_CONTIGUOUS
	BOXTU_(insert_after)(0, 0); BOXTU_(sort)(0); BOXTU_(reverse)(0);
#endif
#endif
#endif /* compare --> */
	BOXTU_(is_equal)(0, 0);
#ifdef BOX_CONTIGUOUS
	BOXTU_(unique_merge)(0, 0); BOXTU_(unique)(0);
#endif
	PTU_(unused_compare_coda)();
}
static void PTU_(unused_compare_coda)(void) { PTU_(unused_compare)(); }

#ifdef COMPARE
#undef COMPARE
#endif
#ifdef COMPARE_IS_EQUAL
#undef COMPARE_IS_EQUAL
#endif

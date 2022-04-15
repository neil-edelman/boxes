/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Iterate trait

 Interface minimum: `BOX_`, `BOX`, `BOX_CONTENT`.

 @param[FWD_]
 A one-argument macro producing a name that is responsible for the name of the
 functions. Should be something like `FWD_(x) -> foo_widget_x`. The caller is
 responsible for undefining `FWD_`.

 @std C89 */

/* `BOX_CONTENT`: is_content, forward, forward_begin, forward_next. */
#if !defined(BOX_) || !defined(BOX) || !defined(BOX_CONTENT) || !defined(FWD_)
#error Unexpected preprocessor symbols.
#endif

#ifndef ITERATION_H /* <!-- idempotent */
#define ITERATION_H
#include <stddef.h>
#include <limits.h>
#if defined(ITERATION_CAT_) || defined(ITERATION_CAT) || defined(PFWD_)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define ITERATION_CAT_(n, m) n ## _ ## m
#define ITERATION_CAT(n, m) ITERATION_CAT_(n, m)
#define PFWD_(n) ITERATION_CAT(iterate, FWD_(n))
#endif /* idempotent --> */

typedef BOX PFWD_(box);
typedef BOX_CONTENT PFWD_(element);
typedef const BOX_CONTENT PFWD_(element_c);

/** <src/iteration.h>: Operates by side-effects. */
typedef void (*PFWD_(action_fn))(PFWD_(element));
/** <src/iteration.h>: Returns a boolean given read-only. */
typedef int (*PFWD_(predicate_fn))(const PFWD_(element));

#ifdef BOX_CONTIGUOUS /* <!-- contiguous */

/** <src/iteration.h>, `BOX_CONTIGUOUS`: For all elements of `src`, calls `copy`,
 and if true, lazily copies the elements to `dst`. `dst` and `src` can not be
 the same but `src` can be null, (in which case, it does nothing.)
 @order \O(|`src`| \times `copy`) @throws[realloc] @allow */
static int FWD_(copy_if)(PFWD_(box) */*restrict*/const dst,
	const PFWD_(box) */*restrict*/const src, const PFWD_(predicate_fn) copy) {
	PFWD_(element) i, fresh, end, rise = 0;
	size_t add;
	int difcpy = 0;
	assert(dst && copy && dst != src);
	if(!src) return 1;
	for(i = BOX_(at)(src, 0), end = i + BOX_(size)(src); i < end; i++) {
		if(!(!!rise ^ (difcpy = copy(i)))) continue; /* Not falling/rising. */
		if(difcpy) { /* Rising edge. */
			assert(!rise);
			rise = i;
		} else { /* Falling edge. */
			assert(rise && !difcpy && rise < i);
			if(!(fresh = BOX_(append)(dst, add = (size_t)(i - rise)))) return 0;
			memcpy(fresh, rise, sizeof *fresh * add);
			rise = 0;
		}
	}
	if(rise) { /* Delayed copy. */
		assert(!difcpy && rise < i);
		if(!(fresh = BOX_(append)(dst, add = (size_t)(i - rise)))) return 0;
		memcpy(fresh, rise, sizeof *fresh * add);
	}
	return 1;
}

/** <src/iteration.h>, `BOX_CONTIGUOUS`: For all elements of `box`, calls `keep`,
 and if false, lazy deletes that item. Calls `destruct` if not-null before
 deleting. @order \O(|`box`| \times `keep` \times `destruct`) @allow */
static void FWD_(keep_if)(PFWD_(box) *const box,
	const PFWD_(predicate_fn) keep, const PFWD_(action_fn) destruct) {
	PFWD_(element) erase = 0, i, retain = 0, end;
	int keep0 = 1, keep1 = 0;
	assert(box && keep);
	for(i = BOX_(at)(box, 0), end = i + BOX_(size)(box); i < end;
		keep0 = keep1, i++) {
		if(!(keep1 = !!keep(i)) && destruct) destruct(i);
		if(!(keep0 ^ keep1)) continue; /* Not a falling/rising edge. */
		if(keep1) { /* Rising edge. */
			assert(erase && !retain);
			retain = i;
		} else if(erase) { /* Falling edge. */
			size_t n = (size_t)(i - retain);
			assert(erase < retain && retain < i);
			memmove(erase, retain, sizeof *i * n);
			erase += n;
			retain = 0;
		} else { /* Falling edge, (first time only.) */
			erase = i;
		}
	}
	if(!erase) return; /* All elements were kept. */
	if(keep1) { /* Delayed move when the iteration ended; repeat. */
		size_t n = (size_t)(i - retain);
		assert(retain && erase < retain && retain < i);
		memmove(erase, retain, sizeof *i * n);
		erase += n;
	}
	/* Adjust the size. */
	assert((size_t)(erase - BOX_(at)(box, 0)) <= BOX_(size)(box));
	BOX_(tell_size)(box, (size_t)(erase - BOX_(at)(box, 0)));
}

/** <src/iteration.h>, `BOX_CONTIGUOUS`: Removes at either end of `box` the
 things that `predicate`, if it exists, returns true.
 @order \O(`box.size` \times `predicate`) @allow */
static void FWD_(trim)(PFWD_(box) *const box,
	const PFWD_(predicate_fn) predicate) {
	size_t right, left;
	PFWD_(element) first;
	assert(box);
	if(!predicate) return;
	right = BOX_(size)(box);
	first = BOX_(at)(box, 0);
	while(right && predicate(first + right - 1)) right--;
	for(left = 0; left < right && predicate(first + left); left++);
	if(right == BOX_(size)(box) && !left) return; /* No change. */
	assert(left <= right);
	if(left) memmove(first, first + left, sizeof *first * (right - left));
	BOX_(tell_size)(box, right - left);
}

/* fixme: These further fns could be implemented with a less restrictive
 setting than `BOX_ITERATOR`, as they only need `BOX_FORWARD`. */

/** <src/iteration.h>, `BOX_CONTIGUOUS`: Iterates through `box` and calls
 `action` on all the elements. The topology of the list must not change while
 in this function.
 @order \O(|`box`| \times `action`) @allow */
static void FWD_(each)(PFWD_(box) *const box, const PFWD_(action_fn) action) {
	PFWD_(element) i, end;
	assert(box && action);
	for(i = BOX_(at)(box, 0), end = i + BOX_(size)(box); i < end; i++)
		action(i);
}

/** <src/iteration.h>, `BOX_CONTIGUOUS`: Iterates through `box` and calls
 `action` on all the elements for which `predicate` returns true. The topology
 of the list should not change while in this function.
 @order \O(`box.size` \times `predicate` \times `action`) @allow */
static void FWD_(if_each)(PFWD_(box) *const box,
	const PFWD_(predicate_fn) predicate, const PFWD_(action_fn) action) {
	PFWD_(element) i, end;
	assert(box && predicate && action);
	for(i = BOX_(at)(box, 0), end = i + BOX_(size)(box); i < end; i++)
		if(predicate(i)) action(i);
}

/** <src/iteration.h>, `BOX_CONTIGUOUS`: Iterates through `box` and calls
 `predicate` until it returns true.
 @return The first `predicate` that returned true, or, if the statement is
 false on all, null. @order \O(`box.size` \times `predicate`) @allow */
static PFWD_(element) FWD_(any)(const PFWD_(box) *const box,
	const PFWD_(predicate_fn) predicate) {
	PFWD_(element) i, end;
	assert(box && predicate);
	for(i = BOX_(at)(box, 0), end = i + BOX_(size)(box); i < end; i++)
		if(predicate(i)) return i;
	return 0;
}

#endif /* contiguous --> */

/*static void PFWD_(unused_function_coda)(void);
static void PFWD_(unused_function)(void)
	{ FWD_(copy_if)(0, 0, 0); FWD_(keep_if)(0, 0, 0); FWD_(trim)(0, 0);
	FWD_(each)(0, 0); FWD_(if_each)(0, 0, 0); FWD_(any)(0, 0);
	PFWD_(unused_function_coda)(); }
static void PFWD_(unused_function_coda)(void) { PFWD_(unused_function)(); }*/

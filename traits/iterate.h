/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Iterate trait

 Interface defined by box. Singleton.

 @std C89 */
/**
 @param[HAVE_ITERATE_H]
 The `<ITR>` functions need this value. This includes <src/iterate.h>, which
 take no parameters. Some functions may only be available for some boxes. This
 does not expire after box completion.
 */

#if !defined(BOX_TYPE) || !defined(BOX_CONTENT) || !defined(BOX_) \
	|| !defined(BOX_MAJOR_NAME) || !defined(BOX_MINOR_NAME)
#error Unexpected preprocessor symbols.
#endif

#ifndef ITERATE_H /* <!-- idempotent */
#define ITERATE_H
#include <stddef.h>
#include <limits.h>
#if defined(ITERATE_CAT_) || defined(ITERATE_CAT) || defined(ITR_) \
	|| defined(PITR_)
#error Unexpected preprocessor symbols.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define ITERATE_CAT_(n, m) n ## _ ## m
#define ITERATE_CAT(n, m) ITERATE_CAT_(n, m)
#define ITR_(n) ITERATE_CAT(ITERATE_CAT(BOX_MINOR_NAME, BOX_MAJOR_NAME), n)
#define PITR_(n) ITERATE_CAT(iterate, ITR_(n))
#endif /* idempotent --> */

typedef BOX_TYPE PITR_(box);
typedef BOX_CONTENT PITR_(element);
typedef const BOX_CONTENT PITR_(element_c); /* Sketchy. */

/** <src/iterate.h>: Operates by side-effects. */
typedef void (*PITR_(action_fn))(PITR_(element));
/** <src/iterate.h>: Returns a boolean given read-only. */
typedef int (*PITR_(predicate_fn))(const PITR_(element_c));
/* This is duplicate const? */

/** <src/iterate.h>: Iterates through `box` and calls `predicate` until it
 returns true. @return The first `predicate` that returned true, or, if the
 statement is false on all, null.
 @order \O(`box.size`) \times \O(`predicate`) @allow */
static PITR_(element) ITR_(any)(const PITR_(box) *const box,
	const PITR_(predicate_fn) predicate) {
	struct BOX_(iterator) it;
	PITR_(element) i;
	assert(box && predicate);
	{ /* We do not modify `box`, but the compiler doesn't know that. */
		PITR_(box) *promise_box;
		memcpy(&promise_box, &box, sizeof box);
		it = BOX_(iterator)(promise_box);
	}
	while(BOX_(is_element)(i = BOX_(next)(&it))) if(predicate(i)) return i;
	return BOX_(null)();
}

/** <src/iterate.h>: Iterates through `box` and calls `action` on all the
 elements. @order \O(|`box`|) \times \O(`action`) @allow */
static void ITR_(each)(PITR_(box) *const box, const PITR_(action_fn) action) {
	struct BOX_(iterator) it;
	PITR_(element) i;
	assert(box && action);
	for(it = BOX_(iterator)(box), i = BOX_(next)(&it); BOX_(is_element)(i); ) {
		PITR_(element) j = BOX_(next)(&it);
		action(i);
		i = j; /* Could be to remove `i` from the list. */
	}
}

/** <src/iterate.h>: Iterates through `box` and calls `action` on all the
 elements for which `predicate` returns true.
 @order \O(`box.size`) \times (\O(`predicate`) + \O(`action`)) @allow */
static void ITR_(if_each)(PITR_(box) *const box,
	const PITR_(predicate_fn) predicate, const PITR_(action_fn) action) {
	struct BOX_(iterator) it;
	PITR_(element) i;
	assert(box && predicate && action);
	for(it = BOX_(iterator)(box), i = BOX_(next)(&it); BOX_(is_element)(i); ) {
		PITR_(element) j = BOX_(next)(&it);
		if(predicate(i)) action(i);
		i = j; /* Could be to remove `i` from the list. */
	}
}

#ifdef BOX_CONTIGUOUS /* <!-- contiguous */

/** <src/iterate.h>, `BOX_CONTIGUOUS`: For all elements of `src`, calls `copy`,
 and if true, lazily copies the elements to `dst`. `dst` and `src` can not be
 the same but `src` can be null, (in which case, it does nothing.)
 @order \O(|`src`|) \times \O(`copy`) @throws[realloc] @allow */
static int ITR_(copy_if)(PITR_(box) *restrict const dst,
	const PITR_(box) *restrict const src, const PITR_(predicate_fn) copy) {
	PITR_(element) i, fresh, end, rise = 0;
	size_t add;
	int difcpy = 0;
	assert(dst && copy && dst != src);
	if(!src) return 1;
	for(i = BOX_(at)(src, 0), end = i + BOX_(size)(src); i < end; i++) {
		/* Not falling/rising. */
		if(!(!!rise ^ (difcpy = copy(i)))) continue;
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

/** <src/iterate.h>, `BOX_CONTIGUOUS`: For all elements of `box`, calls `keep`,
 and if false, lazy deletes that item. Calls `destruct` if not-null before
 deleting. @order \O(|`box`|) (\times O(`keep`) + O(`destruct`)) @allow */
static void ITR_(keep_if)(PITR_(box) *const box,
	const PITR_(predicate_fn) keep, const PITR_(action_fn) destruct) {
	PITR_(element) erase = 0, i, retain = 0, end;
	int keep0 = 1, keep1 = 0;
	assert(box && keep);
	for(i = BOX_(at)(box, 0), end = i + BOX_(size)(box); i < end;
		keep0 = keep1, i++) {
		if(!(keep1 = !!keep(i)) && destruct)
			destruct(i);
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

/** <src/iterate.h>, `BOX_CONTIGUOUS`: Removes at either end of `box` the
 things that `predicate`, if it exists, returns true.
 @order \O(`box.size`) \times \O(`predicate`) @allow */
static void ITR_(trim)(PITR_(box) *const box,
	const PITR_(predicate_fn) predicate) {
	size_t right, left;
	PITR_(element) first;
	assert(box);
	if(!predicate) return;
	right = BOX_(size)(box);
	first = BOX_(at)(box, 0);
	while(right && predicate(first + right - 1))
		right--;
	for(left = 0; left < right
		&& predicate(first + left); left++);
	if(right == BOX_(size)(box) && !left) return; /* No change. */
	assert(left <= right);
	if(left) memmove(first, first + left, sizeof *first * (right - left));
	BOX_(tell_size)(box, right - left);
}

#endif /* contiguous --> */

static void PITR_(unused_iterate_coda)(void);
static void PITR_(unused_function)(void) {
	ITR_(any)(0, 0); ITR_(each)(0, 0); ITR_(if_each)(0, 0, 0);
#ifdef BOX_CONTIGUOUS
	ITR_(copy_if)(0, 0, 0); ITR_(keep_if)(0, 0, 0); ITR_(trim)(0, 0);
#endif
	PITR_(unused_iterate_coda)(); }
static void PITR_(unused_iterate_coda)(void) { PITR_(unused_function)(); }

/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Iterate trait

 Interface minimum: `BOX_`, `BOX`, `BOX_CONTENT`.

 @param[ITR_]
 A one-argument macro producing a name that is responsible for the name of the
 functions. Should be something like `ITR_(x) -> foo_widget_x`. The caller is
 responsible for undefining `ITR_`.

 @std C89 */
/**
 @param[HAVE_ITERATE_H]
 The `<ITR>` functions need this value. This includes <src/iterate.h>, which
 take no parameters. Some functions may only be available for some boxes. This
 does not expire after box completion.
 */

/* `BOX_CONTENT`: is_content, forward, forward_begin, forward_next. */
#if !defined(BOX_) || !defined(BOX) || !defined(BOX_CONTENT) \
	|| !defined(BOX_ITERATOR) || !defined(ITR_)
#error Unexpected preprocessor symbols.
#endif

#ifndef ITERATE_H /* <!-- idempotent */
#define ITERATE_H
#include <stddef.h>
#include <limits.h>
#if defined(ITERATE_CAT_) || defined(ITERATE_CAT) || defined(PITR_)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define ITERATE_CAT_(n, m) n ## _ ## m
#define ITERATE_CAT(n, m) ITERATE_CAT_(n, m)
#define PITR_(n) ITERATE_CAT(iterate, ITR_(n))
#endif /* idempotent --> */

typedef BOX PITR_(box);
typedef BOX_CONTENT PITR_(element);
typedef const BOX_CONTENT PITR_(element_c);

/** <src/iterate.h>: Operates by side-effects. */
typedef void (*PITR_(action_fn))(PITR_(element));
/** <src/iterate.h>: Returns a boolean given read-only. */
typedef int (*PITR_(predicate_fn))(const PITR_(element_c));


/** <src/iterate.h>: Iterates through `box` and calls `predicate` until it
 returns true. @return The first `predicate` that returned true, or, if the
 statement is false on all, null.
 @order \O(`box.size`) \times \O(`predicate`) @allow */
static PITR_(element) ITR_(any)(PITR_(box) *const box,
	const PITR_(predicate_fn) predicate) {
	struct BOX_(iterator) it;
	PITR_(element) i;
	assert(box && predicate);
	for(it = BOX_(begin)(box); i = BOX_(next)(&it); )
		if(predicate(i)) return i;
	return 0;
}

/** <src/iterate.h>: Iterates through `box` and calls `action` on all the
 elements. The topology of the list must not change while in this function.
 @order \O(|`box`|) \times \O(`action`) @allow */
static void ITR_(each)(PITR_(box) *const box, const PITR_(action_fn) action) {
	struct BOX_(iterator) it;
	PITR_(element) i;
	assert(box && action);
	for(it = BOX_(begin)(box); i = BOX_(next)(&it); ) action(i);
}

/** <src/iterate.h>: Iterates through `box` and calls `action` on all the
 elements for which `predicate` returns true. The topology of the list must not
 change while in this function.
 @order \O(`box.size`) \times (\O(`predicate`) + \O(`action`)) @allow */
static void ITR_(if_each)(PITR_(box) *const box,
	const PITR_(predicate_fn) predicate, const PITR_(action_fn) action) {
	struct BOX_(iterator) it;
	PITR_(element) i;
	assert(box && predicate && action);
	for(it = BOX_(begin)(box); i = BOX_(next)(&it); )
		if(predicate(i)) action(i);
}

#ifdef BOX_CONTIGUOUS /* <!-- contiguous */

/** <src/iterate.h>, `BOX_CONTIGUOUS`: For all elements of `src`, calls `copy`,
 and if true, lazily copies the elements to `dst`. `dst` and `src` can not be
 the same but `src` can be null, (in which case, it does nothing.)
 @order \O(|`src`|) \times \O(`copy`) @throws[realloc] @allow */
static int ITR_(copy_if)(PITR_(box) */*restrict*/const dst,
	const PITR_(box) */*restrict*/const src, const PITR_(predicate_fn) copy) {
	PITR_(element) i, fresh, end, rise = 0;
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
	while(right && predicate(first + right - 1)) right--;
	for(left = 0; left < right && predicate(first + left); left++);
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

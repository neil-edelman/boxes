/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Iterate trait

 Interface defined by box. Singleton.

 @std C89 */
/** * <src/iterate.h>: defining `HAVE_ITERATE_H` supplies functions. */

#define BOX_ALL /* Sanity check. */
#include "box.h"

#include <stddef.h>
#include <limits.h>

/** <src/iterate.h>: Operates by side-effects. */
typedef void (*pTU_(action_fn))(pT_(type) *);
/** <src/iterate.h>: Returns a boolean given read-only. */
typedef int (*pTU_(predicate_fn))(const pT_(type) *);

/** <src/iterate.h>: Iterates through `box` and calls `predicate` until it
 returns true. @return The first `predicate` that returned true, or, if the
 statement is false on all, null.
 @order \O(`box.size`) \times \O(`predicate`) @allow */
static pT_(type) *TU_(any)(const pT_(box) *const box,
	const pTU_(predicate_fn) predicate) {
	union { const pT_(box) *readonly; pT_(box) *promise; } slybox;
	struct T_(cursor) it;
	assert(box && predicate);
	for(slybox.readonly = box, it = T_(begin)(slybox.promise);
		T_(cursor_exists)(&it); T_(cursor_next)(&it)) {
		pT_(type) *i = T_(cursor_look)(&it);
		if(predicate(i)) return i;
	}
	return 0;
}

/** <src/iterate.h>: Iterates through `box` and calls `action` on all the
 elements. Differs calling `action` until the iterator is one-ahead, so can
 delete elements as long as it doesn't affect the next, (specifically, a
 linked-list.) @order \O(|`box`|) \times \O(`action`) @allow */
static void TU_(each)(pT_(box) *const box, const pTU_(action_fn) action) {
	struct T_(cursor) it;
	assert(box && action);
	for(it = T_(begin)(box); T_(cursor_exists)(&it); T_(cursor_next)(&it))
		action(T_(cursor_look)(&it));
}

/** <src/iterate.h>: Iterates through `box` and calls `action` on all the
 elements for which `predicate` returns true.
 @order \O(`box.size`) \times (\O(`predicate`) + \O(`action`)) @allow */
static void TU_(if_each)(pT_(box) *const box,
	const pTU_(predicate_fn) predicate, const pTU_(action_fn) action) {
	struct T_(cursor) it;
	assert(box && predicate && action);
	/* fixme: Could I to remove `i` from the list? */
	/* 2024-11-25: it depends what container… but yes, inefficiently. */
	for(it = T_(begin)(box); T_(cursor_exists)(&it); T_(cursor_next)(&it)) {
		pT_(type) *v = T_(cursor_look)(&it);
		if(predicate(v)) action(v);
	}
}

#ifdef BOX_CONTIGUOUS /* <!-- contiguous */

/** <src/iterate.h>, `pT_CONTIGUOUS`: For all elements of `src`, calls `copy`,
 and if true, lazily copies the elements to `dst`. `dst` and `src` can not be
 the same but `src` can be null, (in which case, it does nothing.)
 @order \O(|`src`|) \times \O(`copy`) @throws[realloc] @allow */
static int TU_(copy_if)(pT_(box) *restrict const dst,
	const pTU_(box) *restrict const src, const pTU_(predicate_fn) copy) {
	pT_(type) *v, *fresh, *end, *rise = 0;
	size_t add;
	int difcpy = 0;
	assert(dst && copy && dst != src);
	if(!src) return 1;
	for(v = T_(look)(src, 0), end = v + T_(size)(src); v < end; v++) {
		/* Not falling/rising. */
		if(!(!!rise ^ (difcpy = copy(v)))) continue;
		if(difcpy) { /* Rising edge. */
			assert(!rise);
			rise = v;
		} else { /* Falling edge. */
			assert(rise && !difcpy && rise < v);
			if(!(fresh = T_(append)(dst, add = (size_t)(v - rise)))) return 0;
			memcpy(fresh, rise, sizeof *fresh * add);
			rise = 0;
		}
	}
	if(rise) { /* Delayed copy. */
		assert(!difcpy && rise < v);
		if(!(fresh = T_(append)(dst, add = (size_t)(v - rise)))) return 0;
		memcpy(fresh, rise, sizeof *fresh * add);
	}
	return 1;
}

/** <src/iterate.h>: For all elements of `box`, calls `keep`,
 and if false, if contiguous, lazy deletes that item, if not, eagerly. Calls
 `destruct` if not-null before deleting.
 @order \O(|`box`|) (\times O(`keep`) + O(`destruct`)) @allow */
static void TU_(keep_if)(pT_(box) *const box,
	const pTU_(predicate_fn) keep, const pTU_(action_fn) destruct) {
	pT_(type) *erase = 0, *v, *retain = 0, *end;
	int keep0 = 1, keep1 = 0;
	assert(box && keep);
	for(v = T_(look)(box, 0), end = v + T_(size)(box); v < end;
		keep0 = keep1, v++) {
		if(!(keep1 = !!keep(v)) && destruct)
			destruct(v);
		if(!(keep0 ^ keep1)) continue; /* Not a falling/rising edge. */
		if(keep1) { /* Rising edge. */
			assert(erase && !retain);
			retain = v;
		} else if(erase) { /* Falling edge. */
			size_t n = (size_t)(v - retain);
			assert(erase < retain && retain < v);
			memmove(erase, retain, sizeof *v * n);
			erase += n;
			retain = 0;
		} else { /* Falling edge, (first time only.) */
			erase = v;
		}
	}
	if(!erase) return; /* All elements were kept. */
	if(keep1) { /* Delayed move when the iteration ended; repeat. */
		size_t n = (size_t)(v - retain);
		assert(retain && erase < retain && retain < v);
		memmove(erase, retain, sizeof *v * n);
		erase += n;
	}
	/* Adjust the size. */
	assert((size_t)(erase - T_(look)(box, 0)) <= T_(size)(box));
	T_(tell_size)(box, (size_t)(erase - T_(look)(box, 0)));
}

/** <src/iterate.h>, `pT_CONTIGUOUS`: Removes at either end of `box` the
 things that `predicate`, if it exists, returns true.
 @order \O(`box.size`) \times \O(`predicate`) @allow */
static void TU_(trim)(pT_(box) *const box,
	const pTU_(predicate_fn) predicate) {
	size_t right, left;
	pT_(type) *first;
	assert(box);
	if(!predicate) return;
	right = T_(size)(box);
	first = T_(look)(box, 0);
	while(right && predicate(first + right - 1))
		right--;
	for(left = 0; left < right
		&& predicate(first + left); left++);
	if(right == T_(size)(box) && !left) return; /* No change. */
	assert(left <= right);
	if(left) memmove(first, first + left, sizeof *first * (right - left));
	T_(tell_size)(box, right - left);
}

#endif /* contiguous --> */

static void pTU_(unused_iterate_coda)(void);
static void pTU_(unused_function)(void) {
	TU_(any)(0, 0); TU_(each)(0, 0); TU_(if_each)(0, 0, 0);
#ifdef BOX_CONTIGUOUS
	TU_(copy_if)(0, 0, 0); TU_(trim)(0, 0); TU_(keep_if)(0, 0, 0);
#endif
	pTU_(unused_iterate_coda)(); }
static void pTU_(unused_iterate_coda)(void) { pTU_(unused_function)(); }

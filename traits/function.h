/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Function Trait

 Only one should be included per box.

 @param[Z_]
 A one-argument macro producing a name that is responsible for the name of the
 functions. Should be something like `Z_(x) -> foo_widget_x`.

 @std C89 */

/* Check defines. */
#if !defined(CAT) || !defined(CAT_) || !defined(BOX_) \
	|| !defined(BOX_CONTAINER) || !defined(BOX_CONTENTS) || !defined(Z_)
#error Unexpected preprocessor symbols.
#endif

#define PZ_(n) CAT(function, Z_(n))

typedef BOX_CONTAINER PZ_(box);
typedef BOX_CONTENTS PZ_(type);

/** Operates by side-effects on <typedef:<PZ>type>. */
typedef void (*PZ_(action_fn))(PZ_(type) *);

/** Returns a boolean given read-only <typedef:<PZ>type>. */
typedef int (*PZ_(predicate_fn))(const PZ_(type) *);

#if defined(BOX_CONTIGUOUS_SIZE) /* <!-- fn */
/** @return Converts `i` to an index in `box` from [0, `a.size`]. Negative
 values are implicitly plus `box.size`. @order \Theta(1) @allow */
static size_t Z_(clip)(const PZ_(box) *const box, const long i) {
	/* `SIZE_MAX` is `C99`; assumes two's-compliment. */
	assert(box && (size_t)-1 >= (size_t)LONG_MAX
		&& (unsigned long)((size_t)-1) >= LONG_MAX);
	return i < 0
		? (size_t)-i >= box->size ? 0 : box->size - (size_t)-i
		: (size_t)i > box->size ? box->size : (size_t)i;
}
#endif /* fn --> */

#if defined(BOX_ITERATE) && defined(BOX_COPY) /* <!-- fn */
/** Needs iterate and copy interfaces. For all elements of `b`, calls `copy`,
 and if true, lazily copies the elements to `a`. `a` and `b` can not be the
 same but `b` can be null, (in which case, it does nothing.)
 @order \O(`b.size` \times `copy`) @throws[ERANGE, realloc] @allow */
static int Z_(copy_if)(PZ_(box) *const a, const PZ_(predicate_fn) copy,
	const PZ_(box) *const b) {
	struct BOX_(iterator) it;
	PZ_(type) *i, *fresh;
	const PZ_(type) /* *end,?*/ *rise = 0;
	size_t add;
	int difcpy = 0;
	assert(a && copy && a != b);
	if(!b) return 1;
	BOX_(begin)(&it, b);
	while(i = BOX_(next)(&it)) {
		if(!(!!rise ^ (difcpy = copy(i)))) continue; /* Not falling/rising. */
		if(difcpy) { /* Rising edge. */
			assert(!rise);
			rise = i;
		} else { /* Falling edge. */
			assert(rise && !difcpy && rise < i);
			if(!(fresh = BOX_(append)(a, add = (size_t)(i - rise)))) return 0;
			BOX_(copy)(fresh, rise, sizeof *fresh * add);
			rise = 0;
		}
	}
	if(rise) { /* Delayed copy. */
		assert(!difcpy && rise < i);
		if(!(fresh = BOX_(append)(a, add = (size_t)(i - rise)))) return 0;
		BOX_(copy)(fresh, rise, sizeof *fresh * add);
	}
	return 1;
}
#endif /* fn --> */

#if defined(BOX_ITERATE) && defined(BOX_COPY) /* <!-- fn */
/** For all elements of `box`, calls `keep`, and if false, lazy deletes that
 item, calling `destruct` if not-null.
 @order \O(`a.size` \times `keep` \times `destruct`) @allow */
static void Z_(keep_if)(PZ_(box) *const box,
	const PZ_(predicate_fn) keep, const PZ_(action_fn) destruct) {
	struct BOX_(iterator) it;
	PZ_(type) *erase = 0, *t;
	const PZ_(type) *retain = 0;
	int keep0 = 1, keep1 = 0;
	assert(box && keep);
	for(BOX_(begin)(&it, box); t = BOX_(next)(&it); keep0 = keep1) {
		if(!(keep1 = !!keep(t)) && destruct) destruct(t);
		if(!(keep0 ^ keep1)) continue; /* Not a falling/rising edge. */
		if(keep1) { /* Rising edge. */
			assert(erase && !retain);
			retain = t;
		} else if(erase) { /* Falling edge. */
			size_t n = (size_t)(t - retain);
			assert(erase < retain && retain < t);
			BOX_(move)(erase, retain, n);
			erase += n;
			retain = 0;
		} else { /* Falling edge, (first time only.) */
			erase = t;
		}
	}
	if(!erase) return; /* All elements were kept. */
	if(keep1) { /* Delayed move when the iteration ended; repeat. */
		size_t n = (size_t)(t - retain);
		assert(retain && erase < retain && retain < t);
		BOX_(move)(erase, retain, n);
		erase += n;
	}
	/* Adjust the size. */
	assert((size_t)(erase - box->data) <= box->size);
	box->size = (size_t)(erase - box->data);
}
#endif /* fn --> */

#if defined(BOX_ITERATE) && defined(BOX_REVERSE) \
	&& defined(BOX_REMOVE) /* <!-- fn */
/** Requires iterate, reverse, and copy interfaces. Removes at either end of
 `box` of things that `predicate` returns true.
 @order \O(`box.size` \times `predicate`) @allow @fixme */
static void Z_(trim)(PZ_(box) *const box, const PZ_(predicate_fn) predicate) {
	size_t i;
	assert(a && predicate);
	while(a->size && predicate(a->data + a->size - 1)) a->size--;
	for(i = 0; i < a->size && predicate(a->data + i); i++);
	if(!i) return;
	assert(i < a->size);
	memmove(a->data, a->data + i, sizeof *a->data * i), a->size -= i;
}
#endif /* fn --> */

#if defined(BOX_ITERATE) /* <!-- fn */
/** Iterates through `box` and calls `action` on all the elements. The topology
 of the list should not change while in this function.
 @order \O(`box.size` \times `action`) @allow */
static void Z_(each)(PZ_(box) *const box, const PZ_(action_fn) action) {
	PA_(type) *i;
	struct BOX_(iterator) it;
	assert(box && action);
	BOX_(begin)(&it, box);
	while(i = BOX_(next)(&it)) action(i);
}
#endif /* fn --> */

#if defined(BOX_ITERATE) /* <!-- fn */
/** Iterates through `box` and calls `action` on all the elements for which
 `predicate` returns true. The topology of the list should not change while in
 this function. @order \O(`box.size` \times `predicate` \times `action`)
 @allow */
static void Z_(if_each)(PZ_(box) *const box,
	const PZ_(predicate_fn) predicate, const PZ_(action_fn) action) {
	PZ_(type) *i;
	struct BOX_(iterator) it;
	assert(box && predicate && action);
	BOX_(begin)(&it, box);
	while(i = BOX_(next)(&it)) if(predicate(i)) action(i);
}
#endif /* fn --> */

#if defined(BOX_ITERATE) /* <!-- fn */
/** Requires iterate interface. Iterates through `box` and calls `predicate`
 until it returns true.
 @return The first `predicate` that returned true, or, if the statement is
 false on all, null. @order \O(`box.size` \times `predicate`) @allow */
static const PZ_(type) *Z_(any)(const PZ_(box) *const box,
	const PZ_(predicate_fn) predicate) {
	struct BOX_(iterator) it;
	PZ_(type) *x;
	assert(box && predicate);
	BOX_(begin)(&it, box);
	while(x = BOX_(next)(&it)) if(predicate(x)) return x;
	return 0;
}
#endif /* fn --> */

static void PZ_(unused_function_coda)(void);
static void PZ_(unused_function)(void) {
#if defined(BOX_CONTIGUOUS_SIZE) /* <!-- fn */
	Z_(clip)(0, 0);
#endif /* fn --> */
#if defined(BOX_ITERATE) && defined(BOX_COPY) /* <!-- fn */
	Z_(copy_if)(0, 0, 0);
#endif /* fn --> */
#if defined(BOX_ITERATE) && defined(BOX_COPY) /* <!-- fn */
	Z_(keep_if)(0, 0, 0);
#endif /* fn --> */
#if defined(BOX_ITERATE) && defined(BOX_REVERSE) \
	&& defined(BOX_REMOVE) /* <!-- fn */
	Z_(trim)(0, 0);
#endif /* fn --> */
#if defined(BOX_ITERATE) /* <!-- fn */
	Z_(each)(0, 0);
#endif /* fn --> */
#if defined(BOX_ITERATE) /* <!-- fn */
	Z_(if_each)(0, 0, 0);
#endif /* fn --> */
#if defined(BOX_ITERATE) /* <!-- fn */
	Z_(any)(0, 0);
#endif /* fn --> */
	PZ_(unused_function_coda)(); }
static void PZ_(unused_function_coda)(void) { PZ_(unused_function)(); }

#undef PZ_
#undef Z_

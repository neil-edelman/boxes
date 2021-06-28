/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Function Trait

 Emits a `FUNCTION_H` that should be undefined when one is done with the box.
 This trait can be included multiple times with or without different
 parameters, but `F_` can only be set once.

 @param[BOX_, BOX_CONTAINER, BOX_CONTENTS]
 A type that represents the box and the type that goes in the box. Does not
 undefine.

 @param[F_]
 A one-argument macro producing a name that is responsible for the name of the
 functions. Does not undefine. One should keep this until the end and undefine
 both `FUNCTION_H` and `F_`.

 @param[FUNCTION_COMPARABLE_NAME, FUNCTION_IS_EQUAL, FUNCTION_COMPARE]
 Optional compare; `<C>` that satisfies `C` naming conventions when mangled
 and a function implementing, for `FUNCTION_IS_EQUAL`
 <typedef:<PA>bipredicate_fn> that establishes an equivalence relation, or for
 `FUNCTION_COMPARE` <typedef:<PA>compare_fn> that establishes a total order.
 There can be multiple comparable functions, but only one can omit
 `FUNCTION_COMPARABLE_NAME`.

 @std C89 */

/* Check defines. */
#if !defined(CAT) || !defined(CAT_) || !defined(BOX_) \
	|| !defined(BOX_CONTAINER) || !defined(BOX_CONTENTS)
#error Unexpected preprocessor symbols.
#endif

#ifndef F_
#error Macro `F_(n)` producing a name must be defined.
#endif

#ifndef FUNCTION_H /* <!-- idempotent */

#define FUNCTION_H CAT(function, F_(n))

#else /**/

/*  */
#endif

#define PF_(n) CAT(function, F_(n))

#ifdef ARRAY_COMPARABLE_NAME /* <!-- name */
#define PTC_(n) CAT(PA_(n), ARRAY_COMPARABLE_NAME)
#define T_C_(n, m) CAT(A_(n), CAT(ARRAY_COMPARABLE_NAME, m))
#else /* name --><!-- !name */
#define PTC_(n) CAT(PA_(n), anonymous)
#define T_C_(n, m) CAT(A_(n), m)
#endif /* !name --> */


typedef BOX_CONTAINER PF_(box);
typedef BOX_CONTENTS PF_(type);

/** Operates by side-effects. */
typedef void (*PF_(action_fn))(PF_(type) *);

/** Returns a boolean given two <typedef:<PF>type>. */
typedef int (*PF_(biaction_fn))(PF_(type) *, PF_(type) *);

/** Returns a boolean given read-only <typedef:<PF>type>. */
typedef int (*PF_(predicate_fn))(const PF_(type) *);

/** Returns a boolean given two read-only <typedef:<PF>type>. */
typedef int (*PF_(bipredicate_fn))(const PF_(type) *, const PF_(type) *);

/** Three-way comparison on a totally order set; returns an integer value less
 then, equal to, greater then zero, if `a < b`, `a == b`, `a > b`,
 respectively. */
typedef int (*PF_(compare_fn))(const PF_(type) *a, const PF_(type) *b);

#if 0
/** @return Converts `i` to an index in `a` from [0, `a.size`]. Negative values
 are implicitly plus `a.size`. @order \Theta(1) @allow */
static size_t A_(array_clip)(const struct A_(array) *const a, const long i) {
	/* `SIZE_MAX` is `C99`; assumes two's-compliment, not many hw-tests. */
	assert(a && (size_t)-1 >= (size_t)LONG_MAX
		&& (unsigned long)((size_t)-1) >= LONG_MAX);
	return i < 0
		? (size_t)-i >= a->size ? 0 : a->size - (size_t)-i
		: (size_t)i > a->size ? a->size : (size_t)i;
}
#endif

#if defined(BOX_ITERATE) && defined(BOX_COPY) /* <!-- fn */
/** Needs iterate and copy interfaces. For all elements of `b`, calls `copy`,
 and if true, lazily copies the elements to `a`. `a` and `b` can not be the
 same but `b` can be null.
 @order \O(`b.size` \times `copy`) @throws[ERANGE, realloc] @allow */
static int F_(copy_if)(PF_(box) *const a, const PF_(predicate_fn) copy,
	const PF_(box) *const b) {
	struct BOX_(iterator) it;
	PF_(type) *i, *fresh;
	const PF_(type) /* *end,?*/ *rise = 0;
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
			if(!(fresh = F_(append)(a, add = (size_t)(i - rise)))) return 0;
			F_(copy)(fresh, rise, sizeof *fresh * add);
			rise = 0;
		}
	}
	if(rise) { /* Delayed copy. */
		assert(!difcpy && rise < i);
		if(!(fresh = F_(append)(a, add = (size_t)(i - rise)))) return 0;
		F_(copy)(fresh, rise, sizeof *fresh * add);
	}
	return 1;
}
#endif /* fn --> */

#if 0
/** For all elements of `a`, calls `keep`, and if false, lazy deletes that
 item, calling `destruct` if not-null.
 @order \O(`a.size` \times `keep` \times `destruct`) @allow */
static void A_(array_keep_if)(struct A_(array) *const a,
	const PA_(predicate_fn) keep, const PA_(action_fn) destruct) {
	PA_(type) *erase = 0, *t;
	const PA_(type) *retain = 0, *end;
	int keep0 = 1, keep1 = 0;
	assert(a && keep);
	for(t = a->data, end = a->data + a->size; t < end; keep0 = keep1, t++) {
		if(!(keep1 = !!keep(t)) && destruct) destruct(t);
		if(!(keep0 ^ keep1)) continue; /* Not a falling/rising edge. */
		if(keep1) { /* Rising edge. */
			assert(erase && !retain);
			retain = t;
		} else if(erase) { /* Falling edge. */
			size_t n = (size_t)(t - retain);
			assert(erase < retain && retain < t);
			memmove(erase, retain, n * sizeof *t);
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
		memmove(erase, retain, n * sizeof *t);
		erase += n;
	}
	/* Adjust the size. */
	assert((size_t)(erase - a->data) <= a->size);
	a->size = (size_t)(erase - a->data);
}
#endif

#if defined(BOX_ITERATE) && defined(BOX_REVERSE) \
	&& defined(BOX_COPY) && 0 /* <!-- fn */
/** Requires iterate, reverse, and copy interfaces. Removes at either end of
 `a` of things that `predicate` returns true.
 @order \O(`a.size` \times `predicate`) @allow */
static void F_(trim)(PF_(box) *const a, const PF_(predicate_fn) predicate) {
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
/** Iterates through `a` and calls `action` on all the elements. The topology
 of the list should not change while in this function.
 @order \O(`a.size` \times `action`) @allow */
static void F_(each)(PF_(box) *const a, const PF_(action_fn) action) {
	PA_(type) *i;
	struct F_(iterator) it;
	assert(a && action);
	BOX_(begin)(&it, a);
	while(i = BOX_(next)(&it)) action(i);
}
#endif /* fn --> */

#if defined(BOX_ITERATE) /* <!-- fn */
/** Iterates through `a` and calls `action` on all the elements for which
 `predicate` returns true. The topology of the list should not change while in
 this function. @order \O(`a.size` \times `predicate` \times `action`) @allow */
static void F_(if_each)(PF_(box) *const a,
	const PF_(predicate_fn) predicate, const PF_(action_fn) action) {
	PF_(type) *i;
	struct F_(iterator) it;
	assert(a && predicate && action);
	BOX_(begin)(&it, a);
	while(i = BOX_(next)(&it)) if(predicate(i)) action(i);
}
#endif /* fn --> */

#if defined(BOX_ITERATE) /* <!-- fn */
/** Requires iterate interface. Iterates through `a` and calls `predicate`
 until it returns true.
 @return The first `predicate` that returned true, or, if the statement is
 false on all, null. @order \O(`a.size` \times `predicate`) @allow */
static const PF_(type) *F_(any)(const PF_(box) *const box,
	const PF_(predicate_fn) predicate) {
	struct BOX_(iterator) it;
	PF_(type) *x;
	assert(box && predicate);
	BOX_(begin)(&it, box);
	while(x = BOX_(next)(&it)) if(predicate(x)) return x;
	return 0;
}
#endif /* fn --> */

static void PF_(unused_function_coda)(void);
static void PF_(unused_function)(void)
	{ /*A_(array_clip)(0, 0);F_(any)(0, 0);*/ PF_(unused_function_coda)(); }
static void PF_(unused_function_coda)(void) { PF_(unused_function)(); }

#undef PF_
#undef F_

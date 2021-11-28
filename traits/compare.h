/* @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Compare Trait

 Requires contiguous data elements are stored in array `data` up to
 `size_t size` such that `memcpy` will work. `<BOX>append` function defined.

 @param[CM_]
 A one-argument macro producing a name that is responsible for the name of the
 functions. The caller is responsible for undefining `CM_`.

 @param[FUNCTION_COMPARABLE_NAME, FUNCTION_IS_EQUAL, FUNCTION_COMPARE]
 Optional unique name `<Z>` that satisfies `C` naming conventions when mangled,
 and a function implementing, for `FUNCTION_IS_EQUAL`
 <typedef:<PZ>bipredicate_fn> that establishes an equivalence relation, or for
 `FUNCTION_COMPARE` <typedef:<PZ>compare_fn> that establishes a total order.
 There can be multiple comparable functions, but only one can omit
 `FUNCTION_COMPARABLE_NAME`.

 @std C89 */

#if !defined(BOX_) || !defined(BOX_CONTAINER) || !defined(BOX_CONTENTS) \
	|| !defined(CM_)
#error Unexpected preprocessor symbols.
#endif

#ifndef COMPARE_H /* <!-- idempotent */
#define COMPARE_H
#if defined(COMPARE_CAT_) || defined(COMPARE_CAT) || defined(PCM_)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define COMPARE_CAT_(n, m) n ## _ ## m
#define COMPARE_CAT(n, m) COMPARE_CAT_(n, m)
#define PCM_(n) COMPARE_CAT(compare, CM_(n))
#endif /* idempotent --> */

typedef BOX_CONTAINER PCM_(box);
typedef BOX_CONTENTS PCM_(type);

/** Returns a boolean given two read-only <typedef:<PZ>type>. */
typedef int (*PCM_(bipredicate_fn))(const PCM_(type) *, const PCM_(type) *);

/** Returns a boolean given two <typedef:<PZ>type>. */
typedef int (*PCM_(biaction_fn))(PCM_(type) *, PCM_(type) *);

#ifdef ARRAY_COMPARE /* <!-- compare */

/** Three-way comparison on a totally order set; returns an integer value less
 then, equal to, greater then zero, if `a < b`, `a == b`, `a > b`,
 respectively. */
typedef int (*PCM_(compare_fn))(const PCM_(type) *a, const PCM_(type) *b);

/* Check that `ARRAY_COMPARE` is a function implementing
 <typedef:<PZ>compare_fn>. */
static const PCM_(compare_fn) PCM_(compare) = (ARRAY_COMPARE);

/** Lexicographically compares `a` to `b`. Null values are before everything.
 @return `a < b`: negative; `a == b`: zero; `a > b`: positive.
 @order \O(`a.size`) @allow */
static int CM_(compare)(const PCM_(box) *const a, const PCM_(box) *const b) {
	PCM_(type) *ia, *ib, *end;
	int diff;
	/* Null counts as `-\infty`. */
	if(!a) return b ? -1 : 0;
	else if(!b) return 1;
	if(a->size > b->size) {
		for(ia = a->data, ib = b->data, end = ib + b->size; ib < end;
			ia++, ib++) if((diff = PCM_(compare)(ia, ib))) return diff;
		return 1;
	} else {
		for(ia = a->data, ib = b->data, end = ia + a->size; ia < end;
			ia++, ib++) if((diff = PCM_(compare)(ia, ib))) return diff;
		return -(a->size != b->size);
	}
}

/** `a` should be partitioned true/false with less-then `value`.
 @return The first index of `a` that is not less than `value`.
 @order \O(log `a.size`) @allow */
static size_t CM_(lower_bound)(const PCM_(box) *const a,
	const PCM_(type) *const value) {
	size_t low = 0, high = a->size, mid;
	assert(a && value);
	while(low < high) if(PCM_(compare)(value, a->data
		+ (mid = low + ((high - low) >> 1))) <= 0) high = mid;
		else low = mid + 1;
	return low;
}

/** `a` should be partitioned false/true with greater-than or equals `value`.
 @return The first index of `a` that is greater than `value`.
 @order \O(log `a.size`) @allow */
static size_t CM_(upper_bound)(const PCM_(box) *const a,
	const PCM_(type) *const value) {
	size_t low = 0, high = a->size, mid;
	assert(a && value);
	while(low < high) if(PCM_(compare)(value, a->data
		+ (mid = low + ((high - low) >> 1))) >= 0) low = mid + 1;
		else high = mid;
	return low;
}

/** Copies `datum` at the upper bound of a sorted `a`.
 @return Success. @order \O(`a.size`) @throws[realloc, ERANGE] */
static int CM_(insert_after)(PCM_(box) *const a,
	const PCM_(type) *const datum) {
	size_t bound;
	assert(a && datum);
	bound = CM_(upper_bound)(a, datum);
	assert(0);
#if 0
	if(!A_(array_new)(a)) return 0;
	memmove(a->data + bound + 1, a->data + bound,
		sizeof *a->data * (a->size - bound - 1));
	memcpy(a->data + bound, datum, sizeof *datum);
	return 1;
#endif
	return 0;
}

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PCM_(vcompar)(const void *const a, const void *const b)
	{ return PCM_(compare)(a, b); }

/** Sorts `a` by `qsort` on `ARRAY_COMPARE`.
 @order \O(`a.size` \log `a.size`) @allow */
static void CM_(sort)(PCM_(box) *const a)
	{ assert(a), qsort(a->data, a->size, sizeof *a->data, PCM_(vcompar)); }

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PCM_(vrevers)(const void *const a, const void *const b)
	{ return PCM_(compare)(b, a); }

/** Sorts `a` in reverse by `qsort` on `ARRAY_COMPARE`.
 @order \O(`a.size` \log `a.size`) @allow */
static void CM_(reverse)(PCM_(box) *const a)
	{ assert(a), qsort(a->data, a->size, sizeof *a->data, PCM_(vrevers)); }

/** !compare(`a`, `b`) == equals(`a`, `b`) for not `ARRAY_IS_EQUAL`.
 @implements <typedef:<PZ>bipredicate_fn> */
static int PCM_(is_equal)(const PCM_(type) *const a, const PCM_(type) *const b)
	{ return !PCM_(compare)(a, b); }

#else /* compare --><!-- is equal */

/* Check that `ARRAY_IS_EQUAL` is a function implementing
 <typedef:<PZ>bipredicate_fn>. */
static const PZ_(bipredicate_fn) PZ_(is_equal) = (ARRAY_IS_EQUAL);

#endif /* is equal --> */

/** @return If `a` piecewise equals `b`, which both can be null.
 @order \O(`size`) */
static int CM_(is_equal)(const PCM_(box) *const a, const PCM_(box) *const b) {
	const PCM_(type) *ia, *ib, *end;
	if(!a) return !b;
	if(!b || a->size != b->size) return 0;
	for(ia = a->data, ib = b->data, end = ia + a->size; ia < end; ia++, ib++)
		if(!PCM_(is_equal)(ia, ib)) return 0;
	return 1;
}

/** Removes consecutive duplicate elements in `a`.
 @param[merge] Controls surjection. Called with duplicate elements, if false
 `(x, y)->(x)`, if true `(x,y)->(y)`. More complex functions, `(x, y)->(x+y)`
 can be simulated by mixing the two in the value returned. Can be null: behaves
 like false. @order \O(`a.size` \times `merge`) @allow */
static void CM_(unique_merge)(PCM_(box) *const a, const PCM_(biaction_fn) merge) {
	size_t target, from, cursor, choice, next, move;
	const size_t last = a->size;
	int is_first, is_last;
	assert(a);
	for(target = from = cursor = 0; cursor < last; cursor += next) {
		/* Bijective `[from, cursor)` is moved lazily. */
		for(choice = 0, next = 1; cursor + next < last && PCM_(is_equal)(a->data
			+ cursor + choice, a->data + cursor + next); next++)
			if(merge && merge(a->data + choice, a->data + next)) choice = next;
		if(next == 1) continue;
		/* Must move injective `cursor + choice \in [cursor, cursor + next)`. */
		is_first = !choice;
		is_last  = (choice == next - 1);
		move = cursor - from + (size_t)is_first;
		memmove(a->data + target, a->data + from, sizeof *a->data * move),
		target += move;
		if(!is_first && !is_last) memcpy(a->data + target,
			a->data + cursor + choice, sizeof *a->data), target++;
		from = cursor + next - (size_t)is_last;
	}
	/* Last differed move. */
	move = last - from;
	memmove(a->data + target, a->data + from, sizeof *a->data * move),
	target += move, assert(a->size >= target);
	a->size = target;
}

/** Removes consecutive duplicate elements in `a`. @order \O(`a.size`) @allow */
static void CM_(unique)(PCM_(box) *const a) { CM_(unique_merge)(a, 0); }

static void PCM_(unused_compare_coda)(void);
static void PCM_(unused_compare)(void) {
#ifdef ARRAY_COMPARE /* <!-- compare */
	CM_(compare)(0, 0); CM_(lower_bound)(0, 0); CM_(upper_bound)(0, 0);
	CM_(insert_after)(0, 0); CM_(sort)(0); CM_(reverse)(0);
#endif /* compare --> */
	CM_(is_equal)(0, 0); CM_(unique_merge)(0, 0); CM_(unique)(0);
	PCM_(unused_compare_coda)(); }
static void PCM_(unused_compare_coda)(void) { PCM_(unused_compare)(); }

#ifdef BOX_COMPARE
#undef BOX_COMPARE
#endif
#ifdef BOX_IS_EQUAL
#undef BOX_IS_EQUAL
#endif
#ifdef BOX_COMPARE_NAME
#undef BOX_COMPARE_NAME
#endif

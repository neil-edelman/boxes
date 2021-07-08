/* @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Compare Trait

 @param[Z_]
 A one-argument macro producing a name that is responsible for the name of the
 functions.

 @param[FUNCTION_COMPARABLE_NAME, FUNCTION_IS_EQUAL, FUNCTION_COMPARE]
 Optional unique name `<Z>` that satisfies `C` naming conventions when mangled,
 and a function implementing, for `FUNCTION_IS_EQUAL`
 <typedef:<PZ>bipredicate_fn> that establishes an equivalence relation, or for
 `FUNCTION_COMPARE` <typedef:<PZ>compare_fn> that establishes a total order.
 There can be multiple comparable functions, but only one can omit
 `FUNCTION_COMPARABLE_NAME`.

 @std C89 */

/* Check defines. */
#if !defined(CAT) || !defined(CAT_) || !defined(BOX_) \
	|| !defined(BOX_CONTAINER) || !defined(BOX_CONTENTS) || !defined(Z_)
#error Unexpected preprocessor symbols.
#endif

#define PZ_(n) CAT(compare, Z_(n))

typedef BOX_CONTAINER PZ_(box);
typedef BOX_CONTENTS PZ_(type);

/** Returns a boolean given two read-only <typedef:<PZ>type>. */
typedef int (*PZ_(bipredicate_fn))(const PZ_(type) *, const PZ_(type) *);

/** Returns a boolean given two <typedef:<PZ>type>. */
typedef int (*PZ_(biaction_fn))(PZ_(type) *, PZ_(type) *);

#ifdef ARRAY_COMPARE /* <!-- compare */

/** Three-way comparison on a totally order set; returns an integer value less
 then, equal to, greater then zero, if `a < b`, `a == b`, `a > b`,
 respectively. */
typedef int (*PZ_(compare_fn))(const PZ_(type) *a, const PZ_(type) *b);

/* Check that `ARRAY_COMPARE` is a function implementing
 <typedef:<PZ>compare_fn>. */
static const PZ_(compare_fn) PZ_(compare) = (ARRAY_COMPARE);

/** Lexicographically compares `a` to `b`. Null values are before everything.
 @return `a < b`: negative; `a == b`: zero; `a > b`: positive.
 @order \O(`a.size`) @allow */
static int Z_(compare)(const PZ_(box) *const a, const PZ_(box) *const b) {
	PA_(type) *ia, *ib, *end;
	int diff;
	/* Null counts as `-\infty`. */
	if(!a) return b ? -1 : 0;
	else if(!b) return 1;
	if(a->size > b->size) {
		for(ia = a->data, ib = b->data, end = ib + b->size; ib < end;
			ia++, ib++) if((diff = PZ_(compare)(ia, ib))) return diff;
		return 1;
	} else {
		for(ia = a->data, ib = b->data, end = ia + a->size; ia < end;
			ia++, ib++) if((diff = PZ_(compare)(ia, ib))) return diff;
		return -(a->size != b->size);
	}
}

/** `a` should be partitioned true/false with less-then `value`.
 @return The first index of `a` that is not less than `value`.
 @order \O(log `a.size`) @allow */
static size_t Z_(lower_bound)(const PZ_(box) *const a,
	const PA_(type) *const value) {
	size_t low = 0, high = a->size, mid;
	assert(a && value);
	while(low < high) if(PZ_(compare)(value, a->data
		+ (mid = low + ((high - low) >> 1))) <= 0) high = mid;
		else low = mid + 1;
	return low;
}

/** `a` should be partitioned false/true with greater-than or equals `value`.
 @return The first index of `a` that is greater than `value`.
 @order \O(log `a.size`) @allow */
static size_t Z_(upper_bound)(const PZ_(box) *const a,
	const PA_(type) *const value) {
	size_t low = 0, high = a->size, mid;
	assert(a && value);
	while(low < high) if(PZ_(compare)(value, a->data
		+ (mid = low + ((high - low) >> 1))) >= 0) low = mid + 1;
		else high = mid;
	return low;
}

/** Copies `datum` at the lower bound of a sorted `a`.
 @return Success. @order \O(`a.size`) @throws[realloc, ERANGE]
 @fixme No, have it insert `n` from the buffer space. */
static int Z_(insert)(PZ_(box) *const a,
	const PA_(type) *const datum) {
	size_t bound;
	assert(a && datum);
	bound = Z_(lower_bound)(a, datum); /* @fixme: shouldn't it be upper? */
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

#ifdef BOX_CONTIGUOUS_SIZE /* <!-- size */

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PZ_(vcompar)(const void *const a, const void *const b)
	{ return PZ_(compare)(a, b); }

/** Sorts `a` by `qsort` on `ARRAY_COMPARE`.
 @order \O(`a.size` \log `a.size`) @allow */
static void Z_(sort)(PZ_(box) *const a)
	{ assert(a), qsort(a->data, a->size, sizeof *a->data, PZ_(vcompar)); }

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PZ_(vrevers)(const void *const a, const void *const b)
	{ return PZ_(compare)(b, a); }

/** Sorts `a` in reverse by `qsort` on `ARRAY_COMPARE`.
 @order \O(`a.size` \log `a.size`) @allow */
static void Z_(reverse)(PZ_(box) *const a)
	{ assert(a), qsort(a->data, a->size, sizeof *a->data, PZ_(vrevers)); }

#endif /* size --> */

/** !compare(`a`, `b`) == equals(`a`, `b`) for not `ARRAY_IS_EQUAL`.
 @implements <typedef:<PZ>bipredicate_fn> */
static int PZ_(is_equal)(const PZ_(type) *const a, const PZ_(type) *const b)
	{ return !PZ_(compare)(a, b); }

#else /* compare --><!-- is equal */

/* Check that `ARRAY_IS_EQUAL` is a function implementing
 <typedef:<PZ>bipredicate_fn>. */
static const PZ_(bipredicate_fn) PZ_(is_equal) = (ARRAY_IS_EQUAL);

#endif /* is equal --> */

/** @return If `a` piecewise equals `b`, which both can be null.
 @order \O(`size`) */
static int Z_(is_equal)(const PZ_(box) *const a, const PZ_(box) *const b) {
	const PZ_(type) *ia, *ib;
	if(!a) return !b;
	if(!b || a->size != b->size) return 0;
	assert(0);
	/* Use iterate.
	 for(ia = a->data, ib = b->data, end = ia + a->size; ia < end; ia++, ib++)
		if(!PZ_(is_equal)(a, b)) return 0;*/
	return 1;
}

/** Removes consecutive duplicate elements in `a`.
 @param[merge] Controls surjection. Called with duplicate elements, if false
 `(x, y)->(x)`, if true `(x,y)->(y)`. More complex functions, `(x, y)->(x+y)`
 can be simulated by mixing the two in the value returned. Can be null: behaves
 like false. @order \O(`a.size` \times `merge`) @allow */
static void Z_(unique_merge)(PZ_(box) *const a, const PZ_(biaction_fn) merge) {
	assert(a && merge);
	if(a) assert(0);
#if 0
	size_t target, from, cursor, choice, next, move;
	const size_t last = a->size;
	int is_first, is_last;
	assert(a);
	for(target = from = cursor = 0; cursor < last; cursor += next) {
		/* Bijective `[from, cursor)` is moved lazily. */
		for(choice = 0, next = 1; cursor + next < last && PTC_(is_equal)(a->data
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
#endif
}

/** Removes consecutive duplicate elements in `a`. @order \O(`a.size`) @allow */
static void Z_(unique)(PZ_(box) *const a) { Z_(unique_merge)(a, 0); }

static void PZ_(unused_compare_coda)(void);
static void PZ_(unused_compare)(void) {
#ifdef ARRAY_COMPARE /* <!-- compare */
	Z_(compare)(0, 0); Z_(lower_bound)(0, 0); Z_(upper_bound)(0, 0);
	Z_(insert)(0, 0);
#ifdef BOX_CONTIGUOUS_SIZE /* <!-- size */
	Z_(sort)(0); Z_(reverse)(0);
#endif /* size --> */
#endif /* compare --> */
	Z_(is_equal)(0, 0); Z_(unique_merge)(0, 0); Z_(unique)(0);
	PZ_(unused_compare_coda)(); }
static void PZ_(unused_compare_coda)(void) { PZ_(unused_compare)(); }

#ifdef BOX_COMPARE
#undef BOX_COMPARE
#endif
#ifdef BOX_IS_EQUAL
#undef BOX_IS_EQUAL
#endif
#ifdef BOX_COMPARE_NAME
#undef BOX_COMPARE_NAME
#endif
#undef PZ_
#undef Z_

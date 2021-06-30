/* @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Compare Trait

 @param[BOX_, BOX_CONTAINER, BOX_CONTENTS]
 A type that represents the box and the type that goes in the box. Does not
 undefine.

 @param[Z_]
 A one-argument macro producing a name that is responsible for the name of the
 functions. Does not undefine.

 @param[FUNCTION_COMPARABLE_NAME, FUNCTION_IS_EQUAL, FUNCTION_COMPARE]
 Optional unique name `<C>` that satisfies `C` naming conventions when mangled,
 and a function implementing, for `FUNCTION_IS_EQUAL`
 <typedef:<PA>bipredicate_fn> that establishes an equivalence relation, or for
 `FUNCTION_COMPARE` <typedef:<PA>compare_fn> that establishes a total order.
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

#ifdef ARRAY_COMPARE /* <!-- compare */

#if 0
/* Check that `ARRAY_COMPARE` is a function implementing
 <typedef:<PA>compare>. */
static const PA_(compare_fn) PTC_(compare) = (ARRAY_COMPARE);

/** Lexicographically compares `a` to `b`, which both can be null.
 @return { `a < b`: negative, `a == b`: zero, `a > b`: positive }.
 @order \O(`a.size`) @allow */
static int T_C_(array, compare)(const struct A_(array) *const a,
	const struct A_(array) *const b) {
	PA_(type) *ia, *ib, *end;
	int diff;
	/* Null counts as `-\infty`. */
	if(!a) return b ? -1 : 0;
	else if(!b) return 1;
	if(a->size > b->size) {
		for(ia = a->data, ib = b->data, end = ib + b->size; ib < end;
			ia++, ib++) if((diff = PTC_(compare)(ia, ib))) return diff;
		return 1;
	} else {
		for(ia = a->data, ib = b->data, end = ia + a->size; ia < end;
			ia++, ib++) if((diff = PTC_(compare)(ia, ib))) return diff;
		return -(a->size != b->size);
	}
}

/** `a` should be partitioned true/false with less-then `value`.
 @return The first index of `a` that is not less than `value`.
 @order \O(log `a.size`) @allow */
static size_t T_C_(array, lower_bound)(const struct A_(array) *const a,
	const PA_(type) *const value) {
	size_t low = 0, high = a->size, mid;
	assert(a && value);
	while(low < high) if(PTC_(compare)(value, a->data
		+ (mid = low + ((high - low) >> 1))) <= 0) high = mid;
		else low = mid + 1;
	return low;
}

/** `a` should be partitioned false/true with greater-than or equals `value`.
 @return The first index of `a` that is greater than `value`.
 @order \O(log `a.size`) @allow */
static size_t T_C_(array, upper_bound)(const struct A_(array) *const a,
	const PA_(type) *const value) {
	size_t low = 0, high = a->size, mid;
	assert(a && value);
	while(low < high) if(PTC_(compare)(value, a->data
		+ (mid = low + ((high - low) >> 1))) >= 0) low = mid + 1;
		else high = mid;
	return low;
}

/** Copies `datum` at the lower bound of a sorted `a`. @fixme shouldn't up be
 the upper-bound?
 @return Success. @order \O(`a.size`) @throws[realloc, ERANGE] */
static int T_C_(array, insert)(struct A_(array) *const a,
	const PA_(type) *const datum) {
	size_t bound;
	assert(a && datum);
	bound = T_C_(array, lower_bound)(a, datum);
	if(!A_(array_new)(a)) return 0;
	memmove(a->data + bound + 1, a->data + bound,
		sizeof *a->data * (a->size - bound - 1));
	memcpy(a->data + bound, datum, sizeof *datum);
	return 1;
}

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PTC_(vcompar)(const void *const a, const void *const b)
	{ return PTC_(compare)(a, b); }

/** Sorts `a` by `qsort` on `ARRAY_COMPARE`.
 @order \O(`a.size` \log `a.size`) @allow */
static void T_C_(array, sort)(struct A_(array) *const a)
	{ assert(a), qsort(a->data, a->size, sizeof *a->data, PTC_(vcompar)); }

/** Wrapper with void `a` and `b`. @implements qsort bsearch */
static int PTC_(vrevers)(const void *const a, const void *const b)
	{ return PTC_(compare)(b, a); }

/** Sorts `a` in reverse by `qsort` on `ARRAY_COMPARE`.
 @order \O(`a.size` \log `a.size`) @allow */
static void T_C_(array, reverse)(struct A_(array) *const a)
	{ assert(a), qsort(a->data, a->size, sizeof *a->data, PTC_(vrevers)); }

/** !compare(`a`, `b`) == equals(`a`, `b`) for not `ARRAY_IS_EQUAL`.
 @implements <PA>bipredicate */
static int PTC_(is_equal)(const void *const a, const void *const b)
	{ return !PTC_(compare)(a, b); }
#endif

#else /* compare --><!-- is equal */

#if 0
/* Check that `ARRAY_IS_EQUAL` is a function implementing
 <typedef:<PA>bipredicate>. */
static const PA_(bipredicate) PTC_(is_equal) = (ARRAY_IS_EQUAL);
#endif

#endif /* is equal --> */

#if 0
/** @return If `a` piecewise equals `b`, which both can be null.
 @order \O(`size`) */
static int T_C_(array, is_equal)(const struct A_(array) *const a,
	const struct A_(array) *const b) {
	PA_(type) *ia, *ib, *end;
	if(!a) return !b;
	if(!b || a->size != b->size) return 0;
	for(ia = a->data, ib = b->data, end = ia + a->size; ia < end; ia++, ib++)
		if(!PTC_(is_equal)(a, b)) return 0;
	return 1;
}

/** Removes consecutive duplicate elements in `a`.
 @param[merge] Controls surjection. Called with duplicate elements, if false
 `(x, y)->(x)`, if true `(x,y)->(y)`. More complex functions, `(x, y)->(x+y)`
 can be simulated by mixing the two in the value returned. Can be null: behaves
 like false. @order \O(`a.size` \times `merge`) @allow */
static void T_C_(array, unique_merge)(struct A_(array) *const a,
	const PA_(biaction_fn) merge) {
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
}

/** Removes consecutive duplicate elements in `a`. @order \O(`a.size`) @allow */
static void T_C_(array, unique)(struct A_(array) *const a)
	{ T_C_(array, unique_merge)(a, 0); }

#endif

static void PZ_(unused_compare_coda)(void);
static void PZ_(unused_compare)(void) {
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

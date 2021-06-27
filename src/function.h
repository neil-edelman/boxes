/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Function Trait

 A trait relying on interface <iterate.h> and optionally <copy.h>.

 @param[BOX_NAME, BOX_TYPE]
 A type that represents the box and the type that goes in the box. Does not
 undefine.

 @param[F_]
 A one-argument macro producing a name that is responsible for the name of the
 functions.

 @param[FI_]
 A one-argument macro producing a name that is the same as has been previously
 been called on as `I_` on <iterate.h>. This is responsible for the order.

 @param[FC_]
 An optional one-argument macro producing a name that is the same as has been
 previously been called on as `C_` on <copy.h>. This is responsible for move.
 If one does not specify, than all the functions which require copying will not
 be defined.

 @param[FUNCTION_TYPE]
 A type, <typedef:<PF>type>, that .

 @std C89 */

/* Check defines. */
#if !defined(CAT) || !defined(CAT_) || !defined(BOX_) \
	|| !defined(BOX_CONTAINER) || !defined(BOX_CONTENTS) \
	|| !defined(F_)
#error Unexpected preprocessor symbols.
#endif

#define PF_(n) CAT(function, F_(n))

typedef BOX_CONTAINER PF_(box);
typedef BOX_CONTENTS PF_(type);

/** Operates by side-effects. */
typedef void (*F_(action_fn))(PF_(type) *);

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

/** Iterates through `a` and calls `predicate` until it returns true.
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

static void PF_(unused_function_coda)(void);
static void PF_(unused_function)(void)
	{ F_(any)(0, 0); PF_(unused_function_coda)(); }
static void PF_(unused_function_coda)(void) { PF_(unused_function)(); }

#undef PF_
#undef F_

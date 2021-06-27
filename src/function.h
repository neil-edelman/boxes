/* @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Function Trait

 A trait relying on interface <iterate.h> and optionally <copy.h>.

 @param[BOX_NAME]
 A type, <typedef:<PI>box>, that `ITERATE_BEGIN` converts into an
 <typedef:<PI>iterator>. Does not undefine.

 @param[BOX_TYPE]
 A type, <typedef:<PI>type>, that is the return from `ITERATE_NEXT`. Does not
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
#if !defined(CAT) || !defined(CAT_) || !defined(F_) || !defined(FI_)
#error Unexpected preprocessor symbols.
#endif

#define PF_(n) CAT(function, F_(n))
#define PFI_(n) CAT(iterate, FI_(n)) /* Same as in <iterate.h>. */

/** Iterates through `a` and calls `predicate` until it returns true.
 @return The first `predicate` that returned true, or, if the statement is
 false on all, null. @order \O(`a.size` \times `predicate`) @allow */
static PA_(type) *A_(array_any)(const struct A_(array) *const a,
	const PA_(predicate_fn) predicate) {
	PA_(type) *i, *i_end;
	if(!a || !predicate) return 0;
	for(i = a->data, i_end = i + a->size; i < i_end; i++)
		if(predicate(i)) return i;
	return 0;
}


static const char *Z_(to_string)(const PZI_(box) *const box) {
}

static void PZ_(unused_to_string_coda)(void);
static void PZ_(unused_to_string)(void)
	{ Z_(to_string)(0); PZ_(unused_to_string_coda)(); }
static void PZ_(unused_to_string_coda)(void) { PZ_(unused_to_string)(); }

#undef F_
#undef FI_
#ifdef FC_
#undef FC_
#endif

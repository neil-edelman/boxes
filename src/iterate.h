/* @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Iterate Interface

 This is an interface for a strict well-ordering that defines an
 <typedef:<PI>iterator> and functions <typedef:<PI>begin_fn> and
 <typedef:<PI>next_fn>.

 @param[BOX_NAME, BOX_TYPE]
 A type that represents the box and the type that goes in the box. Does not
 undefine.

 @param[I_]
 A one-argument macro producing a name that defines the prefix of the interface.

 @param[ITERATE_TYPE]
 A type, <typedef:<PI>iterator>, that contains all the iteration parameters.

 @param[ITERATE_BEGIN]
 A function satisfying <typedef:<PI>begin_fn> that takes the <typedef:<PI>box>
 and fills the <typedef:<PI>iterator> with initial values.

 @param[ITERATE_NEXT]
 A function satisfying <typedef:<PI>next_fn> that takes the
 <typedef:<PI>iterator>, modifies it, and returns the next <typedef:<PI>type>
 or null on end.

 @fixme Add a `<PI>remove` function.
 @std C89 */

/* Check defines. */
#if !defined(CAT) || !defined(CAT_) || !defined(I_) || defined(PI_) \
	|| !defined(ITERATE_TYPE) || !defined(BOX_NAME) || !defined(BOX_TYPE) \
	|| !defined(ITERATE_BEGIN) || !defined(ITERATE_NEXT)
#error Unexpected preprocessor symbols.
#endif

#define PI_(n) CAT(iterate, I_(n))

typedef ITERATE_TYPE PI_(iterator);
typedef BOX_NAME PI_(box);
typedef BOX_TYPE PI_(type);
typedef void (*PI_(begin_fn))(PI_(iterator) *, const PI_(box) *);
typedef const PI_(type) *(*PI_(next_fn))(PI_(iterator) *);

static const PI_(begin_fn) PI_(begin) = (ITERATE_BEGIN);
static const PI_(next_fn) PI_(next) = (ITERATE_NEXT);

#undef PI_
#undef I_
#undef ITERATE_TYPE
#undef ITERATE_BEGIN
#undef ITERATE_NEXT

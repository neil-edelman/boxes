/* @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Iterate

 `I_`

 The inclusion must define an iterator, ITERATE, ITERATE_BOX, ITERATE_TYPE,
 ITERATE_BEGIN, and ITERATE_NEXT.

 @param[Z_]
 Function-like define macro accepting one argument and producing a valid name.
 Defines `PI_` to be private.

 @std C89 */

/* Check defines. */
#if !defined(CAT) || !defined(CAT_) || !defined(I_) || defined(PI_) \
	|| !defined(ITERATE) || !defined(ITERATE_BOX) || !defined(ITERATE_TYPE) \
	|| !defined(ITERATE_BEGIN) || !defined(ITERATE_NEXT) \
#error Unexpected preprocessor symbols.
#endif

#define PI_(thing) CAT(iterate, I_(thing))

typedef ITERATE PI_(iterator);
typedef ITERATE_BOX PI_(box);
typedef ITERATE_TYPE PI_(type);
typedef void (*PI_(begin_fn))(PI_(iterator) *, const PI_(box) *);
typedef const PI_(type) *(*PI_(next_fn))(PI_(iterator) *);

static const PI_(begin_fn) PI_(begin) = (ITERATE_BEGIN);
static const PI_(next_fn) PI_(next) = (ITERATE_NEXT);

#undef PI_
#undef ITERATE
#undef ITERATE_BOX
#undef ITERATE_TYPE
#undef ITERATE_BEGIN
#undef ITERATE_NEXT

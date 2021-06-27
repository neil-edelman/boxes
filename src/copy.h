/* @license 2021 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Copy Interface

 This will emit a macro `COPY_H` that will have to be undefined at the end.
 
 @param[C_]
 A one-argument macro producing a name that is responsible for the name of the
 copy interface.

 @param[COPY_TYPE]
 A type, <typedef:<PC>type>, that is used in <typedef:<PC>copy_fn>.

 @param[COPY_FN, COPY_MOVE_FN]
 Functions satisfying <typedef:<PC>copy_fn>, which obey the same semantics as
 `memcpy` and `memmove`

 @std C89 */

#include <stddef.h>

/* Check defines. */
#if !defined(CAT) || !defined(CAT_) || !defined(C_) || defined(PC_) \
	|| !defined(COPY_TYPE) || !defined(COPY_FN) || !defined(COPY_MOVE_FN)
#error Unexpected preprocessor symbols.
#endif
#ifdef COPY_H
#error Move semantics already defined.
#endif

#define COPY_H

#define PC_(n) CAT(copy, C_(n))

typedef COPY_TYPE PC_(type);

/** Copy `n` items from the object pointed to by `src` into the object pointed
 to by `dest`. */
typedef void (*PC_(copy_fn))(PC_(type) *dest, const PC_(type) *src, size_t n);

/** `<PC>copy` copies non-overlapping (restricted) data, and `<PC>move` copies
 without this restriction. */
static const PC_(copy_fn) PC_(copy) = (COPY_FN), PC_(move) = (COPY_MOVE_FN);

#undef PC_
#undef C_
#undef COPY_TYPE
#undef COPY_FN
#undef COPY_MOVE_FN

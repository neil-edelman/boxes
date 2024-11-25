/* It's more convenient to have implicit `ARRAY_DECLARE_ONLY` unless otherwise
 stated. The definition happens once. */
#ifdef PUBLIC_ARRAY_DEFINE
#undef PUBLIC_ARRAY_DEFINE
#else
#define ARRAY_DECLARE_ONLY
#endif

#define ARRAY_NAME static
#define ARRAY_TYPE int
#define ARRAY_TEST
#define ARRAY_COMPARE
#define ARRAY_TO_STRING
#include "../src/array.h"

/* Encase `static_array` in `public_array`. Could just use
 `static_array public_array()` but that's ugly. I guess `static_array` is
 also public, but one can't use the functions outside of the translation unit.
 Then the name won't be the same. */
struct public_array { struct static_array _; };

/* All the functions are static, so we must build wrapper functions on all that
 we need. This is tedious and fragile, so if you have any ideas… */
struct public_array public_array(void);
void public_array_(struct public_array *);
void public_array_test(void);

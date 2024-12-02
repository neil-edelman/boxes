/* This header is for potentially multiple files, but only one can contain the
 definition. All others just have declarations by `ARRAY_DECLARE_ONLY`. It's
 more convenient to have implicit declarations unless otherwise stated. We make
 up a preprocessor variable that flips them. */
#ifdef HEADER_ARRAY_DEFINE
#undef HEADER_ARRAY_DEFINE
#else
#define ARRAY_DECLARE_ONLY
#endif

#define ARRAY_NAME static
#define ARRAY_TYPE int
#define ARRAY_TEST
#define ARRAY_COMPARE
#define ARRAY_TO_STRING
#include "../src/array.h"

/* Encase `static_array` in `header_array`. Could just use
 `static_array header_array()` but that's ugly. */
struct header_array { struct static_array _; };

/* All the functions are static, so we must build wrapper functions on all that
 we need. This is tedious and fragile, so if you have any ideas… */
struct header_array header_array(void);
void header_array_(struct header_array *);
void header_array_test(void);

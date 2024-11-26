/* This header is for potentially multiple files, but only one can contain the
 definition. All others just have declarations by `BOX_DECLARE_ONLY`. It's
 more convenient to have implicit declarations unless otherwise stated. We make
 up a preprocessor variable that flips them. */
#ifdef INTEGER_ARRAY_DEFINE
#undef INTEGER_ARRAY_DEFINE
#else
#define BOX_DECLARE_ONLY
#endif

#define BOX_NAME static
#define BOX_TYPE int
#define BOX_TEST
#define BOX_COMPARE
#define BOX_TO_STRING
#include "../src/array.h"

/* Encase `static_array` in `integer_array`. Could just use
 `static_array integer_array()` but that's ugly. */
struct integer_array { struct static_array _; };

/* All the functions are static, so we must build wrapper functions on all that
 we need. This is tedious and fragile, so if you have any ideas… */
struct integer_array integer_array(void);
void integer_array_(struct integer_array *);
void integer_array_test(void);

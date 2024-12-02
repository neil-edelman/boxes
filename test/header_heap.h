#ifdef HEADER_HEAP_DEFINE
#undef HEADER_HEAP_DEFINE
#else
#define HEAP_DECLARE_ONLY
#endif

#define ARRAY_NAME static
#define ARRAY_TYPE int
#define ARRAY_TEST
#define ARRAY_COMPARE
#define ARRAY_TO_STRING
#include "../src/array.h"

/* Encase `static_array` in `integer_array`. Could just use
 `static_array integer_array()` but that's ugly. */
struct integer_array { struct static_array _; };

/* All the functions are static, so we must build wrapper functions on all that
 we need. This is tedious and fragile, so if you have any ideas… */
struct integer_array integer_array(void);
void integer_array_(struct integer_array *);
void integer_array_test(void);

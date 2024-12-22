#ifdef DEFINE
#	undef DEFINE
#else
#	define HEAP_DECLARE_ONLY
#endif
#define HEAP_NAME header
#define HEAP_TYPE int
#define HEAP_TEST
#define HEAP_NON_STATIC
#define HEAP_TO_STRING
#include "../src/heap.h"

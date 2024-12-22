#ifdef DEFINE
#	undef DEFINE
#else
#	define HEAP_DECLARE_ONLY
#endif
#define HEAP_NAME static
#define HEAP_TYPE int
#define HEAP_TEST
#define HEAP_TO_STRING
#include "../src/heap.h"

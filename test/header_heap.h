#ifdef HEADER_HEAP_DEFINE
#undef HEADER_HEAP_DEFINE
#else
#define HEAP_DECLARE_ONLY
#endif

#define HEAP_NAME static
#define HEAP_TYPE int
#define HEAP_TEST
#define HEAP_TO_STRING
#include "../src/heap.h"

struct header_heap { struct static_heap _; };

struct header_heap header_heap(void);
void header_heap_(struct header_heap *);
void header_heap_test(void);

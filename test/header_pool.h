#ifdef HEADER_POOL_DEFINE
#	undef HEADER_POOL_DEFINE
#else
#	define HEAP_DECLARE_ONLY
#endif

#define POOL_NAME static
#define POOL_TYPE enum colour
#define POOL_TO_STRING
#define POOL_TEST
#include "../src/pool.h"

struct header_pool { struct static_pool _; };

struct header_pool header_pool(void);
void header_pool_(struct header_pool *);
void header_pool_test(void);

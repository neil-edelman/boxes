#ifdef DEFINE
#	undef DEFINE
#else
#	define POOL_DECLARE_ONLY
#endif
#define POOL_NAME static
#define POOL_TYPE enum colour
#define POOL_TO_STRING
#define POOL_TEST
#include "../src/pool.h"

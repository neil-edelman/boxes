#ifdef DEFINE
#	undef DEFINE
#else
#	define DEQUE_DECLARE_ONLY
#endif
#define DEQUE_NAME header
#define DEQUE_TYPE int
#define DEQUE_TEST
#define DEQUE_TO_STRING
#define DEQUE_NON_STATIC
#include "../src/deque.h"

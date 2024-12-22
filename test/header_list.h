#ifdef DEFINE
#	undef DEFINE
#else
#	define LIST_DECLARE_ONLY
#endif
#define LIST_NAME header
#define LIST_TEST
#define LIST_TO_STRING
#define LIST_NON_STATIC
#include "../src/list.h"

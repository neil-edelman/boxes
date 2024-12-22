#ifdef DEFINE
#	undef DEFINE
#else
#	define TREE_DECLARE_ONLY
#endif
#define TREE_NAME static
#define TREE_KEY char
#define TREE_DEFAULT '_'
#define TREE_TO_STRING
#define TREE_TEST
#include "../src/tree.h"

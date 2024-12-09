#ifdef HEADER_TREE_DEFINE
#	undef HEADER_TREE_DEFINE
#else
#	define TREE_DECLARE_ONLY
#endif

#define TREE_NAME static
#define TREE_KEY char
#define TREE_DEFAULT '_'
#define TREE_TO_STRING
#define TREE_TEST
#include "../src/tree.h"

struct header_tree { struct static_tree _; };

struct header_tree header_tree(void);
void header_tree_(struct header_tree *);
void header_tree_test(void);

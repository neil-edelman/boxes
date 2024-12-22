#ifdef DEFINE
#	undef DEFINE
#else
#	define TRIE_DECLARE_ONLY
#endif
#define TRIE_NAME header
#define TRIE_KEY enum colour
#define TRIE_TO_STRING
#define TRIE_TEST
#include "../src/trie.h"

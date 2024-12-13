#ifdef HEADER_TRIE_DEFINE
#	undef HEADER_TRIE_DEFINE
#else
#	define TRIE_DECLARE_ONLY
#endif

/* Public separating header/body. */
#define TRIE_NAME static
#define TRIE_KEY enum colour
#define TRIE_TO_STRING
#define TRIE_TEST
#include "../src/trie.h"

struct header_trie { struct static_trie _; };

struct header_trie header_trie(void);
void header_trie_(struct header_trie *);
void header_trie_test(void);

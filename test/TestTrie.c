/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 Test Trie.

 @std C89/90 */

#include <stdlib.h> /* EXIT malloc free rand */
#include <stdio.h>  /* *printf */
#include <assert.h> /* assert */
#include <errno.h>  /* errno */
#include <string.h> /* strncpy */
#include "Orcish.h"

extern const char *const parole;
size_t parole_size = sizeof parole / sizeof *parole;

static void fill_str(const char *str) {
	/* nothing */ (void)(str);
}

#define TRIE_NAME Str
#define TRIE_TEST &fill_str
#include "../src/Trie.h"

static int test(void) {
	struct StrTrie trie = TRIE_IDLE, ingleshi = TRIE_IDLE;
	size_t i;
	union trie_Str_TrieNode *n;

	printf("TrieInternal %lu\n"
		"size_t %lu\n"
		"Type * %lu\n"
		"union %lu\n",
		sizeof(struct TrieInternal),
		sizeof(size_t),
		sizeof(trie_Str_Type *),
		sizeof(union trie_Str_TrieNode));

	/* fixme: <N>TrieAdd(struct N_(Trie) *, PN_(Type) *) fails if present. */
	
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie0.gv");
	/*printf("Trie0: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTriePut(&trie, "foo", 0)) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie1.gv");
	/*printf("Trie1: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTriePut(&trie, "bar", 0)) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie2.gv");
	/*printf("Trie2: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTriePut(&trie, "baz", 0)) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie3.gv");
	/*printf("Trie3: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTriePut(&trie, "qux", 0)) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie4.gv");
	/*printf("Trie4: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTriePut(&trie, "quxx", 0)) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie5.gv");
	/*printf("Trie5: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTriePut(&trie, "quxxx", 0)) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie6.gv");
	/*printf("Trie6: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTriePut(&trie, "a", 0)) goto catch;
	trie_Str_graph(&trie, "graph/trie_a.gv");
	if(!StrTriePut(&trie, "b", 0)) goto catch;
	trie_Str_graph(&trie, "graph/trie_b.gv");
	trie_Str_print(&trie);
	if(!StrTriePut(&trie, "c", 0)) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie_c.gv");
	if(!StrTriePut(&trie, "d", 0)
		|| !StrTriePut(&trie, "e", 0)
		|| !StrTriePut(&trie, "f", 0)
		|| !StrTriePut(&trie, "g", 0)
		|| !StrTriePut(&trie, "h", 0)
		|| !StrTriePut(&trie, "i", 0)
		|| !StrTriePut(&trie, "j", 0)
		|| !StrTriePut(&trie, "k", 0)
		|| !StrTriePut(&trie, "l", 0)
		|| !StrTriePut(&trie, "m", 0)
		|| !StrTriePut(&trie, "n", 0)
		|| !StrTriePut(&trie, "o", 0)
		|| !StrTriePut(&trie, "p", 0)
		|| !StrTriePut(&trie, "q", 0)
		|| !StrTriePut(&trie, "r", 0)
		|| !StrTriePut(&trie, "s", 0)
		|| !StrTriePut(&trie, "t", 0)
		|| !StrTriePut(&trie, "u", 0)
		|| !StrTriePut(&trie, "v", 0)
		|| !StrTriePut(&trie, "w", 0)
		|| !StrTriePut(&trie, "x", 0)
		|| !StrTriePut(&trie, "y", 0)
		|| !StrTriePut(&trie, "z", 0)) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie_z.gv");

	n = trie_Str_match(&trie, "");
	printf("\"\": %s\n", n ? n->leaf : "null");	
	n = trie_Str_match(&trie, "foo");
	printf("\"foo\": %s\n", n ? n->leaf : "null");	
	n = trie_Str_match(&trie, "qux");
	printf("\"qux\": %s\n", n ? n->leaf : "null");	
	n = trie_Str_match(&trie, "quxx");
	printf("\"quxx\": %s\n", n ? n->leaf : "null");	
	n = trie_Str_match(&trie, "quux");
	printf("\"quux\": %s\n", n ? n->leaf : "null");	

	printf("parole %lu\n", parole_size);

	for(i = 0; i < parole_size / 3000; i++) {
		const char *eject;
		if(!StrTriePut(&trie, parole[i], &eject)) goto catch;
	}

	printf("Test passed.\n");
	goto finally;
catch:
	printf("Test failed.\n");
finally:
	StrTrie_(&ingleshi);
	StrTrie_(&trie);
	return 1;
}

int main(void) {
	return test() ? EXIT_SUCCESS : EXIT_FAILURE;
}

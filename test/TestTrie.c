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

static void fill_str(const char *str) {
	/* nothing */ (void)(str);
}

#define TRIE_NAME Str
#define TRIE_TEST &fill_str
#include "../src/Trie.h"

static int test(void) {
	struct StrTrie trie = TRIE_IDLE;
	union trie_Str_TrieNode *n;

	printf("TrieInternal %lu\n"
		"size_t %lu\n"
		"Type * %lu\n"
		"union %lu\n",
		sizeof(struct TrieInternal),
		sizeof(size_t),
		sizeof(trie_Str_Type *),
		sizeof(union trie_Str_TrieNode));

	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie0.gv");
	/*printf("Trie0: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTriePut(&trie, "foo")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie1.gv");
	/*printf("Trie1: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTriePut(&trie, "bar")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie2.gv");
	/*printf("Trie2: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTriePut(&trie, "baz")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie3.gv");
	/*printf("Trie3: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTriePut(&trie, "qux")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie4.gv");
	/*printf("Trie4: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTriePut(&trie, "quxx")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie5.gv");
	/*printf("Trie5: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTriePut(&trie, "quxxx")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie6.gv");
	/*printf("Trie6: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTriePut(&trie, "a")) goto catch;
	trie_Str_graph(&trie, "graph/trie_a.gv");
	if(!StrTriePut(&trie, "b")) goto catch;
	trie_Str_graph(&trie, "graph/trie_b.gv");
	trie_Str_print(&trie);
	if(!StrTriePut(&trie, "c")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie_c.gv");
	if(!StrTriePut(&trie, "d")
		|| !StrTriePut(&trie, "e")
		|| !StrTriePut(&trie, "f")
		|| !StrTriePut(&trie, "g")
		|| !StrTriePut(&trie, "h")
		|| !StrTriePut(&trie, "i")
		|| !StrTriePut(&trie, "j")
		|| !StrTriePut(&trie, "k")
		|| !StrTriePut(&trie, "l")
		|| !StrTriePut(&trie, "m")
		|| !StrTriePut(&trie, "n")
		|| !StrTriePut(&trie, "o")
		|| !StrTriePut(&trie, "p")
		|| !StrTriePut(&trie, "q")
		|| !StrTriePut(&trie, "r")
		|| !StrTriePut(&trie, "s")
		|| !StrTriePut(&trie, "t")
		|| !StrTriePut(&trie, "u")
		|| !StrTriePut(&trie, "v")
		|| !StrTriePut(&trie, "w")
		|| !StrTriePut(&trie, "x")
		|| !StrTriePut(&trie, "y")
		|| !StrTriePut(&trie, "z")) goto catch;
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

	printf("Test passed.\n");
	goto finally;
catch:
	printf("Test failed.\n");
finally:
	StrTrie_(&trie);
	return 1;
}

int main(void) {
	return test() ? EXIT_SUCCESS : EXIT_FAILURE;
}

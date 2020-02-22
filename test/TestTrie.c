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

	trie_Str_print(&trie);
	printf("Trie: %s.\n", StrTrieToString(&trie));
	trie_Str_graph(&trie, "graph/trie0.gv");

	if(!StrTrieAdd(&trie, "foo")) goto catch;
	trie_Str_print(&trie);
	printf("Trie: %s.\n", StrTrieToString(&trie));
	trie_Str_graph(&trie, "graph/trie1.gv");

	if(!StrTrieAdd(&trie, "bar")) goto catch;
	trie_Str_print(&trie);
	printf("Trie: %s.\n", StrTrieToString(&trie));
	trie_Str_graph(&trie, "graph/trie2.gv");
	if(!StrTrieAdd(&trie, "baz")) goto catch;
	trie_Str_print(&trie);
	/*printf("%s.\n", StrTrieToString(&trie));
	trie_Str_graph(&trie, "graph/trie3.gv");*/
	if(!StrTrieAdd(&trie, "qux")) goto catch;
	trie_Str_print(&trie);
	/*printf("%s.\n", StrTrieToString(&trie));
	trie_Str_graph(&trie, "graph/trie4.gv");*/
	if(!StrTrieAdd(&trie, "quxx")) goto catch;
	trie_Str_print(&trie);
	/*printf("%s.\n", StrTrieToString(&trie));
	trie_Str_graph(&trie, "graph/trie5.gv");*/
	if(!StrTrieAdd(&trie, "quxxx")) goto catch;
	trie_Str_print(&trie);
	/*printf("%s.\n", StrTrieToString(&trie));
	trie_Str_graph(&trie, "graph/trie6.gv");*/
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

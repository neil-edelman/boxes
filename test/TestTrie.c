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

extern const char *const parole[];
extern const size_t parole_size;

static void fill_str(const char *str) {
	/* nothing */ (void)(str);
}

#define TRIE_NAME Str
#define TRIE_TEST &fill_str
#include "../src/Trie.h"

static int test(void) {
	struct StrTrie trie = TRIE_IDLE, inglesi = TRIE_IDLE;
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
	
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie0.gv");
	/*printf("Trie0: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "foo")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie1.gv");
	/*printf("Trie1: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "bar")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie2.gv");
	/*printf("Trie2: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "baz")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie3.gv");
	/*printf("Trie3: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "qux")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie4.gv");
	/*printf("Trie4: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "quxx")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie5.gv");
	/*printf("Trie5: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "quxxx")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie6.gv");
	/*printf("Trie6: %s.\n\n", StrTrieToString(&trie));*/

	if(!StrTrieAdd(&trie, "a")) goto catch;
	trie_Str_graph(&trie, "graph/trie_a.gv");
	if(!StrTrieAdd(&trie, "b")) goto catch;
	trie_Str_graph(&trie, "graph/trie_b.gv");
	trie_Str_print(&trie);
	if(!StrTrieAdd(&trie, "c")) goto catch;
	trie_Str_print(&trie);
	trie_Str_graph(&trie, "graph/trie_c.gv");
	if(!StrTrieAdd(&trie, "d")
		|| !StrTrieAdd(&trie, "e")
		|| !StrTrieAdd(&trie, "f")
		|| !StrTrieAdd(&trie, "g")
		|| !StrTrieAdd(&trie, "h")
		|| !StrTrieAdd(&trie, "i")
		|| !StrTrieAdd(&trie, "j")
		|| !StrTrieAdd(&trie, "k")
		|| !StrTrieAdd(&trie, "l")
		|| !StrTrieAdd(&trie, "m")
		|| !StrTrieAdd(&trie, "n")
		|| !StrTrieAdd(&trie, "o")
		|| !StrTrieAdd(&trie, "p")
		|| !StrTrieAdd(&trie, "q")
		|| !StrTrieAdd(&trie, "r")
		|| !StrTrieAdd(&trie, "s")
		|| !StrTrieAdd(&trie, "t")
		|| !StrTrieAdd(&trie, "u")
		|| !StrTrieAdd(&trie, "v")
		|| !StrTrieAdd(&trie, "w")
		|| !StrTrieAdd(&trie, "x")
		|| !StrTrieAdd(&trie, "y")
		|| !StrTrieAdd(&trie, "z")) goto catch;
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

	printf("parole_size %lu\n", (unsigned long)parole_size);
	for(i = 0; i < parole_size; i++)
		if(!StrTriePut(&inglesi, parole[i], 0)) goto catch;
	printf("ingesi.size %lu; %s.\n", (unsigned long)StrTrieSize(&inglesi),
		StrTrieToString(&inglesi));
	/*trie_Str_graph(&inglesi, "graph/inglesi.gv"); -- 31MB. */
	/*trie_Str_print(&inglesi);*/

	printf("Test passed.\n");
	goto finally;
catch:
	printf("Test failed.\n");
finally:
	StrTrie_(&inglesi);
	StrTrie_(&trie);
	return 1;
}

int main(void) {
	return test() ? EXIT_SUCCESS : EXIT_FAILURE;
}

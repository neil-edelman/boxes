/** @license 2020 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 Test Trie.

 @std C89/90 */

#include <stdlib.h> /* EXIT malloc free rand */
#include <stdio.h>  /* *printf */
#include <assert.h> /* assert */
#include <errno.h>  /* errno */
#include <string.h> /* strncpy */
#include <time.h>   /* clock time */
#include "Orcish.h"

extern const char *const parole[];
extern const size_t parole_size;

static void fill_str(const char *str) {
	/* nothing */ (void)(str);
}

#define TRIE_NAME Str
#define TRIE_TEST &fill_str
#include "../src/Trie.h"



/** For comparison with linked hash. */

static int string_is_equal(const char *const a, const char *const b) {
	return !strcmp(a, b);
}
static void pstring_to_string(const char *const*const ps, char (*const a)[12]) {
	strncpy(*a, *ps, sizeof(*a) - 1);
	(*a)[sizeof(*a) - 1] = '\0';
}
/** Perform a 32 bit
 [Fowler/Noll/Vo FNV-1a](http://www.isthe.com/chongo/tech/comp/fnv/) hash on
 `str`. */
static unsigned fnv_32a_str(const char *const str) {
	const unsigned char *s = (const unsigned char *)str;
	/* 32 bit FNV-1 and FNV-1a non-zero initial basis, `FNV1_32A_INIT`. */
	unsigned hval = 0x811c9dc5;
	/* FNV magic prime `FNV_32_PRIME 0x01000193`. */
	while(*s) {
		hval ^= *s++;
		hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
	}
	return hval;
}
#define SET_NAME Str
#define SET_TYPE const char *
#define SET_HASH &fnv_32a_str
#define SET_IS_EQUAL &string_is_equal
#define SET_TO_STRING &pstring_to_string
#include "Set.h"

struct StrListNode;
static int str_list_compare(const struct StrListNode *,
	const struct StrListNode *);
static void str_list_to_string(const struct StrListNode *, char (*)[12]);
#define LIST_NAME Str
#define LIST_COMPARE &str_list_compare
#define LIST_TO_STRING &str_list_to_string
#include "List.h"

struct Entry {
	struct StrSetElement elem;
	struct StrListNode node;
	char str[24];
};
static struct Entry *elem_upcast(struct StrSetElement *const elem) {
	return (struct Entry *)(void *)((char *)elem
		- offsetof(struct Entry, elem));
}
static const struct Entry *node_upcast_c(const struct StrListNode *const node) {
	return (const struct Entry *)(const void *)((const char *)node
		- offsetof(struct Entry, node));
}
static void str_list_to_string(const struct StrListNode *s,
	char (*const a)[12]) {
	sprintf(*a, "%.11s", node_upcast_c(s)->str);
}
/** @implements <KeyListNode>Compare */
static int str_list_compare(const struct StrListNode *const a,
	const struct StrListNode *const b) {
	return strcmp(node_upcast_c(a)->elem.key, node_upcast_c(b)->elem.key);
}

#define POOL_NAME Entry
#define POOL_TYPE struct Entry
#include "Pool.h"


static void test_basic_trie_str() {
	struct StrTrie trie = TRIE_IDLE;
	union trie_Str_TrieNode *n;

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
	goto finally;
catch:
	printf("Test failed.\n");
	assert(0);
finally:
	StrTrie_(&trie);
}

static void test(void) {
	struct StrTrie trie = TRIE_IDLE;
	struct EntryPool entries = POOL_IDLE;
	struct StrSet set = SET_IDLE;
	struct StrList list;
	size_t i;
	clock_t t;
	unsigned seed = (unsigned)0/*clock()*/;

	srand(seed), rand(), printf("Seed %u.\n", seed);

	printf("TrieInternal %lu\n"
		"size_t %lu\n"
		"Type * %lu\n"
		"union <PN>TrieNode %lu\n",
		sizeof(struct TrieInternal),
		sizeof(size_t),
		sizeof(trie_Str_Type *),
		sizeof(union trie_Str_TrieNode));

	test_basic_trie_str();

	/* Trie. */

	printf("parole_size %lu\n", (unsigned long)parole_size);
	t = clock();
	for(i = 0; i < parole_size; i++)
		if(!StrTriePut(&trie, parole[i], 0)) goto catch;
	t = clock() - t;
	printf("Initialisation of trie took %fms; trie.size %lu; %s.\n",
		1000.0 / CLOCKS_PER_SEC * t, (unsigned long)StrTrieSize(&trie),
		StrTrieToString(&trie));
	t = clock();
	for(i = 0; i < parole_size; i++) {
		const char *const s = StrTrieGet(&trie, parole[i]);
		assert(s);
	}
	t = clock() - t;
	printf("Lookup of %lu trie elements took %fms.\n",
		(unsigned long)StrTrieSize(&trie), 1000.0 / CLOCKS_PER_SEC * t);
	/*trie_Str_graph(&inglesi, "graph/inglesi.gv"); -- 31MB. */
	/*trie_Str_print(&inglesi);*/
	StrTrie_(&trie);

	/* Linked set. */

	StrListClear(&list);
	t = clock();
	for(i = 0; i < parole_size; i++) {
		struct Entry *const e = EntryPoolNew(&entries);
		e->elem.key = e->str;
		strcpy(e->str, parole[i]);
		if(StrSetPolicyPut(&set, &e->elem, 0))
			{ EntryPoolRemove(&entries, e); continue; } /* Duplicate. */
		StrListPush(&list, &e->node);
	}
	StrListSort(&list);
	t = clock() - t;
	printf("Initialisation of linked set took %fms; set.size %lu; %s.\n",
		1000.0 / CLOCKS_PER_SEC * t, (unsigned long)StrSetSize(&set),
		StrListToString(&list));
	t = clock();
	for(i = 0; i < parole_size; i++) {
		struct StrSetElement *const s = StrSetGet(&set, parole[i]);
		assert(s);
	}
	t = clock() - t;
	printf("Lookup of %lu linked set elements took %fms.\n",
		(unsigned long)StrSetSize(&set), 1000.0 / CLOCKS_PER_SEC * t);
	EntryPool_(&entries);
	StrSet_(&set);

	printf("Test passed.\n");
	goto finally;
catch:
	printf("Test failed.\n");
	assert(0);
	exit(EXIT_FAILURE);
finally:
	EntryPool_(&entries);
	StrSet_(&set);
	StrTrie_(&trie);
}

int main(void) {
	test();
	return EXIT_SUCCESS;
}

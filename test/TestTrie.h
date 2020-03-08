/* Intended to be included by `Heap.h` on `TREE_TEST`. */

/* Define macros. */
#ifdef QUOTE
#undef QUOTE
#endif
#ifdef QUOTE_
#undef QUOTE_
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** @fixme */
typedef void (*PN_(Action))(PN_(Type) *);

/* `TRIE_TEST` must be a function that implements `<PN>Action`. */
static const PN_(Action) PN_(filler) = (TRIE_TEST);

/*static void PN_(print)(const struct N_(Trie) *const trie) {
	size_t i;
	printf("__print__ size %lu.\n", (unsigned long)N_(TrieSize)(trie));
	if(!trie) { printf("null\n"); goto end; }
	if(!trie->leaves.size)
		{ assert(!trie->branches.size); printf("empty\n"); goto end; }
	assert(trie->branches.size + 1 == trie->leaves.size);
	for(i = 0; i < trie->branches.size; i++)
		printf("branch%lu: bit %u, left %u.\n",
		(unsigned long)i, trie->branches.data[i].bit,
		trie->branches.data[i].left);
	for(i = 0; i < trie->leaves.size; i++)
		printf("leaf%lu:   \"%s\".\n", (unsigned long)i,
		PN_(to_key)(trie->leaves.data[i]));
end:
	printf("^^end print^^\n\n");
}*/

/** Draw a graph of `trie` to `fn` in Graphviz format. */
static void PN_(graph)(const struct N_(Trie) *const trie,
	const char *const fn) {
	FILE *fp;
	size_t i;
	assert(trie && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\trankdir = TB;\n"
		"\tnode [shape = record, style = filled];\n"
		"\tTrie [label=\"{\\<" QUOTE(TRIE_NAME) "\\>Trie: " QUOTE(TRIE_TYPE)
		"\\l|size: %lu\\l}\"];\n"
		"\tnode [fillcolor=lightsteelblue];\n",
		(unsigned long)N_(TrieSize(trie)));
	for(i = 0; i < trie->leaves.size; i++)
		fprintf(fp, "\tleaf%lu [label=\"%s\"];\n",
		(unsigned long)i, PN_(to_key)(trie->leaves.data[i]));
	for(i = 0; i < trie->branches.size; i++) {
		fprintf(fp, "\tbranch%lu [shape = \"oval\" label=\"%u\"];\n",
			(unsigned long)i, trie->branches.data[i].bit);
	}
	/*	fprintf(fp,
		"\tn%lu [shape = \"oval\" label=\"%u\"];\n"
		"\tn%lu -> n%lu [style = dashed];\n"
		"\tn%lu -> n%lu;\n",
		(unsigned long)i, trie->branches.data[i].bit,
		(unsigned long)i, (unsigned long)i + 1,
		(unsigned long)i, (unsigned long)n +);*/
	fprintf(fp, "\tnode [colour=red];\n"
		"}\n");
	fclose(fp);
}

#if 0
/** Makes sure the `heap` is in a valid state. */
static void PN_(valid)(const struct N_(Trie) *const trie) {
	if(!trie) return;
	if(!trie->a.data) { assert(!trie->a.size); return; }
	assert(trie->a.size == 1 || (trie->a.size - 1) % 3 == 0);
}

/** Will be tested on stdout. Requires `TREE_TEST`, `TREE_TO_STRING`, and not
 `NDEBUG` while defining `assert`.
 @param[param] The parameter to call <typedef:<PH>BiAction> `TREE_TEST`.
 @allow */
static void N_(TrieTest)(void *const param) {
	printf("<" QUOTE(TRIE_NAME) ">Trie"
		" of type <" QUOTE(TRIE_TYPE) ">"
		" was created using: TREE_KEY<" QUOTE(TRIE_KEY) ">;"
		" TRIE_TO_STRING <" QUOTE(TRIE_TO_STRING) ">;"
		" TRIE_TEST <" QUOTE(TRIE_TEST) ">;"
		" testing:\n");
	(void)param;
	fprintf(stderr, "Done tests of <" QUOTE(TRIE_NAME) ">Trie.\n\n");
}
#endif

/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_

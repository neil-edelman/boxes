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

/** Draw a graph of `trie` to `fn` in Graphviz format. */
static void PN_(graph)(const struct N_(Trie) *const trie,
	const char *const fn) {
	FILE *fp;
	assert(trie && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\trankdir = BT;\n"
		"\tnode [shape = record, style = filled];\n"
		"\tTrie [label=\"{\\<" QUOTE(TRIE_NAME) "\\>Hash: " QUOTE(TRIE_TYPE)
		"\\l|size: %lu\\lcapacity: %lu\\l"
		"next capacity: %lu\\l}\"];\n", (unsigned long)trie->a.size,
		(unsigned long)trie->a.capacity, (unsigned long)trie->a.next_capacity);
	if(trie->a.data) {
		union PN_(TrieNode) *n, *n_end;
		fprintf(fp, "\tnode [fillcolor=lightsteelblue];\n");
		if(trie->a.size) fprintf(fp, "\tn0 -> Hash [dir = back];\n");
		fprintf(fp, "\tedge [style = dashed];\n"
			"\tsubgraph cluster_data {\n"
			"\t\tstyle=filled;\n");
		for(n = trie->a.data, n_end = n + trie->a.size; n < n_end; n++) {
			fprintf(fp, "\t\tn%lu [label=\"%s\"];\n",
				(unsigned long)(n - trie->a.data), PN_(to_key)(n->leaf));
			/*fprintf(fp, "\t\tn%lu -> n%lu;\n", (unsigned long)i,
				(unsigned long)((i - 1) >> 1));*/
		}
		fprintf(fp, "\t}\n");
	}
	fprintf(fp, "\tnode [colour=red];\n");
	fprintf(fp, "}\n");
	fclose(fp);
}

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

/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_

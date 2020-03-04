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

static void PN_(print_leaf)(const struct N_(Trie) *const trie,
	const size_t n) {
	assert(trie && n < trie->a.size);
	printf("node%lu:   leaf, \"%s\".\n",
		(unsigned long)n, PN_(to_key)(trie->a.data[n].leaf));
}

static void PN_(print_branch)(const struct N_(Trie) *const trie,
	const size_t n) {
	assert(trie && n < trie->a.size);
	printf("node%lu: branch, bit %u, left %u.\n",
		(unsigned long)n, trie->a.data[n].branch.choice_bit,
		trie->a.data[n].branch.left_branches);
	
}

static int PN_(is_branch)(const struct N_(Trie) *const trie,
	const size_t i1) {
	size_t i0 = 0, i0_lnode, i2 = trie->a.size - 1;
	assert(trie && trie->a.size && i1 <= i2);
	while(i0 < i1) {
		i0_lnode = (((size_t)trie->a.data[i0].branch.left_branches) << 1) + 1;
		if(i1 <= i0 + i0_lnode) i2 = i0++ + i0_lnode;
		else                    i0 += i0_lnode + 1;
	}
	/*printf("  is_branch %lu:%lu:%lu %s\n", i0, i1, i2, i1 < i2 ? "yes" : "no");*/
	assert(i0 == i1);
	return i1 < i2;
}

static void PN_(print_node)(const struct N_(Trie) *const trie, const size_t n) {
	PN_(is_branch)(trie, n)
		? PN_(print_branch)(trie, n) : PN_(print_leaf)(trie, n);
}

static void PN_(print)(const struct N_(Trie) *const trie) {
	size_t i;
	printf("__print__ trie.size %lu -> %lu entries\n", trie ? trie->a.size : 0,
		(unsigned long)N_(TrieSize)(trie));
	if(!trie) { printf("null\n\n"); return; }
	if(!trie->a.size) { printf("empty\n\n"); return; }
	for(i = 0; i < trie->a.size; i++) PN_(print_node)(trie, i);
	printf("\n");
}

/** Draw a graph of `trie` to `fn` in Graphviz format. */
static void PN_(graph)(const struct N_(Trie) *const trie,
	const char *const fn) {
	const size_t size = trie->a.size;
	FILE *fp;
	size_t n;
	assert(trie && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\trankdir = TB;\n"
		"\tnode [shape = record, style = filled];\n"
		"\tTrie [label=\"{\\<" QUOTE(TRIE_NAME) "\\>Trie: " QUOTE(TRIE_TYPE)
		"\\l|size: %lu\\lcapacity: %lu\\l"
		"next capacity: %lu\\l}\"];\n", (unsigned long)trie->a.size,
		(unsigned long)trie->a.capacity, (unsigned long)trie->a.next_capacity);
	if(!trie->a.size) goto outer;
	fprintf(fp, "\tnode [fillcolor=lightsteelblue];\n");
	fprintf(fp, "\tn0 -> Trie [dir = back];\n");
	fprintf(fp, "\tsubgraph cluster_data {\n"
		"\t\tstyle=filled;\n");
	if(trie->a.size == 1) {
		fprintf(fp, "\t\tn0 [label=\"%s\"];\n",
			PN_(to_key)(trie->a.data->leaf));
		goto inner;
	}
	assert(trie->a.size > 1);
	for(n = 0; n < size; n++) {
		if(PN_(is_branch)(trie, n)) {
			size_t offset = (trie->a.data[n].branch.left_branches << 1) + 2;
			fprintf(fp,
				"\t\tn%lu [shape = \"oval\" label=\"%u\"];\n"
				"\t\tn%lu -> n%lu [style = dashed];\n"
				"\t\tn%lu -> n%lu [label = \"%lu\"];\n",
				(unsigned long)n, trie->a.data[n].branch.choice_bit,
				(unsigned long)n, (unsigned long)n + 1,
				(unsigned long)n, (unsigned long)n + offset,
				(unsigned long)offset);
		} else {
			fprintf(fp, "\t\tn%lu [label=\"%s\"];\n",
				(unsigned long)n, PN_(to_key)(trie->a.data[n].leaf));
		}
	}
inner:
	fprintf(fp, "\t}\n");
outer:
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

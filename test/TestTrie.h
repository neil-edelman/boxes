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

static void PN_(print)(const struct N_(Trie) *const trie) {
	size_t target, n, on;
	unsigned branch;
	printf("__print__ trie.size %lu\n", trie ? trie->a.size : 0);
	if(!trie) { printf("null\n\n"); return; }
	if(!trie->a.size) { printf("empty\n\n"); return; }
	if(trie->a.size == 1) {
		printf("n0: leaf \"%s\".\n\n", PN_(to_key)(trie->a.data->leaf));
		return;
	}
	for(target = 0, branch = 1; target < trie->a.size; target += 1 + branch) {
		n = 0;
		while(n < target) {
			on = n + 1, on = on + trie->a.data[on].right_offset;
			if(on > target) {
				branch = trie->a.data[n].branch.left_branch;
				n += 2;
			} else {
				branch = trie->a.data[n].branch.right_branch;
				n = on;
			}
			assert(n < trie->a.size && branch <= 1);
		}
		assert(n == target);
		if(branch) {
			printf("n%lu: bit %u; %s:%s.\n"
				"n%lu: on offset %lu.\n", (unsigned long)n,
				trie->a.data[n].branch.choice_bit,
				trie->a.data[n].branch.left_branch
				? "branch" : "leaf",
				trie->a.data[n].branch.right_branch
				? "branch" : "leaf",
				(unsigned long)(n + 1),
				trie->a.data[n + 1].right_offset);
		} else {
			printf("n%lu: leaf \"%s\".\n",
				(unsigned long)n, PN_(to_key)(trie->a.data[n].leaf));
		}
	}
	printf("\n");
}

/** Draw a graph of `trie` to `fn` in Graphviz format. */
static void PN_(graph)(const struct N_(Trie) *const trie,
	const char *const fn) {
	FILE *fp;
	size_t target, n, on;
	unsigned branch;
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
	for(target = 0, branch = 1; target < trie->a.size; target += 1 + branch) {
		n = 0;
		while(n < target) {
			on = n + 1, on = on + trie->a.data[on].right_offset;
			if(on > target) {
				branch = trie->a.data[n].branch.left_branch;
				n += 2;
			} else {
				branch = trie->a.data[n].branch.right_branch;
				n = on;
			}
			assert(n < trie->a.size && branch <= 1);
		}
		assert(n == target);
		fflush(fp);
		if(branch) {
			fprintf(fp, "\t\tn%lu [shape = \"oval\" label=\"%u\"];\n"
				"\t\tn%lu -> n%lu [style = dashed];\n"
				"\t\tn%lu -> n%lu [label = %lu];\n",
				(unsigned long)n, trie->a.data[n].branch.choice_bit,
				(unsigned long)n, (unsigned long)n + 2,
				(unsigned long)n, n + 1 + trie->a.data[n + 1].right_offset,
				trie->a.data[n + 1].right_offset);
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

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

/** Only used if `TRIE_TEST`. */
typedef void (*PN_(Action))(PN_(Type) *);

/* `TRIE_TEST` must be a function that implements `<PN>Action`. */
static const PN_(Action) PN_(filler) = (TRIE_TEST);

/** Debug: prints `trie`. */
static void PN_(print)(const struct N_(Trie) *const trie) {
	size_t i, n;
	printf("__Trie: ");
	if(!trie) { printf("null.\n"); return; }
	printf("{");
	for(i = 0; i < trie->leaves.size; i++)
		printf("%s%s", i ? ", " : "", PN_(to_key)(trie->leaves.data[i]));
	printf("}; {");
	for(n = 0; n < trie->branches.size; n++)
		printf("%s%u:%lu", n ? ", " : "", trie_bit(trie->branches.data[n]),
		(unsigned long)trie_left(trie->branches.data[n]));
	printf("}.\n");
}

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

/* Given `n1` in `trie` branches, calculate the corresponding left leaf. */
/*static size_t PN_(leaf)(const struct N_(Trie) *const trie,
	const size_t n) {
	size_t n0, i;
	assert(trie && n < trie->branches.size && trie->branches.data[n].left);
	printf("n = %lu of { ", n);
	for(i = 0; i < trie->branches.size; i++)
		printf("%u ", trie->branches.data[i].left);
	printf("}.\n");
	for(i = 0, n0 = 0; ; ) {
		const unsigned left = trie->branches.data[n0].left;
		printf("i=%lu,n0=%lu,left=%u;", i, n0, left);
		if(n <= n0) break;
		{
			size_t x;
			unsigned l = trie->branches.data[n0].left;
			printf("...%u [", trie->branches.data[n0].left);
			for(x = n0 + 1; x < n0 + 1 + l; x++)
				printf("%u ", trie->branches.data[x].left);
			printf("][");
			for( ; x < trie->branches.size; x++)
				printf("%u ", trie->branches.data[x].left);
			printf("].\n");
		}
		if(n0 + left <= n) n0++;
		else n0 += left + 1, i += left + 1;
	}
	printf("Left leaf %lu.\n", i);
}*/

/** Given `n` in `trie` branches, caluculate the right child branches.
 @order \O(log `size`) */
static size_t PN_(right)(const struct N_(Trie) *const trie, const size_t n) {
	size_t remaining = trie->branches.size, n0 = 0, left, right;
	assert(trie && n < remaining);
	for( ; ; ) {
		left = trie_left(trie->branches.data[n0]);
		right = remaining - left - 1;
		assert(left < remaining && right < remaining);
		if(n0 >= n) break;
		if(n <= n0 + left) remaining = left, n0++;
		else remaining = right, n0 += left + 1;
	}
	assert(n0 == n);
	return right;
}

/** @return Given `n` in `trie` branches, follows the internal nodes left until
 it hits a branch. */
static size_t PN_(left_leaf)(const struct N_(Trie) *const trie,
	const size_t n) {
	size_t remaining = trie->branches.size, n0 = 0, left, right, i = 0;
	assert(trie && n < remaining);
	for( ; ; ) {
		left = trie_left(trie->branches.data[n0]);
		right = remaining - left - 1;
		assert(left < remaining && right < remaining);
		if(n0 >= n) break;
		if(n <= n0 + left) remaining = left, n0++;
		else remaining = right, n0 += left + 1, i += left + 1;
	}
	assert(n0 == n);
	return i;
}

/** Draw a graph of `trie` to `fn` in Graphviz format. */
static void PN_(graph)(const struct N_(Trie) *const trie,
	const char *const fn) {
	FILE *fp;
	size_t i, n;
	assert(trie && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\trankdir = TB;\n"
		"\tnode [shape = record, style = filled];\n"
		"\tTrie [label = \"{\\<" QUOTE(TRIE_NAME) "\\>Trie: " QUOTE(TRIE_TYPE)
		"\\l|size: %lu\\l}\"];\n", (unsigned long)N_(TrieSize)(trie));
	fprintf(fp, "\tnode [shape = none, fillcolor = none];\n");
	for(n = 0; n < trie->branches.size; n++) {
		const size_t branch = trie->branches.data[n];
		const size_t left = trie_left(branch), right = PN_(right)(trie, n);
		fprintf(fp, "\tbranch%lu [label = \"%u %lu:%lu\"];\n"
			"\tbranch%lu -> ", (unsigned long)n, trie_bit(branch),
			(unsigned long)left, (unsigned long)right, (unsigned long)n);
		if(left) fprintf(fp, "branch%lu [style = dashed]; // left branch\n",
			(unsigned long)n + 1);
		else fprintf(fp, "leaf%lu [style = dashed]; // left leaf\n",
			(unsigned long)PN_(left_leaf)(trie, n));
		fprintf(fp, "\tbranch%lu -> ", (unsigned long)n);
		if(right) fprintf(fp, "branch%lu; // right branch\n",
			(unsigned long)n + left + 1);
		else fprintf(fp, "leaf%lu; // right leaf\n",
			(unsigned long)PN_(left_leaf)(trie, n) + left + 1);
	}
	/*assert(i == trie->leaves.size);*/
	/* This must be after the branches, or it will mix up the order. Since they
	 have been referenced, one needs explicit formatting? */
	for(i = 0; i < trie->leaves.size; i++)
		fprintf(fp, "\tleaf%lu [label = \"%s\", shape = box, "
		"fillcolor = lightsteelblue, style = filled];\n", (unsigned long)i,
			PN_(to_key)(trie->leaves.data[i]));
	fprintf(fp, "\tnode [color = red];\n"
		"}\n");
	fclose(fp);
}

#if 0
/** Makes sure the `trie` is in a valid state. */
static void PN_(valid)(const struct N_(Trie) *const trie) {
	if(!trie) return;
	if(!trie->a.data) { assert(!trie->a.size); return; }
	assert(trie->a.size == 1 || (trie->a.size - 1) % 3 == 0);
}

/** Will be tested on stdout. Requires `TRIE_TEST`, and not `NDEBUG` while
 defining `assert`.
 @allow */
static void N_(TrieTest)(void) {
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

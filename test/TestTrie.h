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

/** Makes sure the `trie` is in a valid state. */
static void PN_(valid)(const struct N_(Trie) *const trie) {
	PN_(Type) *const*a;
	size_t i, i_end;
	int cmp;
	if(!trie) return;
	if(!trie->leaves.data) { assert(!trie->leaves.size && !trie->branches.data
		&& !trie->branches.size); return; }
	assert(trie->leaves.size == trie->branches.size + 1);
	for(a = N_(TrieArray)(trie), i = 1, i_end = N_(TrieSize)(trie);
		i < i_end; i++) {
		cmp = strcmp(PN_(to_key)(a[i - 1]), PN_(to_key)(a[i]));
		assert(cmp < 0);
	}
}

static void PN_(test)(void) {
	struct N_(Trie) trie = TRIE_IDLE;
	size_t n, add;
	PN_(Type) *const*a, *i, *data[10];
	const size_t data_size = sizeof data / sizeof *data;
	PN_(valid)(0);
	PN_(valid)(&trie);
	N_(Trie)(0);
	N_(Trie)(&trie), PN_(valid)(&trie);
	n = N_(TrieSize)(&trie), a = N_(TrieArray)(&trie), assert(!n && !a);
	N_(TrieClear)(0);
	N_(TrieClear)(&trie), PN_(valid)(&trie);
	i = N_(TrieGet)(0, 0), assert(!i);
	i = N_(TrieGet)(0, ""), assert(!i);
	i = N_(TrieGet)(&trie, 0), assert(!i);
	i = N_(TrieGet)(&trie, ""), assert(!i);

	/* Make random data. */
	for(n = 0; n < data_size; n++) PN_(filler)(data[n]);

	assert(!N_(TrieAdd)(0, 0));
	assert(!N_(TrieAdd)(0, ""));
	assert(!N_(TrieAdd)(&trie, 0));
	errno = 0;
	for(add = 0, n = 0; n < data_size; n++) add += N_(TrieAdd)(&trie, data[n]);
	assert(n > 0 && n <= data_size);
	N_(Trie_)(0);
	N_(Trie_)(&trie), assert(!N_(TrieSize)(&trie)), PN_(valid)(&trie);
}

/** Will be tested on stdout. Requires `TRIE_TEST`, and not `NDEBUG` while
 defining `assert`.
 @allow */
static void N_(TrieTest)(void) {
	printf("<" QUOTE(TRIE_NAME) ">Trie"
		" of type <" QUOTE(TRIE_TYPE) ">"
		" was created using: TREE_KEY<" QUOTE(TRIE_KEY) ">;"
		" TRIE_TEST <" QUOTE(TRIE_TEST) ">;"
		" testing:\n");
	PN_(test)();
	fprintf(stderr, "Done tests of <" QUOTE(TRIE_NAME) ">Trie.\n\n");
}

/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_

#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/* `TRIE_TEST` must be a function that implements this prototype. */
static void (*PT_(filler))(PT_(type) *) = (TRIE_TEST);

/** Debug: prints `trie`. */
static void PT_(print)(const struct T_(trie) *const trie) {
	size_t i, n;
	printf("__Trie: ");
	if(!trie) { printf("null.\n"); return; }
	printf("{");
	for(i = 0; i < trie->leaves.size; i++)
		printf("%s%s", i ? ", " : "", PT_(to_key)(trie->leaves.data[i]));
	printf("}; {");
	for(n = 0; n < trie->branches.size; n++)
		printf("%s%lu:%lu", n ? ", " : "", trie_skip(trie->branches.data[n]),
		(unsigned long)trie_left(trie->branches.data[n]));
	printf("}.\n");
}

/** Given `n` in `trie` branches, caluculate the right child branches.
 @order \O(log `size`) */
static size_t PT_(right)(const struct T_(trie) *const trie, const size_t n) {
	size_t remaining = trie->branches.size, n0 = 0, left, right;
	assert(trie && n < remaining);
	for( ; ; ) {
		left = trie_left(trie->branches.data[n0]);
		right = remaining - left - 1;
		/*assert(left < remaining && right < remaining); <- Invalid tries. */
		if(n0 >= n) break;
		if(n <= n0 + left) remaining = left, n0++;
		else remaining = right, n0 += left + 1;
	}
	assert(n0 == n);
	return right;
}

/** @return Given `n` in `trie` branches, follows the internal nodes left until
 it hits a branch. */
static size_t PT_(left_leaf)(const struct T_(trie) *const trie,
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
static void PT_(graph)(const struct T_(trie) *const trie,
	const char *const fn) {
	FILE *fp;
	size_t i, n;
	assert(trie && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\trankdir = TB;\n"
		"\tnode [shape = record, style = filled];\n"
		"\tTrie [label = \"{\\<" QUOTE(TRIE_NAME) "\\>Trie: " QUOTE(TRIE_TYPE)
		"\\l|size: %lu\\l}\"];\n", (unsigned long)T_(trie_size)(trie));
	fprintf(fp, "\tnode [shape = none, fillcolor = none];\n");
	for(n = 0; n < trie->branches.size; n++) {
		const size_t branch = trie->branches.data[n];
		const size_t left = trie_left(branch), right = PT_(right)(trie, n);
		fprintf(fp, "\tbranch%lu [label = \"%lu\"];\n"
			"\tbranch%lu -> ", (unsigned long)n, trie_skip(branch),
			(unsigned long)n);
		if(left) fprintf(fp, "branch%lu [style = dashed]; // left branch\n",
			(unsigned long)n + 1);
		else fprintf(fp, "leaf%lu [style = dashed]; // left leaf\n",
			(unsigned long)PT_(left_leaf)(trie, n));
		fprintf(fp, "\tbranch%lu -> ", (unsigned long)n);
		if(right) fprintf(fp, "branch%lu; // right branch\n",
			(unsigned long)n + left + 1);
		else fprintf(fp, "leaf%lu; // right leaf\n",
			(unsigned long)PT_(left_leaf)(trie, n) + left + 1);
	}
	/* This must be after the branches, or it will mix up the order. Since they
	 have been referenced, one needs explicit formatting? */
	for(i = 0; i < trie->leaves.size; i++)
		fprintf(fp, "\tleaf%lu [label = \"%s\", shape = box, "
		"fillcolor = lightsteelblue, style = filled];\n", (unsigned long)i,
		PT_(to_key)(trie->leaves.data[i]));
	fprintf(fp, "\tnode [color = red];\n"
		"}\n");
	fclose(fp);
}

/** Makes sure the `trie` is in a valid state. */
static void PT_(valid)(const struct T_(trie) *const trie) {
	PT_(type) *const*a;
	size_t i, i_end;
	int cmp;
	if(!trie) return;
	if(!trie->leaves.data) { assert(!trie->leaves.size
		&& !trie->branches.data && !trie->branches.size); return; }
	assert(trie->leaves.size == trie->branches.size + 1);
	for(a = T_(trie_array)(trie), i = 1, i_end = T_(trie_size)(trie);
		i < i_end; i++) {
		cmp = strcmp(PT_(to_key)(a[i - 1]), PT_(to_key)(a[i]));
		assert(cmp < 0);
	}
}

static void PT_(test)(void) {
	struct T_(trie) trie = TRIE_IDLE;
	size_t n, size;
	struct { PT_(type) data; int is_in; } es[10];
	const size_t es_size = sizeof es / sizeof *es;
	PT_(type) *const*a, *i, *eject/*, copy*/;
	int ret;

	PT_(valid)(0);
	PT_(valid)(&trie);
	T_(trie)(&trie), PT_(valid)(&trie);
	n = T_(trie_size)(&trie), a = T_(trie_array)(&trie), assert(!n && !a);
	T_(trie_clear)(&trie), PT_(valid)(&trie);
	i = T_(trie_get)(&trie, ""), assert(!i);

	/* Make random data. */
	for(n = 0; n < es_size; n++) PT_(filler)(&es[n].data);

	assert(!T_(trie_remove)(&trie, ""));
	errno = 0;
	for(n = 0; n < es_size; n++)
		es[n].is_in = T_(trie_add)(&trie, &es[n].data);
	assert(es[0].is_in);
	size = T_(trie_size)(&trie);
	assert(size > 0 && size <= T_(trie_size)(&trie));
	PT_(print)(&trie);
	printf("Now trie is %s.\n", T_(trie_to_string)(&trie));
	PT_(graph)(&trie, "graph/" QUOTE(TRIE_NAME) "Trie-test.gv");
	/*...*/
	ret = T_(trie_add)(&trie, &es[0].data); /* Doesn't add. */
	assert(ret && size == T_(trie_size)(&trie));
	ret = T_(trie_put)(&trie, &es[0].data, 0),
		assert(ret && size == T_(trie_size)(&trie));
	ret = T_(trie_put)(&trie, &es[0].data, &eject),
		assert(ret && size == T_(trie_size)(&trie) && eject == &es[0].data);
	ret = T_(trie_policy_put)(&trie, &es[0].data, 0, 0),
		assert(ret && size == T_(trie_size)(&trie));
	ret = T_(trie_policy_put)(&trie, &es[0].data, &eject, 0),
		assert(ret && size == T_(trie_size)(&trie) && eject == &es[0].data);
	ret = T_(trie_policy_put)(&trie, &es[0].data, &eject, &PT_(false_replace)),
		assert(ret && size == T_(trie_size)(&trie) && eject == &es[0].data);
	/* fixme: this is no. We should get errors. What am I doing? */
	/*memcpy((void *)&copy, &es[0].data, sizeof es[0].data);
	ret = T_(trie_policy_put)(&trie, &copy, &eject, &PT_(false_replace)),
		assert(ret && size == T_(trie_size)(&trie) && eject == &copy);*/
	T_(trie_)(&trie), assert(!T_(trie_size)(&trie)), PT_(valid)(&trie);
}

/** Will be tested on stdout. Requires `TRIE_TEST`, and not `NDEBUG` while
 defining `assert`. @allow */
static void T_(trie_test)(void) {
	printf("<" QUOTE(TRIE_NAME) ">Trie"
		" of type <" QUOTE(TRIE_TYPE) ">"
		" was created using: TREE_KEY<" QUOTE(TRIE_KEY) ">;"
		" TRIE_TEST <" QUOTE(TRIE_TEST) ">;"
		" testing:\n");
	PT_(test)();
	fprintf(stderr, "Done tests of <" QUOTE(TRIE_NAME) ">Trie.\n\n");
}

/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_

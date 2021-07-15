#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/* `TRIE_TEST` must be a function that implements this prototype. */
static void (*PT_(filler))(PT_(type) *) = (TRIE_TEST);

/** Debug: prints `trie`. */
static void PT_(print)(const struct T_(trie) *const trie) {
	size_t i;
	printf("__Trie: ");
	if(!trie) { printf("null.\n"); return; }
	printf("{");
	if(!trie->root.s) {
		printf("empty");
	} else {
		struct PT_(tree) tree;
		PT_(extract)(trie->root, &tree);
		for(i = 0; i < tree.bsize; i++) /* No, polyvalent. */
			printf("%s%s", i ? ", " : "", PT_(to_key)(tree.leaves[i].data));
	}
	/*printf("}; {");
	for(n = 0; n < trie->branches.size; n++)
		printf("%s%lu:%lu", n ? ", " : "", trie_skip(trie->branches.data[n]),
		(unsigned long)trie_left(trie->branches.data[n]));*/
	printf("}.\n");
}

/** Given a branch `b` in `tree` branches, calculate the right child branches.
 @order \O(log `size`) */
static unsigned short PT_(right)(union PT_(any_ptree) tree,
	const unsigned short b) {
	struct trie_branch *branches;
	union PT_(leaf) *leaves;
	unsigned short bsize = PT_(extract)(tree, &branches, &leaves),
		left, right, b0 = 0;
	assert(tree.t0 && b < bsize);
	for( ; ; ) {
		right = bsize - (left = branches[b0].left) - 1;
		assert(left < bsize && right < bsize);
		if(b0 >= b) break;
		if(b <= b0 + left) bsize = left, b0++;
		else bsize = right, b0 += left + 1;
	}
	assert(b0 == b);
	return right;
}

/** @return Follows the branches to `b` in `tree` and returns the leaf. */
static unsigned short PT_(left_leaf)(union PT_(any_ptree) tree,
	const unsigned short b) {
	struct trie_branch *branches;
	union PT_(leaf) *leaves;
	unsigned short bsize = PT_(extract)(tree, &branches, &leaves),
		left, right, i = 0, b0 = 0;
	assert(tree.t0 && b < bsize);
	for( ; ; ) {
		right = bsize - (left = branches[b0].left) - 1;
		assert(left < bsize && right < bsize);
		if(b0 >= b) break;
		if(b <= b0 + left) bsize = left, b0++;
		else bsize = right, b0 += left + 1, i += left + 1;
	}
	assert(b0 == b);
	return i;
}

static void PT_(graph_tree)(const union PT_(any_ptree) tree,
	const unsigned short depth, FILE *const fp) {
	struct trie_branch *branches, *branch;
	union PT_(leaf) *leaves;
	const unsigned short bsize = PT_(extract)(tree, &branches, &leaves);
	unsigned short b, i;
	assert(tree.t && fp);
	fprintf(fp, "\tsubgraph cluster_tree%p {\n"
		"\t\tstyle = filled;\n"
		"\t\tlabel = \"depth %u\";\n", (void *)tree.t, depth);
	for(b = 0; b < bsize; b++) { /* Branches. */
		branch = branches + b;
		const unsigned short left = branch->left, right = PT_(right)(tree, b);
		fprintf(fp, "\t\ttree%pbranch%u "
			"[label = \"%u\", shape = none, fillcolor = none];\n"
			"\t\ttree%pbranch%u -> ", (void *)tree.t, b, branch->skip,
			(void *)tree.t, b);
		if(left) fprintf(fp, "tree%pbranch%u [style = dashed];\n",
			(void *)tree.t, b + 1);
		else fprintf(fp, "tree%pleaf%u [style = dashed];\n",
			(void *)tree.t, PT_(left_leaf)(tree, b));
		fprintf(fp, "\t\ttree%pbranch%u -> ", (void *)tree.t, b);
		if(right) fprintf(fp, "tree%pbranch%u;\n",
			(void *)tree.t, b + left + 1);
		else fprintf(fp, "tree%pleaf%u;\n", (void *)tree.t,
			PT_(left_leaf)(tree, b) + left + 1);
	}
	for(i = 0; i <= bsize; i++)
		fprintf(fp, "\t\ttree%pleaf%u [label = \"%s\"];\n", (void *)tree.t,
			i, PT_(to_key)(leaves[i].data)); /* fixme: or link! */
	fprintf(fp, "\t}\n\n");
}

/** Draw a graph of `trie` to `fn` in Graphviz format. */
static void PT_(graph)(const struct T_(trie) *const trie,
	const char *const fn) {
	FILE *fp;
	size_t n;
	assert(trie && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\trankdir = TB;\n"
		"\tnode [shape = record, style = filled];\n"
		"\ttrie [label = \"{\\<" QUOTE(TRIE_NAME) "\\>trie: " QUOTE(TRIE_TYPE)
		"%s}\"];\n"
		"\tnode [shape = box, fillcolor = lightsteelblue];\n",
		!trie->depth && !trie->root.data ? "\\l|idle\\l" : "");
	/* "\tnode [shape = none, fillcolor = none];\n" */
	if(!trie->depth) {
		if(trie->root.data) {
			fprintf(fp, "\tsingle [label = \"%s\"]\n"
				"\ttrie -> single [color = firebrick];\n",
				PT_(to_key)(trie->root.data));
		}
	} else {
		fprintf(fp, "\ttrie -> tree%p%s0 [color = firebrick];\n",
			(void *)trie->root.tree.t0,
			trie->root.tree.t->bsize ? "branch" : "leaf");
		PT_(graph_tree)(trie->root.tree, trie->depth, fp);
	}

#if 0
	color = royalblue
	if(!trie->depth) {
		if(!trie->depth) {
			PT_(type) *data = trie->root.data;
			if(!data) fprintf(fp, "idle");
			else fprintf(fp, "%s", PT_(to_key)(data));
		} else {
			struct trie_branch *branch;
			union PT_(leaf) *leaf;
			const unsigned short bsize
				= PT_(extract)(trie->root.tree, &branch, &leaf);
			for(i = 0; i < bsize; i++)
				printf("%s%s", i ? ", " : "", PT_(to_key)(leaf[i].data));
		}
	}
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
#endif
end:
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
#if 0
	if(!trie->leaves.data) { assert(!trie->leaves.size
		&& !trie->branches.data && !trie->branches.size); return; }
	assert(trie->leaves.size == trie->branches.size + 1);
	for(a = T_(trie_array)(trie), i = 1, i_end = T_(trie_size)(trie);
		i < i_end; i++) {
		cmp = strcmp(PT_(to_key)(a[i - 1]), PT_(to_key)(a[i]));
		assert(cmp < 0);
	}
#endif
}

static void PT_(test)(void) {
	char fn[64];
	struct T_(trie) trie = TRIE_IDLE;
	size_t n, size;
	struct { PT_(type) data; int is_in; } es[10];
	const size_t es_size = sizeof es / sizeof *es;
	PT_(type) *const*a, *i, *eject/*, copy*/;
	int ret;

	PT_(valid)(0);
	PT_(valid)(&trie);
	T_(trie)(&trie), PT_(valid)(&trie);
	PT_(graph)(&trie, "graph/" QUOTE(TRIE_NAME) "_trie-idle.gv");
#if 0
	n = T_(trie_size)(&trie), a = T_(trie_array)(&trie), assert(!n && !a);
#endif
	T_(trie_clear)(&trie), PT_(valid)(&trie);
	i = T_(trie_get)(&trie, ""), assert(!i);

	/* Make random data. */
	for(n = 0; n < es_size; n++) PT_(filler)(&es[n].data);

	errno = 0;
	/* fixme: could be duplicates. */
	for(n = 0; n < es_size; n++)
		es[n].is_in = !!T_(trie_add)(&trie, &es[n].data), assert(es[n].is_in),
		sprintf(fn, "graph/" QUOTE(TRIE_NAME) "_trie-%lu.gv",
		(unsigned long)n + 1lu), PT_(graph)(&trie, fn);
	PT_(print)(&trie);
	printf("Now trie is %s.\n", T_(trie_to_string)(&trie));
	/*...*/
#if 0
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
#endif
}

/** Will be tested on stdout. Requires `TRIE_TEST`, and not `NDEBUG` while
 defining `assert`. @allow */
static void T_(trie_test)(void) {
	printf("<" QUOTE(TRIE_NAME) ">trie"
		" of type <" QUOTE(TRIE_TYPE) ">"
		" was created using: TREE_KEY<" QUOTE(TRIE_KEY) ">;"
		" TRIE_TEST <" QUOTE(TRIE_TEST) ">;"
		" testing:\n");
	PT_(test)();
	fprintf(stderr, "Done tests of <" QUOTE(TRIE_NAME) ">trie.\n\n");
}

#undef QUOTE
#undef QUOTE_

#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/* `TRIE_TEST` must be a function that implements this prototype. */
static void (*PT_(filler))(PT_(type) *) = (TRIE_TEST);

/** Given a branch `b` in `tree` branches, calculate the right child branches.
 @order \O(log `size`) */
static unsigned PT_(right)(const union PT_(any_tree) any, const unsigned b) {
	struct PT_(tree) tree;
	unsigned left, right, b0 = 0;
	assert(any.info);
	PT_(extract)(any, &tree), assert(b < tree.bsize);
	for( ; ; ) {
		right = tree.bsize - (left = tree.branches[b0].left) - 1;
		assert(left < tree.bsize && right < tree.bsize);
		if(b0 >= b) break;
		if(b <= b0 + left) tree.bsize = left, b0++;
		else tree.bsize = right, b0 += left + 1;
	}
	assert(b0 == b);
	return right;
}

/** @return Follows the branches to `b` in `tree` and returns the leaf. */
static unsigned PT_(left_leaf)(union PT_(any_tree) any, const unsigned b) {
	struct PT_(tree) tree;
	unsigned left, right, i = 0, b0 = 0;
	assert(any.info);
	PT_(extract)(any, &tree), assert(b < tree.bsize);
	for( ; ; ) {
		right = tree.bsize - (left = tree.branches[b0].left) - 1;
		assert(left < tree.bsize && right < tree.bsize);
		if(b0 >= b) break;
		if(b <= b0 + left) tree.bsize = left, b0++;
		else tree.bsize = right, b0 += left + 1, i += left + 1;
	}
	assert(b0 == b);
	return i;
}

/** Graphs `any` on `fp`. */
static void PT_(graph_tree_mem)(const union PT_(any_tree) any, FILE *const fp) {
	struct PT_(tree) tree;
	struct trie_branch *branch;
	unsigned b, i;
	assert(any.info && fp);
	PT_(extract)(any, &tree);
	/* Tree is one record node in memory -- GraphViz says html is
	 case-insensitive, but I cannot get it to work without screaming. */
	fprintf(fp, "\ttree%pbranch0 [shape = box, style = filled, "
		"label = <\n"
		"<TABLE BORDER=\"0\" CELLBORDER=\"1\">\n"
		"\t<TR>\n"
		"\t\t<TD ALIGN=\"right\" BORDER=\"0\">left</TD>\n", (void *)any.info);
	for(b = 0; b < tree.bsize; b++) branch = tree.branches + b,
		fprintf(fp, "\t\t<TD>%u</TD>\n", branch->left);
	fprintf(fp, "\t</TR>\n"
		"\t<TR>\n"
		"\t\t<TD ALIGN=\"right\" BORDER=\"0\">skip</TD>\n");
	for(b = 0; b < tree.bsize; b++) branch = tree.branches + b,
		fprintf(fp, "\t\t<TD>%u</TD>\n", branch->skip);
	fprintf(fp, "\t</TR>\n"
		"\t<TR>\n"
		"\t\t<TD ALIGN=\"right\" BORDER=\"0\">leaves</TD>\n");
	for(i = 0; i <= tree.bsize; i++) {
		if(TRIE_BITTEST(tree.children, i))
			fprintf(fp, "\t\t<TD PORT=\"%u\">...</TD>\n", i);
		else
			fprintf(fp, "\t\t<TD>%s</TD>\n", PT_(to_key)(tree.leaves[i].data));
			/* Should really escape it . . . don't have weird characters. */
	}
	fprintf(fp, "\t</TR>\n"
		"</TABLE>>];\n");
	/* Draw the lines between trees. */
	for(i = 0; i <= tree.bsize; i++) if(TRIE_BITTEST(tree.children, i))
		fprintf(fp, "\ttree%pbranch0:%u -> tree%pbranch0 [label = \"%uB\", "
		"color = firebrick];\n",
		(void *)any.info, i, (void *)tree.leaves[i].child.info,
		PT_(tree_sizes)[tree.leaves[i].child.info->no]);
	/* Recurse. */
	for(i = 0; i <= tree.bsize; i++) if(TRIE_BITTEST(tree.children, i))
		PT_(graph_tree_mem)(tree.leaves[i].child, fp);
}

/** Graphs `any` on `fp`. */
static void PT_(graph_tree)(const union PT_(any_tree) any, FILE *const fp) {
	struct PT_(tree) tree;
	struct trie_branch *branch;
	unsigned left, right, b, i;
	assert(any.info && fp);
	PT_(extract)(any, &tree);
	fprintf(fp, "\t//subgraph cluster_tree%p { "
		"// confuse the order in dot\n"
		"\t\t//style = filled;\n"
		"\t\t//label = \"leaves %u/%u; tree%u (%u); %uB\";\n",
		(void *)any.info, tree.bsize + 1, trie_tree_bsizes[tree.no] + 1,
		tree.no, trie_tree_count, PT_(tree_sizes)[tree.no]);
	if(tree.bsize) {
		for(b = 0; b < tree.bsize; b++) { /* Branches. */
			branch = tree.branches + b;
			left = branch->left, right = PT_(right)(any, b);
			fprintf(fp, "\t\ttree%pbranch%u "
				"[label = \"%u\", shape = none, fillcolor = none];\n"
				"\t\ttree%pbranch%u -> ", (void *)any.info, b, branch->skip,
				(void *)any.info, b);
			if(left) {
				fprintf(fp, "tree%pbranch%u [style = dashed];\n",
					(void *)any.info, b + 1);
			} else {
				unsigned leaf = PT_(left_leaf)(any, b);
				if(TRIE_BITTEST(tree.children, leaf)) {
					fprintf(fp, "tree%pbranch0 [label = \"%uB\", "
						"style = dashed, color = firebrick];\n",
						(void *)tree.leaves[leaf].child.info,
						PT_(tree_sizes)[tree.leaves[leaf].child.info->no]);
				} else {
					fprintf(fp,
						"tree%pleaf%u [style = dashed, color = royalblue];\n",
						(void *)any.info, leaf);
				}
			}
			fprintf(fp, "\t\ttree%pbranch%u -> ", (void *)any.info, b);
			if(right) {
				fprintf(fp, "tree%pbranch%u;\n",
					(void *)any.info, b + left + 1);
			} else {
				unsigned leaf = PT_(left_leaf)(any, b) + left + 1;
				if(TRIE_BITTEST(tree.children, leaf)) {
					fprintf(fp, "tree%pbranch0 [label = \"%uB\", "
						"color = firebrick];\n",
						(void *)tree.leaves[leaf].child.info,
						PT_(tree_sizes)[tree.leaves[leaf].child.info->no]);
				} else {
					fprintf(fp, "tree%pleaf%u [color = royalblue];\n",
						(void *)any.info, leaf);
				}
			}
		}
		for(i = 0; i <= tree.bsize; i++) if(!TRIE_BITTEST(tree.children, i))
			fprintf(fp, "\t\ttree%pleaf%u [label = \"%s\"];\n",
			(void *)any.info, i, PT_(to_key)(tree.leaves[i].data));
	} else {
		/* Instead of creating a lookahead function to previous references, we
		 very lazily also just call this a branch, even though it's a leaf. */
		assert(!TRIE_BITTEST(tree.children, 0)); /* fixme:
		 should be possible; then what? */
		fprintf(fp, "\t\ttree%pbranch0 [label = \"%s\"];\n", (void *)any.info,
			PT_(to_key)(tree.leaves[0].data));
	}
	fprintf(fp, "\t//}\n\n");
	for(i = 0; i <= tree.bsize; i++) if(TRIE_BITTEST(tree.children, i))
		PT_(graph_tree)(tree.leaves[i].child, fp);
}

/** Draw a graph of `trie` to `fn` in Graphviz format. */
static void PT_(graph_choose)(const struct T_(trie) *const trie,
	const char *const fn,
	void (*PT_(tree))(const union PT_(any_tree), FILE *)) {
	FILE *fp;
	assert(trie && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\trankdir = TB;\n"
		"\tnode [shape = record, style = filled];\n"
		"\ttrie [label = \"{\\<" QUOTE(TRIE_NAME) "\\>trie: " QUOTE(TRIE_TYPE)
		"; %luB%s}\"];\n"
		"\tnode [shape = box, fillcolor = lightsteelblue];\n",
		(unsigned long)sizeof *trie, trie->root.info ? "" : "\\l|idle\\l");
	if(trie->root.info) {
		fprintf(fp, "\ttrie -> tree%pbranch0 [label = \"%uB\", "
			"color = firebrick];\n", (void *)trie->root.info,
			PT_(tree_sizes)[trie->root.info->no]);
		PT_(tree)(trie->root, fp);
	}
	fprintf(fp, "\tnode [color = red];\n"
		"}\n");
	fclose(fp);
}

static void PT_(graph)(const struct T_(trie) *const trie,
	const char *const fn) { PT_(graph_choose)(trie, fn, &PT_(graph_tree)); }

static void PT_(graph_mem)(const struct T_(trie) *const trie,
	const char *const fn) { PT_(graph_choose)(trie, fn, &PT_(graph_tree_mem)); }

/** Make sure `any` is in a valid state, (and all the children.) */
static void PT_(valid_tree)(const union PT_(any_tree) any) {
	unsigned i;
	int cmp = 0;
	const char *str1 = 0;
	struct PT_(tree) tree;
	assert(any.info && any.info->bsize <= trie_tree_bsizes[any.info->no]);
	PT_(extract)(any, &tree);
	for(i = 0; i < tree.bsize; i++)
		assert(tree.branches[i].left < tree.bsize - 1 - i);
	for(i = 0; i <= tree.bsize; i++) {
		if(TRIE_BITTEST(tree.children, i)) {
			PT_(valid_tree)(tree.leaves[i].child);
		} else {
			const char *str2;
			assert(tree.leaves[i].data);
			str2 = PT_(to_key)(tree.leaves[i].data);
			if(str1) cmp = strcmp(str1, str2), assert(cmp < 0);
			str1 = str2;
		}
	}
}

/** Makes sure the `trie` is in a valid state. */
static void PT_(valid)(const struct T_(trie) *const trie) {
	if(!trie || !trie->root.info) return;
	PT_(valid_tree)(trie->root);
}

static void PT_(test)(void) {
	char fn[64];
	struct T_(trie) trie = TRIE_IDLE;
	size_t n, size;
	struct { PT_(type) data; int is_in; } es[20];
	const size_t es_size = sizeof es / sizeof *es;
	PT_(type) *const*a, *i, *eject;
	int ret;

	PT_(valid)(0);
	PT_(valid)(&trie);
	T_(trie)(&trie), PT_(valid)(&trie);
	printf("Idle graph.\n");
	PT_(graph)(&trie, "graph/" QUOTE(TRIE_NAME) "_trie-idle.gv");
#if 0
	n = T_(trie_size)(&trie), a = T_(trie_array)(&trie), assert(!n && !a);
#endif
	T_(trie_clear)(&trie), PT_(valid)(&trie);
	i = T_(trie_get)(&trie, ""), assert(!i);

	/* Make random data. */
	for(n = 0; n < es_size; n++) PT_(filler)(&es[n].data);

	errno = 0;
	for(n = 0; n < es_size; n++) {
		es[n].is_in = T_(trie_add)(&trie, &es[n].data);
		sprintf(fn, "graph/" QUOTE(TRIE_NAME) "_trie-%lu.gv",
			(unsigned long)n + 1lu);
		printf("Graph %s: %s.\n", fn, T_(trie_to_string)(&trie));
		PT_(graph)(&trie, fn);
		sprintf(fn, "graph/" QUOTE(TRIE_NAME) "_trie-%lu-mem.gv",
			(unsigned long)n + 1lu);
		PT_(graph_mem)(&trie, fn);
		assert(!errno || (perror("Check"), 0));
		if(!es[n].is_in) { printf("Duplicate value %s -> %s.\n",
			PT_(to_key)(&es[n].data), T_(trie_to_string)(&trie)); continue; };
		i = T_(trie_get)(&trie, PT_(to_key)(&es[n].data));
		assert(i == &es[n].data);
	}
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

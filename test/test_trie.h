#include <stdio.h>
#include <string.h>
#include "orcish.h"

#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

static const char *T_(trie_to_string)(const struct T_(trie) *);

#ifndef TRIE_VALUE /* <!-- key set */
/** Works by side-effects, _ie_ fills the type with data. */
typedef void (*PT_(action_fn))(PT_(key) *);
#elif !defined(TRIE_KEY_IN_VALUE) /* ket set --><!-- key map */
typedef void (*PT_(action_fn))(PT_(key) *, PT_(value) *);
#else /* key map --><!-- custom */
typedef void (*PT_(action_fn))(PT_(value) *);
#endif /* custom --> */

typedef void (*PT_(tree_file_fn))(struct PT_(tree) *, size_t, FILE *);

/** Outputs a direction string for `lf` in `tr`, `{ "", "r", "l" }`. */
static const char *PT_(leaf_to_dir)(const struct PT_(tree) *const tr,
	const unsigned lf) {
	struct { unsigned br0, br1, lf; } t;
	unsigned left;
	const char *shape = "";
	t.br0 = 0, t.br1 = tr->bsize, t.lf = 0;
	while(t.br0 < t.br1) {
		left = tr->branch[t.br0].left;
		if(lf <= t.lf + left) t.br1 = ++t.br0 + left, shape = "r";
		else t.br0 += left + 1, t.lf += left + 1, shape = "l";
	}
	return shape;
}

/** Given a branch `b` in `tr` branches, calculate the right child branches.
 @order \O(log `size`) */
static unsigned PT_(right)(const struct PT_(tree) *const tr,
	const unsigned b) {
	unsigned left, right, total = tr->bsize, b0 = 0;
	assert(tr && b < tr->bsize);
	for( ; ; ) {
		right = total - (left = tr->branch[b0].left) - 1;
		assert(left < total && right < total);
		if(b0 >= b) break;
		if(b <= b0 + left) total = left, b0++;
		else total = right, b0 += left + 1;
	}
	assert(b0 == b);
	return right;
}

/** @return Follows the branches to `b` in `tr` and returns the leaf. */
static unsigned PT_(left_leaf)(const struct PT_(tree) *const tr,
	const unsigned b) {
	unsigned left, right, total = tr->bsize, i = 0, b0 = 0;
	assert(tr && b < tr->bsize);
	for( ; ; ) {
		right = total - (left = tr->branch[b0].left) - 1;
		assert(left < tr->bsize && right < tr->bsize);
		if(b0 >= b) break;
		if(b <= b0 + left) total = left, b0++;
		else total = right, b0 += left + 1, i += left + 1;
	}
	assert(b0 == b);
	return i;
}

/** Graphs `tr` on `fp`. `treebit` is the number of bits currently
 (recursive.) */
static void PT_(graph_tree_bits)(struct PT_(tree) *const tree,
	const size_t treebit, FILE *const fp) {
	unsigned b, i;
	assert(tree && fp);
	fprintf(fp, "\ttree%pbranch0 [label = <\n"
		"<table border=\"0\" cellspacing=\"0\">\n", (const void *)tree);
	/*"<table BORDER=\"0\" CELLBORDER=\"0\">\n"*/
	for(i = 0; i <= tree->bsize; i++) {
		const char *key = PT_(sample)(tree, i);
		const struct trie_branch *branch = tree->branch;
		size_t next_branch = treebit + branch->skip;
		const char *params, *start, *end;
		struct { unsigned br0, br1; } in_tree;
		const unsigned is_link = trie_bmp_test(&tree->bmp, i);
		/* 0-width joiner "&#8288;": GraphViz gets upset when tag closed
		 immediately. */
		fprintf(fp, "\t<tr>\n"
			"\t\t<td align=\"left\" port=\"%u\">%s%s%s⊔</font></td>\n",
			i, is_link ? "↓<font color=\"Grey75\">" : "", key,
			is_link ? "" : "<font color=\"Grey75\">");
		in_tree.br0 = 0, in_tree.br1 = tree->bsize;
		for(b = 0; in_tree.br0 < in_tree.br1; b++) {
			const unsigned bit = !!TRIE_QUERY(key, b);
			if(next_branch) {
				next_branch--;
				params = "", start = "", end = "";
			} else {
				if(!bit) {
					in_tree.br1 = ++in_tree.br0 + branch->left;
					params = " bgcolor=\"Grey95\" border=\"1\"";
					start = "", end = "";
				} else {
					in_tree.br0 += branch->left + 1;
					params
						= " bgcolor=\"Black\" color=\"White\" border=\"1\"";
					start = "<font color=\"White\">", end = "</font>";
				}
				next_branch = (branch = tree->branch + in_tree.br0)->skip;
			}
			if(b && !(b & 7)) fprintf(fp, "\t\t<td>&nbsp;</td>\n");
			fprintf(fp, "\t\t<td%s>%s%u%s</td>\n", params, start, bit, end);
		}
		fprintf(fp, "\t</tr>\n");
	}
	fprintf(fp, "</table>>];\n");
	/* Draw the lines between trees. */
	for(i = 0; i <= tree->bsize; i++) if(trie_bmp_test(&tree->bmp, i))
		fprintf(fp, "\ttree%pbranch0:%u -> tree%pbranch0 "
		"[style = dashed, arrowhead = %snormal];\n", (const void *)tree, i,
		(const void *)tree->leaf[i].as_link, PT_(leaf_to_dir)(tree, i));
	/* Recurse. */
	for(i = 0; i <= tree->bsize; i++) {
		struct { unsigned br0, br1, lf; } in_tree;
		size_t bit = treebit;
		if(!trie_bmp_test(&tree->bmp, i)) continue;
		in_tree.br0 = 0, in_tree.br1 = tree->bsize, in_tree.lf = 0;
		while(in_tree.br0 < in_tree.br1) {
			const struct trie_branch *branch = tree->branch + in_tree.br0;
			bit += branch->skip;
			if(i <= in_tree.lf + branch->left)
				in_tree.br1 = ++in_tree.br0 + branch->left;
			else
				in_tree.br0 += branch->left + 1, in_tree.lf += branch->left + 1;
			bit++;
		}
		PT_(graph_tree_bits)(tree->leaf[i].as_link, bit, fp);
	}
}

/** Graphs `tr` on `fp`. `treebit` is the number of bits currently
 (recursive.) */
static void PT_(graph_tree_mem)(struct PT_(tree) *const tree,
	const size_t treebit, FILE *const fp) {
	const struct trie_branch *branch;
	unsigned i;
	(void)treebit;
	assert(tree && fp);
	/* Tree is one record node in memory -- GraphViz says html is
	 case-insensitive, but no. */
	fprintf(fp, "\ttree%pbranch0 [label = <\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td colspan=\"3\" align=\"left\">"
		"<font color=\"Grey75\">%s</font> ∑bit=%lu</td></tr>\n"
		"\t<hr/>\n"
		"\t<tr>\n"
		"\t\t<td><font face=\"Times-Italic\">left</font></td>\n"
		"\t\t<td><font face=\"Times-Italic\">skip</font></td>\n"
		"\t\t<td><font face=\"Times-Italic\">leaves</font></td>\n"
		"\t</tr>\n"
		"\t<hr/>\n", (const void *)tree, orcify(tree), (unsigned long)treebit);
	for(i = 0; i <= tree->bsize; i++) {
		const char *const bgc = i & 1 ? " bgcolor=\"Gray95\"" : "";
		const char *key = PT_(sample)(tree, i);
		const unsigned is_link = trie_bmp_test(&tree->bmp, i);
		if(i < tree->bsize) {
			branch = tree->branch + i;
			fprintf(fp, "\t<tr>\n"
				"\t\t<td align=\"right\"%s>%u</td>\n"
				"\t\t<td align=\"right\"%s>%u</td>\n",
				bgc, branch->left,
				bgc, branch->skip);
		} else {
			fprintf(fp, "\t<tr>\n"
				"\t\t<td>&#8205;</td>\n"
				"\t\t<td>&#8205;</td>\n");
		}
		fprintf(fp, "\t\t<td align=\"left\" port=\"%u\"%s>"
			"%s%s%s⊔</font></td>\n"
			"\t</tr>\n",
			i, bgc, is_link ? "↓<font color=\"Grey75\">" : "", key,
			is_link ? "" : "<font color=\"Grey75\">");
			/* Should really escape it . . . don't have weird characters. */
	}
	fprintf(fp, "\t<hr/>\n"
		"\t<tr><td></td></tr>\n"
		"</table>>];\n");
	/* Draw the lines between trees. */
	for(i = 0; i <= tree->bsize; i++) if(trie_bmp_test(&tree->bmp, i))
		fprintf(fp, "\ttree%pbranch0:%u -> tree%pbranch0 "
		"[style = dashed, arrowhead = %snormal];\n", (const void *)tree, i,
		(const void *)tree->leaf[i].as_link, PT_(leaf_to_dir)(tree, i));
	/* Recurse. */
	for(i = 0; i <= tree->bsize; i++) {
		struct { unsigned br0, br1, lf; } in_tree;
		size_t bit = treebit;
		if(!trie_bmp_test(&tree->bmp, i)) continue;
		in_tree.br0 = 0, in_tree.br1 = tree->bsize, in_tree.lf = 0;
		while(in_tree.br0 < in_tree.br1) {
			branch = tree->branch + in_tree.br0;
			bit += branch->skip;
			if(i <= in_tree.lf + branch->left)
				in_tree.br1 = ++in_tree.br0 + branch->left;
			else
				in_tree.br0 += branch->left + 1, in_tree.lf += branch->left + 1;
			bit++;
		}
		PT_(graph_tree_mem)(tree->leaf[i].as_link, bit, fp);
	}
}

/** Graphs `tr` on `fp`.`treebit` is the number of bits currently
 (recursive.) */
static void PT_(graph_tree_logic)(struct PT_(tree) *const tr,
	const size_t treebit, FILE *const fp) {
	const struct trie_branch *branch;
	unsigned left, right, b, i;
	(void)treebit;
	assert(tr && fp);
	fprintf(fp, "\t// tree %p\n", (const void *)tr);
	if(tr->bsize) {
		fprintf(fp, "\t// branches\n");
		for(b = 0; b < tr->bsize; b++) { /* Branches. */
			branch = tr->branch + b;
			left = branch->left, right = PT_(right)(tr, b);
			fprintf(fp, "\ttree%pbranch%u [label=\"%u\", shape=circle,"
				" style=filled, fillcolor=Grey95];\n"
				"\ttree%pbranch%u -> ", (const void *)tr, b, branch->skip,
				(const void *)tr, b);
			if(left) {
				fprintf(fp, "tree%pbranch%u [arrowhead=rnormal];\n",
					(const void *)tr, b + 1);
			} else {
				unsigned leaf = PT_(left_leaf)(tr, b);
				if(trie_bmp_test(&tr->bmp, leaf)) {
					const struct PT_(tree) *const child =tr->leaf[leaf].as_link;
					const char *root_str = child->bsize ? "branch" : "leaf";
					fprintf(fp,
					"tree%p%s0 [style=dashed, arrowhead=rnormal];\n",
					(const void *)child, root_str);
				} else {
					fprintf(fp,
					"tree%pleaf%u [color=Gray75, arrowhead=rnormal];\n",
					(const void *)tr, leaf);
				}
			}
			fprintf(fp, "\ttree%pbranch%u -> ", (const void *)tr, b);
			if(right) {
				fprintf(fp, "tree%pbranch%u [arrowhead=lnormal];\n",
					(const void *)tr, b + left + 1);
			} else {
				unsigned leaf = PT_(left_leaf)(tr, b) + left + 1;
				if(trie_bmp_test(&tr->bmp, leaf)) {
					const struct PT_(tree) *const child =tr->leaf[leaf].as_link;
					const char *root_str = child->bsize ? "branch" : "leaf";
					fprintf(fp,
					"tree%p%s0 [style=dashed, arrowhead=lnormal];\n",
					(const void *)child, root_str);
				} else {
					fprintf(fp,
					"tree%pleaf%u [color=Gray75, arrowhead=lnormal];\n",
					(const void *)tr, leaf);
				}
			}
		}
	}

	fprintf(fp, "\t// leaves\n");

	for(i = 0; i <= tr->bsize; i++) if(!trie_bmp_test(&tr->bmp, i)) fprintf(fp,
		"\ttree%pleaf%u [label = <%s<font color=\"Gray75\">⊔</font>>];\n",
		(const void *)tr, i,
		PT_(key_string)(PT_(entry_key)(&tr->leaf[i].as_entry)));

	for(i = 0; i <= tr->bsize; i++) if(trie_bmp_test(&tr->bmp, i))
		PT_(graph_tree_logic)(tr->leaf[i].as_link, 0, fp);
}

/** Draw a graph of `trie` to `fn` in Graphviz format with `callback` as it's
 tree-drawing output. */
static void PT_(graph_choose)(const struct T_(trie) *const trie,
	const char *const fn, const PT_(tree_file_fn) callback) {
	FILE *fp;
	assert(trie && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\tgraph [truecolor=true, bgcolor=transparent, fontname=modern];\n"
		"\tnode [shape=none, fontname=modern];\n");
	if(!trie->root) fprintf(fp, "\tidle;\n");
	else if(trie->root->bsize == USHRT_MAX) fprintf(fp, "\tempty;\n");
	else callback(trie->root, 0, fp);
	fprintf(fp, "}\n");
	fclose(fp);
}

/** Graphs logical `trie` output to `fn` using `no` as the filename index. */
static void PT_(graph)(const struct T_(trie) *const trie,
	const char *const fn, const size_t no) {
	const char logic[] = "-tree", mem[] = "-mem", bits[] = "-bits";
	char copy[128], *dot;
	size_t fn_len = strlen(fn), i, i_copy;
	if(fn_len > sizeof copy - 30/*SIZE_MAX*/ - 1 || !(dot = strrchr(fn, '.'))) {
		fprintf(stderr, "Too long or doesn't have extension: <%s>.\n", fn);
		assert(0);
		return;
	}
	printf(" *** graph: \"%s\" %lu.\n", fn, (unsigned long)no);
	/* Insert number. */
	i = (size_t)(dot - fn), memcpy(copy, fn, i_copy = i);
	copy[i_copy++] = '-';
	sprintf(copy + i_copy, "%lu", (unsigned long)no),
		i_copy += strlen(copy + i_copy);

	memcpy(copy + i_copy, bits, sizeof bits - 1);
	memcpy(copy + i_copy + sizeof bits - 1, fn + i, fn_len - i + 1);
	PT_(graph_choose)(trie, copy, &PT_(graph_tree_bits));
	memcpy(copy + i_copy, logic, sizeof logic - 1);
	memcpy(copy + i_copy + sizeof logic - 1, fn + i, fn_len - i + 1);
	PT_(graph_choose)(trie, copy, &PT_(graph_tree_logic));
	memcpy(copy + i_copy, mem, sizeof mem - 1);
	memcpy(copy + i_copy + sizeof mem - 1, fn + i, fn_len - i + 1);
	PT_(graph_choose)(trie, copy, &PT_(graph_tree_mem));
}

#if 0
/** Prints `tree` to `stdout`; useful in debugging. */
static void PT_(print)(const struct PT_(tree) *const tree) {
	const struct trie_branch *branch;
	unsigned b, i;
	assert(tree);
	printf("%s:\n"
		"left ", orcify(tree));
	for(b = 0; b < tree->bsize; b++) branch = tree->branch + b,
		printf("%s%u", b ? ", " : "", branch->left);
	printf("\n"
		"skip ");
	for(b = 0; b < tree->bsize; b++) branch = tree->branch + b,
		printf("%s%u", b ? ", " : "", branch->skip);
	printf("\n"
		"leaves ");
	for(i = 0; i <= tree->bsize; i++)
		printf("%s%s", i ? ", " : "", trie_bmp_test(&tree->bmp, i)
			? orcify(tree->leaf[i].as_link)
			: PT_(key_string)(PT_(entry_key)(tree->leaf[i].as_entry)));
	printf("\n");
}
#endif

/** Make sure `tree` is in a valid state, (and all the children.) */
static void PT_(valid_tree)(const struct PT_(tree) *const tree) {
	unsigned i;
	int cmp = 0;
	const char *str1 = 0;
	assert(tree && tree->bsize <= TRIE_BRANCHES);
	for(i = 0; i < tree->bsize; i++)
		assert(tree->branch[i].left < tree->bsize - i);
	for(i = 0; i <= tree->bsize; i++) {
		if(trie_bmp_test(&tree->bmp, i)) {
			PT_(valid_tree)(tree->leaf[i].as_link);
		} else {
			const char *str2;
			str2 = PT_(key_string)(PT_(entry_key)(&tree->leaf[i].as_entry));
			if(str1) cmp = strcmp(str1, str2), assert(cmp < 0);
			str1 = str2;
		}
	}
}

/** Makes sure the `trie` is in a valid state. */
static void PT_(valid)(const struct T_(trie) *const trie) {
	if(!trie || !trie->root) return;
	PT_(valid_tree)(trie->root);
}

static void PT_(test)(void) {
	struct T_(trie) trie = T_(trie)();
	size_t i, unique, count;
	unsigned letter_counts[UCHAR_MAX];
	const size_t letter_counts_size
		= sizeof letter_counts / sizeof *letter_counts;
	struct { PT_(entry) entry; int is_in; } tests[2000], *test_end, *test;
	const size_t tests_size = sizeof tests / sizeof *tests;
	PT_(entry) *e; //fix
	PT_(key) k;
#ifdef TRIE_VALUE
	PT_(value) *v;
#endif

	/* Idle. */
	errno = 0;
	PT_(valid)(0);
	PT_(valid)(&trie);
	PT_(graph)(&trie, "graph/" QUOTE(TRIE_NAME) "-idle.gv", 0);
	T_(trie_)(&trie), PT_(valid)(&trie);
	e = T_(trie_match)(&trie, ""), assert(!e);
	e = T_(trie_get)(&trie, ""), assert(!e);

	/* Make random data. */
	for(test = tests, test_end = test + tests_size; test < test_end; test++) {
#ifndef TRIE_VALUE /* <!-- key set */
		T_(filler)(&test->entry);
#elif !defined(TRIE_KEY_IN_VALUE) /* ket set --><!-- key map */
		T_(filler)(&test->entry.key, &test->entry.value);
#else /* key map --><!-- custom */
		T_(filler)(&test->entry);
#endif /* custom --> */
		test->is_in = 0;
	}
	/*for(test = tests, test_end = test + tests_size; test < test_end; test++)
		printf("%s\n", PT_(key_string)(PT_(entry_key)(&test->entry)));*/

	/* Adding. */
	unique = 0;
	memset(letter_counts, 0, sizeof letter_counts);
	for(i = 0; i < tests_size; i++) {
		int show = !((i + 1) & i) || i + 1 == tests_size;
		PT_(key) key;
#ifdef TRIE_VALUE
		PT_(value) *value;
#endif
		test = tests + i;
		key = PT_(entry_key)(&test->entry);
		if(show) printf("%lu: adding %s.\n",
			(unsigned long)i, PT_(key_string)(key));
		switch(
#ifndef TRIE_VALUE /* <!-- key set */
		T_(trie_try)(&trie, key)
#else /* key set --><!-- map */
		T_(trie_try)(&trie, key, &value)
#endif /* map --> */
		) {
		case TRIE_ERROR: perror("trie"); assert(0); return;
		case TRIE_UNIQUE: test->is_in = 1; unique++;
			letter_counts[(unsigned char)*PT_(key_string)(key)]++;
#ifndef TRIE_VALUE /* <!-- set */
#elif !defined(TRIE_KEY_IN_VALUE) /* set --><!-- map */
			*value = test->entry.value;
#else /* map --><!-- custom */
			*value = test->entry;
#endif /* custom --> */
			break;
		case TRIE_PRESENT: /*printf("Key %s is in trie already.\n",
			PT_(key_string)(key)); spam */ break;
		}
		if(show) {
			printf("Now: %s.\n", T_(trie_to_string)(&trie));
			PT_(graph)(&trie, "graph/" QUOTE(TRIE_NAME) "-insert.gv", i);
		}
		assert(!errno);
	}
	PT_(valid)(&trie);
	/* Check keys -- there's some key that's there. */
	for(i = 0; i < tests_size; i++) {
		const char *estring, *const tstring
			= PT_(key_string)(PT_(entry_key)(&tests[i].entry));
		e = T_(trie_get)(&trie, tstring);
		assert(e);
		estring = PT_(key_string)(PT_(entry_key)(e));
		assert(e && !strcmp(estring, tstring));
	}
	/* Add up all the letters; should be equal to the overall count. */
	for(count = 0, i = 0; i < letter_counts_size; i++) {
		char letter[2];
		unsigned count_letter = 0;
		struct T_(trie_iterator) it;
		int output = 0;
		letter[0] = (char)i, letter[1] = '\0';
		it = T_(trie_prefix)(&trie, letter);
		while(
#ifdef TRIE_VALUE
			T_(trie_next)(&it, &k, &v)
#else
			T_(trie_next)(&it, &k)
#endif
			) {
			/*printf("%s<%s>", output ? "" : letter,
				PT_(key_string)(PT_(entry_key)(e)));*/
			count_letter++, output = 1;
		}
		/*if(output) printf("\n");*/
		if(i) {
			assert(count_letter == letter_counts[i]);
			count += count_letter;
		} else { /* Sentinel; "" gets all the trie. */
			assert(count_letter == unique);
			if(T_(trie_get)(&trie, "")) count++; /* Is "" included? */
		}
	}
	printf("Counted by letter %lu elements, checksum %lu.\n",
		(unsigned long)count, (unsigned long)unique);
	assert(count == unique);
	T_(trie_clear)(&trie);
	{
		struct T_(trie_iterator) it = T_(trie_prefix)(&trie, "");
#ifdef TRIE_VALUE
		assert(!T_(trie_next)(&it, 0, 0));
#else
		assert(!T_(trie_next)(&it, 0));
#endif
	}
	T_(trie_)(&trie), assert(!trie.root), PT_(valid)(&trie);
	assert(!errno);
}

/* This is NOT standardized, but works in MSVC and GNU. Testing is allowable. */
#pragma push_macro("BOX_TYPE")
#pragma push_macro("BOX_CONTENT")
#pragma push_macro("BOX_")
#pragma push_macro("BOX_MAJOR_NAME")
#pragma push_macro("BOX_MINOR_NAME")
#undef BOX_TYPE
#undef BOX_CONTENT
#undef BOX_
#undef BOX_MAJOR_NAME
#undef BOX_MINOR_NAME
/* Pointer array for random sampling. */
#define ARRAY_NAME PT_(handle)
#define ARRAY_TYPE PT_(entry) *
#include "array.h"
/* Backing for the trie. */
#define POOL_NAME PT_(entry)
#define POOL_TYPE PT_(entry)
#include "pool.h"
#pragma pop_macro("BOX_MINOR_NAME")
#pragma pop_macro("BOX_MAJOR_NAME")
#pragma pop_macro("BOX_")
#pragma pop_macro("BOX_CONTENT")
#pragma pop_macro("BOX_TYPE")

static void PT_(test_random)(void) {
	struct PT_(entry_pool) entries = PT_(entry_pool)();
	struct PT_(handle_array) handles = PT_(handle_array)();
	struct T_(trie) trie = T_(trie)();
	const size_t expectation = 1000;
	size_t i, size = 0;
	FILE *const fp = fopen("graph/" QUOTE(TRIE_NAME) "-random.data", "w");
	printf("Random test; expectation value of items %lu.\n",
		(unsigned long)expectation);
	if(!fp) goto catch;
	for(i = 0; i < 5 * expectation; i++) {
		size_t j;
		if((unsigned)rand() > size * (RAND_MAX / (2 * expectation))) {
			/* Create item. */
			PT_(entry) *entry, **handle;
			PT_(key) key;
#ifdef TRIE_VALUE
			PT_(value) *value;
#endif
			if(!(entry = PT_(entry_pool_new)(&entries))) goto catch;
#if defined(TRIE_VALUE) && !defined(TRIE_KEY_IN_VALUE)
			T_(filler)(&entry->key, &entry->value);
#else
			T_(filler)(entry);
#endif
			key = PT_(entry_key)(entry);
			/*printf("Creating %s: ", PT_(key_string)(key));*/
			switch(
#ifndef TRIE_VALUE /* <!-- key set */
			T_(trie_try)(&trie, key)
#else /* key set --><!-- map */
			T_(trie_try)(&trie, key, &value)
#endif /* map --> */
			) {
			case TRIE_ERROR:
				/*printf("error.\n");*/
				goto catch;
			case TRIE_UNIQUE:
				/*printf("unique.\n");*/
				size++;
#ifdef TRIE_KEY_IN_VALUE
				*value = *entry;
#elif defined(TRIE_VALUE)
				*value = entry->value;
#endif
				if(!(handle = PT_(handle_array_new)(&handles))) goto catch;
				*handle = entry;
				break;
			case TRIE_PRESENT:
				/*printf("present.\n");*/
				PT_(entry_pool_remove)(&entries, entry);
				break;
			}
		} else { /* Delete item. */
			unsigned r = (unsigned)rand() / (RAND_MAX / handles.size + 1);
			PT_(entry) *handle = handles.data[r];
			const char *const string = PT_(key_string)(PT_(entry_key)(handle));
			int result;
			/*printf("Deleting %s.\n", string);*/
			result = T_(trie_remove)(&trie, string);
			assert(result);
			PT_(handle_array_lazy_remove)(&handles, handles.data + r);
			PT_(entry_pool_remove)(&entries, handle);
			size--;
		}
		if(fp) fprintf(fp, "%lu\n", (unsigned long)size);
		if(i % (5 * expectation / 10) == 5 * expectation / 20)
			PT_(graph)(&trie, "graph/" QUOTE(TRIE_NAME) "-step.gv", i);
		for(j = 0; j < handles.size; j++) {
			PT_(entry) *e = T_(trie_get)(&trie,
				PT_(key_string)(PT_(entry_key)(handles.data[j])));
			assert(e);
		}
	}
	PT_(graph)(&trie, "graph/" QUOTE(TRIE_NAME) "-step.gv", i);
	/*for(i = 0; i < handles.size; i++) printf("%s\n",
		PT_(key_string)(PT_(entry_key)(handles.data[i])));*/
	goto finally;
catch:
	perror("random test");
	assert(0);
finally:
	if(fp) fclose(fp);
	T_(trie_)(&trie);
	PT_(entry_pool_)(&entries);
	PT_(handle_array_)(&handles);
}

#if 0
	/* This is old code that is a superior test; merge it, maybe? */
	for( ; i; i--) {
		int is;
		show = 1/*!(i & (i - 1))*/;
		if(show) trie_str_no++;
		if(show) printf("\"%s\" remove.\n", str_array[i - 1]);
		is = str_trie_remove(&strs, str_array[i - 1]);
		if(show) trie_str_graph(&strs, "graph/str-deleted.gv");
		for(j = 0; j < sizeof str_array / sizeof *str_array; j++) {
			const char *get = str_trie_get(&strs, str_array[j]);
			const int want_to_get = j < i - 1;
			printf("Test get(%s) = %s, (%swant to get.)\n",
				str_array[j], get ? get : "<didn't find>",
				want_to_get ? "" : "DON'T ");
			assert(!(want_to_get ^ (get == str_array[j])));
		}
	}
#endif

/** Will be tested on stdout. Requires `TRIE_TEST`, and not `NDEBUG` while
 defining `assert`. @allow */
static void T_(trie_test)(void) {
	printf("<" QUOTE(TRIE_NAME) ">trie"
#ifdef TRIE_KEY
		" custom key <" QUOTE(TRIE_KEY) ">"
#endif
#ifdef TRIE_VALUE
		" value <" QUOTE(TRIE_VALUE) ">"
#endif
#ifdef TRIE_KEY_IN_VALUE
		" and the key is in the value"
#endif
		" testing using <" QUOTE(TRIE_TEST) ">:\n");
	PT_(test)();
	PT_(test_random)();
	fprintf(stderr, "Done tests of <" QUOTE(TRIE_NAME) ">trie.\n\n");
}

#undef QUOTE
#undef QUOTE_

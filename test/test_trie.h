#include <stdio.h>
#include <string.h>
#include "orcish.h"

#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** Works by side-effects, _ie_ fills the type with data. */
typedef void (*PT_(action_fn))(const char **);
/* Used in <fn:<PT>graph_choose>. */
typedef void (*PT_(tree_file_fn))(const struct PT_(tree) *, size_t, FILE *);

#ifndef TRIE_SET /* <!-- !set: Don't bother trying to test it automatically. */

/* `TRIE_TEST` must be a function that implements <typedef:<PT>action_fn>. */
static const PT_(action_fn) PT_(filler) = (TRIE_TEST);

#endif /* set --> */

/* Debug number, which is the number printed next to the graphs, _etc_. */
static unsigned PT_(no);

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

/** Given a branch `b` in `tree` branches, calculate the right child branches.
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

/** @return Follows the branches to `b` in `tree` and returns the leaf. */
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

/** Graphs `tree` on `fp`. `treebit` is the number of bits currently
 (recursive.) */
static void PT_(graph_tree_bits)(const struct PT_(tree) *const tr,
	const size_t treebit, FILE *const fp) {
	unsigned b, i;
	assert(tr && fp);
	fprintf(fp, "\ttree%pbranch0 [label = <\n"
		"<table border=\"0\" cellspacing=\"0\">\n", (const void *)tr);
	/*"<table BORDER=\"0\" CELLBORDER=\"0\">\n"*/
	for(i = 0; i <= tr->bsize; i++) {
		const char *key = PT_(sample)(tr, i);
		const struct trie_branch *branch = tr->branch;
		size_t next_branch = treebit + branch->skip;
		const char *params, *start, *end;
		struct { unsigned br0, br1; } in_tree;
		const unsigned is_link = trie_bmp_test(&tr->bmp, i);
		/* 0-width joiner "&#8288;": GraphViz gets upset when tag closed
		 immediately. */
		fprintf(fp, "\t<tr>\n"
			"\t\t<td align=\"left\" port=\"%u\">%s%s%s⊔</font></td>\n",
			i, is_link ? "↓<font color=\"Grey75\">" : "", key,
			is_link ? "" : "<font color=\"Grey75\">");
		in_tree.br0 = 0, in_tree.br1 = tr->bsize;
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
				next_branch = (branch = tr->branch + in_tree.br0)->skip;
			}
			if(b && !(b & 7)) fprintf(fp, "\t\t<td>&nbsp;</td>\n");
			fprintf(fp, "\t\t<td%s>%s%u%s</td>\n", params, start, bit, end);
		}
		fprintf(fp, "\t</tr>\n");
	}
	fprintf(fp, "</table>>];\n");
	/* Draw the lines between trees. */
	for(i = 0; i <= tr->bsize; i++) if(trie_bmp_test(&tr->bmp, i))
		fprintf(fp, "\ttree%pbranch0:%u -> tree%pbranch0 "
		"[style = dashed, arrowhead = %snormal];\n", (const void *)tr, i,
		(const void *)tr->leaf[i].as_link, PT_(leaf_to_dir)(tr, i));
	/* Recurse. */
	for(i = 0; i <= tr->bsize; i++) {
		struct { unsigned br0, br1, lf; } in_tree;
		size_t bit = treebit;
		if(!trie_bmp_test(&tr->bmp, i)) continue;
		in_tree.br0 = 0, in_tree.br1 = tr->bsize, in_tree.lf = 0;
		while(in_tree.br0 < in_tree.br1) {
			const struct trie_branch *branch = tr->branch + in_tree.br0;
			bit += branch->skip;
			if(i <= in_tree.lf + branch->left)
				in_tree.br1 = ++in_tree.br0 + branch->left;
			else
				in_tree.br0 += branch->left + 1, in_tree.lf += branch->left + 1;
			bit++;
		}
		PT_(graph_tree_bits)(tr->leaf[i].as_link, bit, fp);
	}
}

/** Graphs `tree` on `fp`. `treebit` is the number of bits currently
 (recursive.) */
static void PT_(graph_tree_mem)(const struct PT_(tree) *const tr,
	const size_t treebit, FILE *const fp) {
	const struct trie_branch *branch;
	unsigned i;
	(void)treebit;
	assert(tr && fp);
	/* Tree is one record node in memory -- GraphViz says html is
	 case-insensitive, but no. */
	fprintf(fp, "\ttree%pbranch0 [label = <\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td colspan=\"3\" align=\"left\">"
		"<font color=\"Grey75\">%s</font></td></tr>\n"
		"\t<hr/>\n"
		"\t<tr><td colspan=\"3\" align=\"left\">start bit %lu"
		"</td></tr>\n"
		"\t<tr>\n"
		"\t\t<td><font face=\"Times-Italic\">left</font></td>\n"
		"\t\t<td><font face=\"Times-Italic\">skip</font></td>\n"
		"\t\t<td><font face=\"Times-Italic\">leaves</font></td>\n"
		"\t</tr>\n"
		"\t<hr/>\n", (const void *)tr, orcify(tr), (unsigned long)treebit);
	for(i = 0; i <= tr->bsize; i++) {
		const char *const bgc = i & 1 ? " bgcolor=\"Gray95\"" : "";
		const char *key = PT_(sample)(tr, i);
		const unsigned is_link = trie_bmp_test(&tr->bmp, i);
		if(i < tr->bsize) {
			branch = tr->branch + i;
			fprintf(fp, "\t<tr>\n"
				"\t\t<td align=\"right\"%s>%u</td>\n"
				"\t\t<td align=\"right\"%s>%u</td>\n",
				bgc, branch->left,
				bgc, branch->skip);
		} else {
			fprintf(fp, "\t<tr>\n"
				"\t\t<td>&#8205;</td>"
				"\t\t<td>&#8205;</td>");
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
	for(i = 0; i <= tr->bsize; i++) if(trie_bmp_test(&tr->bmp, i))
		fprintf(fp, "\ttree%pbranch0:%u -> tree%pbranch0 "
		"[style = dashed, arrowhead = %snormal];\n", (const void *)tr, i,
		(const void *)tr->leaf[i].as_link, PT_(leaf_to_dir)(tr, i));
	/* Recurse. */
	for(i = 0; i <= tr->bsize; i++) {
		struct { unsigned br0, br1, lf; } in_tree;
		size_t bit = treebit;
		if(!trie_bmp_test(&tr->bmp, i)) continue;
		in_tree.br0 = 0, in_tree.br1 = tr->bsize, in_tree.lf = 0;
		while(in_tree.br0 < in_tree.br1) {
			branch = tr->branch + in_tree.br0;
			bit += branch->skip;
			if(i <= in_tree.lf + branch->left)
				in_tree.br1 = ++in_tree.br0 + branch->left;
			else
				in_tree.br0 += branch->left + 1, in_tree.lf += branch->left + 1;
			bit++;
		}
		PT_(graph_tree_mem)(tr->leaf[i].as_link, bit, fp);
	}
}

/** Graphs `tree` on `fp`.`treebit` is the number of bits currently
 (recursive.) */
static void PT_(graph_tree_logic)(const struct PT_(tree) *const tr,
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
				if(trie_bmp_test(&tr->bmp, leaf)) fprintf(fp,
					"tree%pbranch0 [style=dashed, arrowhead=rnormal];\n",
					(const void *)tr->leaf[leaf].as_link);
				else fprintf(fp,
					"tree%pleaf%u [color=Gray75, arrowhead=rnormal];\n",
					(const void *)tr, leaf);
			}
			fprintf(fp, "\ttree%pbranch%u -> ", (const void *)tr, b);
			if(right) {
				fprintf(fp, "tree%pbranch%u [arrowhead=lnormal];\n",
					(const void *)tr, b + left + 1);
			} else {
				unsigned leaf = PT_(left_leaf)(tr, b) + left + 1;
				if(trie_bmp_test(&tr->bmp, leaf)) fprintf(fp,
					"tree%pbranch0 [style=dashed, arrowhead=lnormal];\n",
					(const void *)tr->leaf[leaf].as_link);
				else fprintf(fp,
					"tree%pleaf%u [color=Gray75, arrowhead=lnormal];\n",
					(const void *)tr, leaf);
			}
		}
	}

	fprintf(fp, "\t// leaves\n");

	for(i = 0; i <= tr->bsize; i++) if(!trie_bmp_test(&tr->bmp, i)) fprintf(fp,
		"\ttree%pleaf%u [label = <%s<font color=\"Gray75\">⊔</font>>];\n",
		(const void *)tr, i, PT_(entry_key)(&tr->leaf[i].as_entry));

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
	/*"\tnode [shape=box, style=filled, fillcolor=\"Gray95\"];\n"*/
	if(!trie->root) fprintf(fp, "\tidle;\n");
	else if(trie->root->bsize == UCHAR_MAX) fprintf(fp, "\tempty;\n");
	else callback(trie->root, 0, fp);
	fprintf(fp, "}\n");
	fclose(fp);
}

/** Graphs logical `trie` output to `fn`. */
static void PT_(graph)(const struct T_(trie) *const trie,
	const char *const fn) {
	const char logic[] = "-tree", mem[] = "-mem", bits[] = "-bits";
	char name[128], *dash, *dot;
	size_t fn_len = strlen(fn), i, i_fn, i_name;
	/* Whatever we're going to add to the string. */
	if(fn_len > sizeof name - 30 - 1
		|| !(dash = strchr(fn, '-')) || !(dot = strchr(dash, '.'))) {
		fprintf(stderr, "Too long or doesn't '-' and then '.': <%s>.\n", fn);
		assert(0);
		return;
	}
	printf("graph.%u: base %s.\n", PT_(no), fn);
	i = (size_t)(dash - fn), memcpy(name, fn, i_name = i_fn = i);
	name[i_name++] = '-';
	sprintf(name + i_name, "%u", PT_(no)), i_name += strlen(name + i_name);
	i = (size_t)(dot - fn) - i_fn, memcpy(name + i_name, fn + i_fn, i),
		i_name += i, i_fn += i;

	memcpy(name + i_name, logic, sizeof logic - 1);
	memcpy(name + i_name + sizeof logic - 1, fn + i_fn, fn_len - i_fn + 1);
	PT_(graph_choose)(trie, name, &PT_(graph_tree_logic));
	memcpy(name + i_name, mem, sizeof mem - 1);
	memcpy(name + i_name + sizeof mem - 1, fn + i_fn, fn_len - i_fn + 1);
	PT_(graph_choose)(trie, name, &PT_(graph_tree_mem));
	memcpy(name + i_name, bits, sizeof bits - 1);
	memcpy(name + i_name + sizeof bits - 1, fn + i_fn, fn_len - i_fn + 1);
	PT_(graph_choose)(trie, name, &PT_(graph_tree_bits));
}

/** Prints `tree` to `stdout`. */
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
			: PT_(entry_key)(&tree->leaf[i].as_entry));
	printf("\n");
}

#ifndef TRIE_DEFAULT_TEST /* <!-- !set: a set of strings is not testable in the
 automatic framework, but convenient to have graphs for manual tests. */

/** Make sure `tree` is in a valid state, (and all the children.) */
static void PT_(valid_tree)(const struct PT_(tree) *const tree) {
	unsigned i;
	int cmp = 0;
	const char *str1 = 0;
	assert(tree && tree->bsize <= TRIE_BRANCHES);
	for(i = 0; i < tree->bsize; i++)
		assert(tree->branch[i].left < tree->bsize - 1 - i);
	for(i = 0; i <= tree->bsize; i++) {
		if(trie_bmp_test(&tree->bmp, i)) {
			PT_(valid_tree)(tree->leaf[i].as_link);
		} else {
			const char *str2;
			str2 = PT_(entry_key)(&tree->leaf[i].as_entry);
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
	/*struct T_(trie_cursor) it;*/
	size_t n;
	struct { struct PT_(entry) data;
		int is_in; } es[2000], *e;
	const size_t es_size = sizeof es / sizeof *es;
	struct PT_(entry) *data;

	/* Idle. */
	PT_(valid)(0);
	PT_(valid)(&trie);
	PT_(graph)(&trie, "graph/" QUOTE(TRIE_NAME) "-idle.gv");
	T_(trie_)(&trie), PT_(valid)(&trie);
	data = T_(trie_match)(&trie, ""), assert(!data);
	data = T_(trie_get)(&trie, ""), assert(!data);

	/* Make random data. */
	for(n = 0; n < es_size; n++) {
		e = es + n;
		PT_(filler)(&e->data.key); /* FIXME */
		e->is_in = 0;
	}

#if 0
	/* Adding. */
	PT_(no) = 0;
	count = 0;
	errno = 0;
	for(n = 0; n < es_size; n++) {
		show = !((n + 1) & n) || n + 1 == es_size;
		if(show) PT_(no)++;
		if(show) printf("%lu: adding %s.\n",
			(unsigned long)n, PT_(to_key)(es[n].data));
		es[n].is_in = T_(trie_try)(&trie, es[n].data);
		if(show) PT_(graph)(&trie, "graph/" QUOTE(TRIE_NAME) "-sample.gv");
		assert(!errno);
		if(!es[n].is_in) {
			if(show) printf("%lu: duplicate value.\n", (unsigned long)n);
			continue;
		}
		count++;
		for(m = 0; m <= n; m++) {
			if(!es[m].is_in) continue;
			data = T_(trie_get)(&trie, PT_(to_key)(&es[m].data));
			/* This is O(n^2) spam.
			printf("%lu: test get(%s) = %s\n", (unsigned long)n,
				PT_(to_key)(&es[m].data),
				data ? PT_(to_key)(data) : "<didn't find>");*/
			assert(data == &es[m].data);
		}
	}

	/* Test prefix and size. */
	{
		size_t sum = !!T_(trie_get)(&trie, "");
		for(n = 1; n < 256; n++) {
			char a[2] = { '\0', '\0' };
			a[0] = (char)n;
			T_(trie_prefix)(&trie, a, &it);
			sum += T_(trie_size)(&it);
		}
		T_(trie_prefix)(&trie, "", &it), n = T_(trie_size)(&it);
		printf("Trie %s, %lu items inserted; %lu items counted;"
			" sum of sub-trees %lu.\n", T_(trie_to_string)(&trie), count, n,
			sum), assert(n == count && n == sum);
	}
#endif

#if 0
	/* Replacement. */
	ret = T_(trie_add)(&trie, &es[0].data); /* Doesn't add. */
	assert(!ret);
	ret = T_(trie_put)(&trie, &es[0].data, 0); /* Replaces with itself. */
	assert(ret);
	ret = T_(trie_put)(&trie, &es[0].data, &data); /* Replaces with itself. */
	assert(ret && data == &es[0].data);
	ret = T_(trie_policy_put)(&trie, &es[0].data, 0, 0); /* Does add. */
	assert(ret);
	ret = T_(trie_policy_put)(&trie, &es[0].data, &data, 0); /* Does add. */
	assert(ret && data == &es[0].data);
	memcpy(&dup, &es[0].data, sizeof dup);
	ret = T_(trie_policy_put)(&trie, &dup, &data, &PT_(false)); /* Not add. */
	assert(ret && data == &dup);
	ret = T_(trie_policy_put)(&trie, &dup, &data, &PT_(true)); /* Add. */
	assert(ret && data == &es[0].data), es[0].is_in = 0;
	T_(trie_prefix)(&trie, "", &it);
	m = T_(trie_size)(&it);
	printf("Trie %lu items added (some of them duplicates),"
		" size: %lu before, replacement %lu.\n",
		(unsigned long)es_size, (unsigned long)n, (unsigned long)m);
	assert(n == m);
	/* Restore the original. */
	ret = T_(trie_put)(&trie, &es[0].data, 0); /* Add. */
	assert(ret && data == &es[0].data), es[0].is_in = 1;

	for(n = 0; n < es_size; n++) {
		const char *key;
		if(!es[n].is_in) { /*printf("es %lu is not in\n", n);*/ continue; }
		key = PT_(to_key)(&es[n].data);
		data = T_(trie_remove)(&trie, key);
		assert(data == &es[n].data);
		es[n].is_in = 0;
		data = T_(trie_get)(&trie, key), assert(!data);
		T_(trie_prefix)(&trie, "", &it), count = T_(trie_size)(&it);
		show = !((count + 1) & count);
		if(show) {
			PT_(no)++;
			printf("%lu: removed \"%s\" from trie.\n", (unsigned long)n, key);
			PT_(graph)(&trie, "graph/" QUOTE(TRIE_NAME) "-remove.gv");
		}
	}
#endif

	T_(trie_)(&trie), assert(!trie.root), PT_(valid)(&trie);
	assert(!errno);
}

/** Will be tested on stdout. Requires `TRIE_TEST`, and not `NDEBUG` while
 defining `assert`. @allow */
static void T_(trie_test)(void) {
	printf("<" QUOTE(TRIE_NAME) ">trie"
		" of type <" QUOTE(TRIE_VALUE) ">"
		" was created using: TREE_KEY<" QUOTE(TRIE_KEY_IN_VALUE) ">;"
		" TRIE_TEST <" QUOTE(TRIE_TEST) ">;"
		" testing:\n");
	PT_(test)();
	fprintf(stderr, "Done tests of <" QUOTE(TRIE_NAME) ">trie.\n\n");
}

#endif /* !set --> */

#undef QUOTE
#undef QUOTE_

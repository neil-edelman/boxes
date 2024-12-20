#include <stdio.h>
#include <string.h>
#include "orcish.h"

#if defined(QUOTE) || defined(QUOTE_)
#	error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** This makes the key-value in the same place; will have to copy. */
struct pT_(tree_test) {
	int in;
	pT_(key) key;
#ifdef TREE_VALUE
	pT_(value) value;
#endif
};

#ifdef TREE_VALUE
typedef void (*pT_(action_fn))(pT_(key) *, pT_(value) *);
#else
typedef void (*pT_(action_fn))(pT_(key) *);
#endif

/** Recursively draws `outer` in `fp` with the actual `height`. */
static void pT_(subgraph)(const struct pT_(tree) *const sub, FILE *fp) {
	const struct pT_(branch) *branch;
	unsigned i;
	assert(sub->node && fp);
	fprintf(fp, "\ttrunk%p [label = <\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td border=\"0\" port=\"0\">"
		"<font color=\"Gray75\">%s</font></td></tr>\n",
		(const void *)sub->node, orcify(sub->node));
	if(sub->node->size) fprintf(fp, "\t<hr/>\n");
	for(i = 0; i < sub->node->size; i++) {
		const char *const bgc = i & 1 ? " bgcolor=\"Gray95\"" : "";
		char z[12];
#ifdef TREE_VALUE
		t_(to_string)(sub->node->key[i], sub->node->value + i, &z);
#else
		t_(to_string)(sub->node->key[i], &z);
#endif
		fprintf(fp, "\t<tr><td border=\"0\" align=\"left\""
			" port=\"%u\"%s>%s</td></tr>\n", i + 1, bgc, z);
	}
	fprintf(fp, "\t<hr/>\n"
		"\t<tr><td></td></tr>\n"
		"</table>>];\n");
	if(!sub->height) return;
	/* Draw the lines between trees. */
	branch = pT_(as_branch_c)(sub->node);
	for(i = 0; i <= branch->base.size; i++)
		fprintf(fp, "\ttrunk%p:%u:se -> trunk%p;\n",
		(const void *)sub->node, i, (const void *)branch->child[i]);
	/* Recurse. */
	for(i = 0; i <= branch->base.size; i++) {
		struct pT_(tree) child;
		child.node = branch->child[i], child.height = sub->height - 1;
		pT_(subgraph)(&child, fp);
	}
}

/** Draw a graph of `tree` to `fn` in Graphviz format. */
static void pT_(graph)(const struct t_(tree) *const tree,
	const char *const fn) {
	FILE *fp;
	assert(tree && fn);
	printf("*** %s.\n\n", fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\tgraph [rankdir=LR, truecolor=true, bgcolor=transparent,"
		" fontname=modern, splines=false];\n"
		"\tnode [shape=none, fontname=modern];\n");
	if(!tree->root.node)
		fprintf(fp, "\tidle [shape=plaintext];\n");
	else if(tree->root.height == UINT_MAX)
		fprintf(fp, "\tempty [shape=plaintext];\n");
	else pT_(subgraph)(&tree->root, fp);
	fprintf(fp, "\tnode [color=\"Red\"];\n"
		"}\n");
	fclose(fp);
}

/** Aligns the `port` in the right way between nodes. */
static char *pT_(usual_port)(unsigned port) {
	static char s[32];
	if(!port) sprintf(s, "0:sw");
	else sprintf(s, "%u:se", port - 1);
	return s;
}

/** Recursively draws `outer` in `fp` with the actual `height`. */
static void pT_(subgraph_usual)(const struct pT_(tree) *const sub, FILE *fp) {
	const struct pT_(branch) *branch;
	unsigned i;
	assert(sub->node && fp);
	fprintf(fp, "\ttrunk%p [label = <\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td border=\"0\" colspan=\"%u\">"
		"<font color=\"Gray75\">%s</font></td></tr>\n"
		"\t<hr/>\n"
		"\t<tr>\n", (const void *)sub->node,
		sub->node->size ? sub->node->size : 1, orcify(sub->node));
	for(i = 0; i < sub->node->size; i++) {
		char z[12];
#ifdef TREE_VALUE
		t_(to_string)(sub->node->key[i], sub->node->value + i, &z);
#else
		t_(to_string)(sub->node->key[i], &z);
#endif
		fprintf(fp, "\t<td border=\"0\" align=\"center\""
			" port=\"%u\">%s</td>\n", i, z);
	}
	/* Dummy node when size is zero. */
	if(!sub->node->size)
		fprintf(fp, "\t<td border=\"0\" port=\"0\">&nbsp;</td>\n");
	fprintf(fp, "\t</tr>\n");
	if(sub->height) fprintf(fp, "\t<hr/>\n"
		"\t<tr><td></td></tr>\n");
	fprintf(fp, "</table>>];\n");
	if(!sub->height) return;
	/* Draw the lines between trees. */
	branch = pT_(as_branch_c)(sub->node);
	for(i = 0; i <= branch->base.size; i++)
		fprintf(fp, "\ttrunk%p:%s -> trunk%p;\n",
		(const void *)sub->node, pT_(usual_port)(i),
		(const void *)branch->child[i]);
	/* Recurse. */
	for(i = 0; i <= branch->base.size; i++) {
		struct pT_(tree) child;
		child.node = branch->child[i], child.height = sub->height - 1;
		pT_(subgraph_usual)(&child, fp);
	}
}

/** Draw a graph of `tree` to `fn` in Graphviz format, the usual way, but too
 large for many labels. */
static void pT_(graph_horiz)(const struct t_(tree) *const tree,
	const char *const fn) {
	FILE *fp;
	assert(tree && fn);
	printf("***(horizontal) %s.\n\n", fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\tgraph [truecolor=true, bgcolor=transparent,"
		" fontname=modern, splines=false];\n"
		"\tnode [shape=none, fontname=\"Bitstream Vera Sans\"];\n"
		"\n");
	if(!tree->root.node)
		fprintf(fp, "\tidle [shape=plaintext];\n");
	else if(tree->root.height == UINT_MAX)
		fprintf(fp, "\tempty [shape=plaintext];\n");
	else pT_(subgraph_usual)(&tree->root, fp);
	fprintf(fp, "\tnode [color=\"Red\"];\n"
		"}\n");
	fclose(fp);
}

/** Makes sure the `tree` is in a valid state. */
static void pT_(valid)(const struct t_(tree) *const tree) {
	if(!tree) return; /* Null. */
	if(!tree->root.node)
		{ assert(!tree->root.height); return; } /* Idle. */
	if(tree->root.height == UINT_MAX)
		{ assert(tree->root.node); return; } /* Empty. */
	assert(tree->root.node);
	/*...*/
}

/** Ca'n't use `qsort` with `size` because we don't have a comparison;
 <data:<PB>compare> only has to separate it into two, not three. (One can use
 `qsort` compare in this compare, but generally not the other way around.) */
static void pT_(sort)(struct pT_(tree_test) *a, const size_t size) {
	struct pT_(tree_test) temp;
	size_t i;
	for(i = 1; i < size; i++) {
		size_t j;
		for(j = i; j && t_(less)(a[j - 1].key, a[i].key) > 0; j--);
		if(j == i) continue;
		temp = a[i];
		memmove(a + j + 1, a + j, sizeof *a * (i - j));
		a[j] = temp;
	}
}

static void pT_(test)(void) {
	struct t_(tree) tree = t_(tree)(), empty = t_(tree)();
	struct T_(cursor) cur;
	struct pT_(tree_test) test[800];
	const size_t test_size = sizeof test / sizeof *test;
#ifdef TREE_VALUE
	pT_(value) *v;
#endif
	pT_(key) k, k_prev;
	size_t i, n_unique = 0, n_unique2;
	char fn[64];
	/*int succ;*/

	errno = 0;

	/* Fill. */
	for(i = 0; i < test_size; i++) {
		struct pT_(tree_test) *const t = test + i;
		t->in = 0;
		/* This function must exist. */
#ifdef TREE_VALUE
		t_(filler)(&t->key, &t->value);
#else
		t_(filler)(&t->key);
#endif
	}
	pT_(sort)(test, test_size);
	for(i = 0; i < test_size; i++) {
		struct pT_(tree_test) *const t = test + i;
		char z[12];
#ifdef TREE_VALUE
		t_(to_string)(t->key, &t->value, &z);
#else
		t_(to_string)(t->key, &z);
#endif
		/*printf("%s\n", z);*/
	}

	/* Idle. */
	pT_(valid)(0);
	pT_(valid)(&tree);
	pT_(graph)(&tree, "graph/tree/" QUOTE(TREE_NAME) "-idle.gv");
	t_(tree_)(&tree), pT_(valid)(&tree);
	/* Not valid anymore.
	it = T_(tree_less)(0, test[0].key), assert(!it._.root); */
	/*fixme what?
	it = T_(less)(&tree, test[0].key), assert(it._.root && !it._.ref.node);*/

	/* Bulk, (simple.) */
	for(i = 0; i < test_size; i++) {
		/*pT_(entry_c) e;
		char z[12];*/
		struct pT_(tree_test) *const t = test + i;
		/*e = pT_(test_to_entry_c)(t);
		pT_(to_string)(e, &z);
		printf("%lu -- bulk adding <%s>.\n", (unsigned long)i, z);*/
		switch(
#ifdef TREE_VALUE
		T_(bulk_assign)(&tree, t->key, &v)
#else
		T_(bulk_try)(&tree, t->key)
#endif
		) {
		case TREE_ERROR: perror("What?"); assert(0); break;
		case TREE_PRESENT:
			/*assert(T_(tree_get)(&tree, pT_(test_to_key)(t)));*/
			break;
		case TREE_ABSENT:
			n_unique++;
#ifdef TREE_VALUE
			*v = t->value;
#endif
			t->in = 1;
			break;
		}



#if 0 /* fixme: is this even a thing anymore? `get_or`? */
#ifdef TREE_VALUE
		/* Not a very good test. */
		value = T_(tree_get)(&tree, pT_(test_to_key)(t));
		assert(value);
#else
		{
			pT_(key) *pk = T_(tree_get)(&tree, pT_(test_to_key)(t));
			assert(pk && !pT_(compare)(*pk, *t));
		}
#endif
#endif



		if(!(i & (i + 1)) || i == test_size - 1) {
			sprintf(fn, "graph/tree/" QUOTE(TREE_NAME) "-bulk-%lu.gv", i + 1);
			pT_(graph)(&tree, fn);
		}
	}
	printf("Finalize.\n");
	T_(bulk_finish)(&tree);
	printf("Finalize again. This should be idempotent.\n");
	T_(bulk_finish)(&tree);
	pT_(graph)(&tree, "graph/tree/" QUOTE(TREE_NAME) "-bulk-finish.gv");
	printf("Tree: %s.\n", T_(to_string)(&tree));

	/* Iteration; checksum. */
	memset(&k_prev, 0, sizeof k_prev);
	for(cur = T_(begin)(&tree), i = 0; T_(exists)(&cur); T_(next)(&cur)) {
		/*char z[12];*/
		k = T_(key)(&cur);
/*#ifdef TREE_VALUE
		v = T_(tree_value)(&it);
		T_(to_string)(k, v, &z);
#else
		T_(to_string)(k, &z);
#endif
		printf("<%s>\n", z);*/
		if(i) { const int cmp = t_(less)(k, k_prev); assert(cmp > 0); }
		k_prev = k;
		if(++i > test_size) assert(0); /* Avoids loops. */
	}
	assert(i == n_unique);
	/*printf("\n");*/

#if 0 /*fixme*/
	/* Going the other way. */
	for(cur = T_(end)(&tree), i = 0; T_(exists)(&cur); T_(previous)(&cur)) {
		/*char z[12];*/
		k = T_(key)(&cur);
/*#ifdef TREE_VALUE
		v = T_(tree_value)(&it);
		T_(to_string)(k, v, &z);
#else
		T_(to_string)(k, &z);
#endif
		printf("<%s>\n", z);*/
		if(i) { const int cmp = t_(less)(k_prev, k); assert(cmp > 0); }
		k_prev = k;
		if(++i > test_size) assert(0); /* Avoids loops. */
	}
	assert(i == n_unique);
#endif

#if 0 /*fixme*/
	/* Deleting while iterating. */
	cur = T_(begin)(&tree);
	succ = T_(previous)(&cur);
	assert(succ);
	do {
		/*char z[12];*/
		pT_(key) key = T_(key)(&cur);
/*#ifdef TREE_VALUE
		T_(to_string)(key, T_(tree_value)(&it), &z);
#else
		T_(to_string)(key, &z);
#endif
		printf("removing <%s>\n", z);*/
		succ = T_(remove)(&tree, key);
		assert(succ);
		succ = T_(remove)(&tree, key);
		assert(!succ);
		it = T_(tree_less)(&tree, key);
	} while(T_(tree_has_element)(&cur));
	printf("Individual delete tree: %s.\n", T_(tree_to_string)(&tree));
	assert(tree.root.height == UINT_MAX);
#endif

	/* Clear. */
	T_(clear)(0);
	T_(clear)(&empty), assert(!empty.root.node);
	T_(clear)(&tree), assert(tree.root.node
		&& tree.root.height == UINT_MAX);
	n_unique = 0;

	/* Fill again, this time, don't sort. */
	for(i = 0; i < test_size; i++) {
		struct pT_(tree_test) *const t = test + i;
		t->in = 0;
		/* This function must exist. */
#ifdef TREE_VALUE
		t_(filler)(&t->key, &t->value);
#else
		t_(filler)(&t->key);
#endif
	}

	/* Add. */
	for(i = 0; i < test_size; i++) {
		/*char z[12];*/
		struct pT_(tree_test) *const t = test + i;
/*#ifdef TREE_VALUE
		T_(to_string)(t->key, &t->value, &z);
#else
		T_(to_string)(t->key, &z);
#endif
		printf("%lu -- adding <%s>.\n", (unsigned long)i, z);*/
		switch(
#ifdef TREE_VALUE
		T_(assign)(&tree, t->key, &v)
#else
		T_(try)(&tree, t->key)
#endif
		) {
		case TREE_ERROR: perror("unexpected"); assert(0); return;
		case TREE_PRESENT: /*printf("<%s> already in tree\n", z);*/ break;
		case TREE_ABSENT:
			n_unique++;
#ifdef TREE_VALUE
			*v = t->value;
#endif
			t->in = 1;
			/*printf("<%s> added\n", z);*/ break;
		}
		if(!(i & (i + 1)) || i == test_size - 1) {
			sprintf(fn, "graph/tree/" QUOTE(TREE_NAME) "-add-%lu.gv", i + 1);
			pT_(graph)(&tree, fn);
		}
	}
	printf("Number of entries in the tree: %lu/%lu.\n",
		(unsigned long)n_unique, (unsigned long)test_size);

	/* Delete all. Removal invalidates iterator. */
	for(cur = T_(begin)(&tree), i = 0; T_(exists)(&cur); ) {
		/*char z[12];*/
		k = T_(key)(&cur);
/*#ifdef TREE_VALUE
		v = T_(tree_value)(&it);
		t_(to_string)(k, v, &z);
#else
		t_(to_string)(k, &z);
#endif
		printf("Targeting <%s> for removal.\n", z);*/
		if(i) { const int cmp = t_(less)(k, k_prev); assert(cmp > 0); }
		k_prev = k;
		if(++i > test_size) assert(0); /* Avoids loops. */
		assert(T_(contains)(&tree, k));
		T_(remove)(&tree, k);
		assert(!T_(contains)(&tree, k));
		cur = T_(more)(&tree, k);
		/*printf("Iterator now %s:h%u:i%u.\n",
			orcify(cur.ref.node), cur.ref.height, cur.ref.idx);*/
		if(!(i & (i + 1)) || i == test_size - 1) {
			sprintf(fn, "graph/tree/" QUOTE(TREE_NAME) "-rm-%lu.gv", i);
			pT_(graph)(&tree, fn);
		}
	}
	assert(i == n_unique);

	/* Add all back. */
	n_unique2 = 0;
	for(i = 0; i < test_size; i++) {
		struct pT_(tree_test) *const t = test + i;
		/*char z[12];*/
		if(!t->in) continue;
/*#ifdef TREE_VALUE
		T_(to_string)(t->key, &t->value, &z);
#else
		T_(to_string)(t->key, &z);
#endif
		printf("Adding %s.\n", z);*/
		switch(
#ifdef TREE_VALUE
		T_(assign)(&tree, t->key, &v)
#else
		T_(try)(&tree, t->key)
#endif
		) {
		case TREE_ERROR:
		case TREE_PRESENT: perror("unexpected"); assert(0); return;
		case TREE_ABSENT: n_unique2++; break;
		}
#ifdef TREE_VALUE
		*v = t->value;
#endif
	}
	printf("Re-add tree: %lu\n", (unsigned long)n_unique2);
	pT_(graph)(&tree, "graph/tree/" QUOTE(TREE_NAME) "-re-add.gv");
	assert(n_unique == n_unique2);
	i = T_(count)(&tree);
	printf("tree count: %lu; add count: %lu\n",
		(unsigned long)i, (unsigned long)n_unique2);
	assert(i == n_unique);

	/* Remove every 2nd. */
	for(cur = T_(begin)(&tree); T_(exists)(&cur); T_(next)(&cur)) {
		pT_(key) key = T_(key)(&cur);
		const int ret = T_(remove)(&tree, key);
		assert(ret);
		n_unique--;
		cur = T_(more)(&tree, key); /* Move past the erased keys. */
		if(!T_(exists)(&cur)) break;
	}
	i = T_(count)(&tree);
	printf("remove every 2nd: %lu\n", (unsigned long)i);
	assert(i == n_unique);

	printf("clear, destroy\n");
	T_(clear)(&tree);
	assert(tree.root.height == UINT_MAX && tree.root.node);

	/* Destroy. */
	t_(tree_)(&tree), assert(!tree.root.node), pT_(valid)(&tree);
	assert(!errno);
}

/** Will be tested on stdout. Requires `TREE_TEST`, and not `NDEBUG` while
 defining `assert`. @allow */
static void T_(test)(void) {
	printf("<" QUOTE(TREE_NAME) ">tree"
		" of key <" QUOTE(TREE_KEY) ">;"
#ifdef TREE_VALUE
		" value <" QUOTE(TREE_VALUE) ">;"
#endif
		" testing:\n");
	pT_(test)();
	fprintf(stderr, "Done tests of <" QUOTE(TREE_NAME) ">tree.\n\n");
	(void)pT_(graph_horiz); /* Not used in general. */
}

#undef QUOTE
#undef QUOTE_

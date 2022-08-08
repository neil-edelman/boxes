#include <stdio.h>
#include <string.h>
#include "orcish.h"

#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)


#ifdef TREE_VALUE
/** This makes the key-value in the same place; will have to copy. */
typedef struct B_(tree_test) {
	PB_(key) key;
	PB_(value) value;
} PB_(entry_test);
static PB_(key) PB_(test_to_key)(struct B_(tree_test) *const t)
	{ return t->key; }
static PB_(key) PB_(entry_to_key)(PB_(entry) e) { return *e.key; }
static PB_(entry_c) PB_(test_to_entry_c)(struct B_(tree_test) *const t) {
	struct B_(tree_entry_c) e;
	e.key = &t->key, e.value = &t->value;
	return e;
}
#else
typedef PB_(key) PB_(entry_test);
static PB_(key) PB_(test_to_key)(PB_(key) *const x) { return *x; }
static PB_(key) PB_(entry_to_key)(PB_(entry) e) { return *e; }
static PB_(entry_c) PB_(test_to_entry_c)(PB_(key) *const t) { return t; }
#endif

/** Works by side-effects. Only defined if `TREE_TEST`. */
typedef void (*PB_(action_fn))(PB_(entry_test) *);

/** `TREE_TEST` must be a function that implements <typedef:<PT>action_fn>.
 The value pointer is valid, if it exists, and should be filled; this will copy
 the value on successful creation. */
static const PB_(action_fn) PB_(filler) = (TREE_TEST);

/* Debug number, which is the number printed next to the graphs, _etc_. */
static unsigned PB_(no);

/** Recursively draws `outer` in `fp` with the actual `height`. */
static void PB_(subgraph)(const struct PB_(tree) *const sub, FILE *fp) {
	const struct PB_(branch) *branch;
	unsigned i;
	assert(sub->node && fp);
	fprintf(fp, "\ttrunk%p [label = <\n"
		"<table border=\"1\" cellspacing=\"0\" bgcolor=\"Grey95\">\n"
		"\t<tr><td border=\"0\" port=\"0\">"
		"<font color=\"Gray75\">%s</font></td></tr>\n",
		(const void *)sub->node, orcify(sub->node));
	for(i = 0; i < sub->node->size; i++) {
		const char *const bgc = i & 1 ? "" : " bgcolor=\"Gray90\"";
		char z[12];
		PB_(entry_c) e = PB_(leaf_to_entry_c)(sub->node, i);
		PB_(to_string)(e, &z);
		fprintf(fp, "\t<tr><td border=\"0\" align=\"left\""
			" port=\"%u\"%s>%s</td></tr>\n", i + 1, bgc, z);
	}
	fprintf(fp, "</table>>];\n");
#ifdef TREE_MULTIPLE_KEY
	if(sub->node->parent) {
		fprintf(fp, "\ttrunk%p -> trunk%p [dir=back];\n",
			(const void *)&sub->node->parent->base, (const void *)sub->node);
	}
#endif
	if(!sub->height) return;
	/* Draw the lines between trees. */
	branch = PB_(as_branch_c)(sub->node);
	for(i = 0; i <= branch->base.size; i++)
		fprintf(fp, "\ttrunk%p:%u:se -> trunk%p;\n",
		(const void *)sub->node, i, (const void *)branch->child[i]);
	/* Recurse. */
	for(i = 0; i <= branch->base.size; i++) {
		struct PB_(tree) child;
		child.node = branch->child[i], child.height = sub->height - 1;
		PB_(subgraph)(&child, fp);
	}
}

/** Draw a graph of `tree` to `fn` in Graphviz format. */
static void PB_(graph)(const struct B_(tree) *const tree,
	const char *const fn) {
	FILE *fp;
	assert(tree && fn);
	printf("*** %s.\n\n", fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\tgraph [rankdir=LR, truecolor=true, bgcolor=transparent,"
		" fontname=\"Bitstream Vera Sans\", splines=false];\n"
		"\tnode [shape=none, fontname=\"Bitstream Vera Sans\"];\n"
		"\n");
	if(!tree->root.node)
		fprintf(fp, "\tidle [shape=plaintext];\n");
	else if(tree->root.height == UINT_MAX)
		fprintf(fp, "\tempty [shape=plaintext];\n");
	else PB_(subgraph)(&tree->root, fp);
	fprintf(fp, "\tnode [color=\"Red\"];\n"
		"}\n");
	fclose(fp);
}

/** Aligns the `port` in the right way between nodes. */
static char *PB_(usual_port)(unsigned port) {
	static char s[32];
	if(!port) sprintf(s, "0:sw");
	else sprintf(s, "%u:se", port - 1);
	return s;
}

/** Recursively draws `outer` in `fp` with the actual `height`. */
static void PB_(subgraph_usual)(const struct PB_(tree) *const sub, FILE *fp) {
	const struct PB_(branch) *branch;
	unsigned i;
	assert(sub->node && fp);
	fprintf(fp, "\ttrunk%p [label = <\n"
		"<table border=\"1\" cellspacing=\"0\" bgcolor=\"Grey95\">\n"
		"\t<tr><td border=\"0\" colspan=\"%u\">"
		"<font color=\"Gray75\">%s</font></td></tr>\n"
		"\t<tr>\n", (const void *)sub->node,
		sub->node->size ? sub->node->size : 1, orcify(sub->node));
	for(i = 0; i < sub->node->size; i++) {
		const char *const bgc = i & 1 ? "" : " bgcolor=\"Gray90\"";
		char z[12];
		PB_(entry_c) e = PB_(leaf_to_entry_c)(sub->node, i);
		PB_(to_string)(e, &z);
		fprintf(fp, "\t<td border=\"0\" align=\"center\""
			" port=\"%u\"%s>%s</td>\n", i, bgc, z);
	}
	/* Dummy node when size is zero. */
	if(!sub->node->size)
		fprintf(fp, "\t<td border=\"0\" port=\"0\">&nbsp;</td>\n");
	fprintf(fp, "\t</tr>\n"
		"</table>>];\n");
	if(!sub->height) return;
	/* Draw the lines between trees. */
	branch = PB_(as_branch_c)(sub->node);
	for(i = 0; i <= branch->base.size; i++)
		fprintf(fp, "\ttrunk%p:%s -> trunk%p;\n",
		(const void *)sub->node, PB_(usual_port)(i), (const void *)branch->child[i]);
	/* Recurse. */
	for(i = 0; i <= branch->base.size; i++) {
		struct PB_(tree) child;
		child.node = branch->child[i], child.height = sub->height - 1;
		PB_(subgraph_usual)(&child, fp);
	}
}

/** Draw a graph of `tree` to `fn` in Graphviz format, the usual way, but too
 large for many labels. */
static void PB_(graph_usual)(const struct B_(tree) *const tree,
	const char *const fn) {
	FILE *fp;
	assert(tree && fn);
	printf("***(usual) %s.\n\n", fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\tgraph [truecolor=true, bgcolor=transparent,"
		" fontname=\"Bitstream Vera Sans\", splines=false];\n"
		"\tnode [shape=none, fontname=\"Bitstream Vera Sans\"];\n"
		"\n");
	if(!tree->root.node)
		fprintf(fp, "\tidle [shape=plaintext];\n");
	else if(tree->root.height == UINT_MAX)
		fprintf(fp, "\tempty [shape=plaintext];\n");
	else PB_(subgraph_usual)(&tree->root, fp);
	fprintf(fp, "\tnode [color=\"Red\"];\n"
		"}\n");
	fclose(fp);
}

static void PB_(print_r)(const struct PB_(tree) sub) {
	struct PB_(tree) child = { 0, 0 };
	const struct PB_(branch) *branch = 0;
	unsigned i;
	assert(sub.node);
	printf("/");
	if(sub.height) {
		branch = PB_(as_branch_c)(sub.node);
		child.height = sub.height - 1;
	}
	for(i = 0; ; i++) {
		char z[12];
		PB_(entry_c) e;
		if(sub.height) child.node = branch->child[i], PB_(print_r)(child);
		if(i == sub.node->size) break;
		e = PB_(leaf_to_entry_c)(sub.node, i);
		PB_(to_string)(e, &z);
		printf("%s%s", i ? ", " : "", z);
	}
	printf("\\");
}
static void PB_(print)(const struct B_(tree) *const tree) {
	printf("Inorder: ");
	if(!tree) {
		printf("null");
	} else if(!tree->root.node) {
		assert(!tree->root.height);
		printf("idle");
	} else if(tree->root.height == UINT_MAX) {
		printf("empty");
	} else {
		PB_(print_r)(tree->root);
	}
	printf("\n");
}

/** Makes sure the `trie` is in a valid state. */
static void PB_(valid)(const struct B_(tree) *const tree) {
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
static void PB_(sort)(PB_(entry_test) *a, const size_t size) {
	PB_(entry_test) temp;
	size_t i;
	for(i = 1; i < size; i++) {
		size_t j;
		for(j = i; j && PB_(compare)(PB_(test_to_key)(a + j - 1),
			PB_(test_to_key)(a + i)) > 0; j--);
		if(j == i) continue;
		temp = a[i];
		memmove(a + j + 1, a + j, sizeof *a * (i - j));
		a[j] = temp;
	}
}

static int PB_(contents)(const PB_(entry) *const e) {
#ifdef TREE_VALUE
	return !!e->key;
#else
	return !!*e;
#endif
}

static PB_(entry_c) PB_(to_const)(const PB_(entry) e) {
#ifdef TREE_VALUE
	PB_(entry_c) c; c.key = e.key, c.value = e.value;
	return c;
#else
	return e;
#endif
}

static void PB_(test)(void) {
	struct B_(tree) tree = B_(tree)(), empty = B_(tree)();
	struct B_(tree_iterator) it;
	PB_(entry_test) n[80];
	const size_t n_size = sizeof n / sizeof *n;
	PB_(entry) entry;
	PB_(value) *value;
	PB_(key) last;
	size_t i, n_unique = 0;
	char fn[64];

	errno = 0;

	/* Fill. */
	for(i = 0; i < n_size; i++) PB_(filler)(n + i);
	PB_(sort)(n, n_size);

	/* Idle. */
	PB_(valid)(0);
	PB_(valid)(&tree);
	PB_(graph)(&tree, "graph/" QUOTE(TREE_NAME) "-idle.gv");
	B_(tree_)(&tree), PB_(valid)(&tree);
	it = B_(tree_lower_iterator)(0, PB_(test_to_key)(n + 0)),
		assert(!it._.root);
	value = B_(tree_lower_value)(0, PB_(test_to_key)(n + 0)), assert(!value);
	it = B_(tree_lower_iterator)(&tree, PB_(test_to_key)(n + 0)),
		assert(!it._.i.node);
	value = B_(tree_lower_value)(&tree, PB_(test_to_key)(n + 0)),assert(!value);

	/* Bulk, (simple.) */
	for(i = 0; i < n_size; i++) {
		PB_(entry_c) ent;
		char z[12];
		PB_(entry_test) *const e = n + i;
		ent = PB_(test_to_entry_c)(e);
		PB_(to_string)(ent, &z);
		printf("%lu -- bulk adding <%s>.\n", (unsigned long)i, z);
		switch(
#ifdef TREE_VALUE
		B_(tree_bulk_add)(&tree, PB_(test_to_key)(e), &value)
#else
		B_(tree_bulk_add)(&tree, PB_(test_to_key)(e))
#endif
			){
		case TREE_ERROR: perror("What?"); assert(0); break;
		case TREE_YIELD: printf("Key <%s> is already in tree.\n", z); break;
		case TREE_UNIQUE:
			n_unique++;
#ifdef TREE_VALUE
			*value = e->value;
#endif
			break;
		}
		if(!(i & (i + 1)) || i == n_size - 1) {
			sprintf(fn, "graph/" QUOTE(TREE_NAME) "-bulk-%lu.gv", i + 1);
			PB_(graph)(&tree, fn);
		}
	}
	printf("Finalize.\n");
	B_(tree_bulk_finish)(&tree);
	printf("Finalize again. This should be idempotent.\n");
	B_(tree_bulk_finish)(&tree);
	PB_(graph)(&tree, "graph/" QUOTE(TREE_NAME) "-bulk-finalized.gv");
	printf("Tree: %s.\n", PB_(tree_to_string)(&tree));

	/* Iteration; checksum. */
	memset(&last, 0, sizeof last);
	it = B_(tree_iterator)(&tree), i = 0;
	while(entry = B_(tree_next)(&it), PB_(contents)(&entry)) {
		char z[12];
		PB_(to_string)(PB_(to_const)(entry), &z);
		printf("<%s>\n", z);
		if(i) {
			const int cmp = PB_(compare)(PB_(entry_to_key)(entry), last);
			assert(cmp > 0);
		}
		last = PB_(entry_to_key)(entry);
		if(++i > n_size) assert(0); /* Avoids loops. */
	}
	assert(i == n_unique);

	/* Clear. */
	B_(tree_clear)(0);
	B_(tree_clear)(&empty), assert(!empty.root.node);
	B_(tree_clear)(&tree), assert(tree.root.node
		&& tree.root.height == UINT_MAX);
	n_unique = 0;

	/* Fill again, this time, don't sort. */
	for(i = 0; i < n_size; i++) PB_(filler)(n + i);

	/* Add. */
	for(i = 0; i < n_size; i++) {
		PB_(entry_c) ent;
		char z[12];
		PB_(entry_test) *const e = n + i;
		ent = PB_(test_to_entry_c)(e);
		PB_(to_string)(ent, &z);
		printf("%lu -- adding <%s>.\n", (unsigned long)i, z);
#ifdef TREE_VALUE
		switch(B_(tree_add)(&tree, e->key, &value))
#else
		switch(B_(tree_add)(&tree, *e))
#endif
		{
		case TREE_ERROR: perror("unexpected"); assert(0); return;
		case TREE_YIELD: printf("<%s> already in tree\n", z); break;
		case TREE_UNIQUE: printf("<%s> added\n", z); n_unique++; break;
		}
#ifdef TREE_VALUE
		*value = e->value;
#endif
		if(!(i & (i + 1)) || i == n_size - 1) {
			sprintf(fn, "graph/" QUOTE(TREE_NAME) "-add-%lu.gv", i + 1);
			PB_(graph)(&tree, fn);
		}
	}

	/* Iteration; checksum. */
	memset(&last, 0, sizeof last);
	it = B_(tree_iterator)(&tree), i = 0;
	while(entry = B_(tree_next)(&it), PB_(contents)(&entry)) {
		char z[12];
		PB_(to_string)(PB_(to_const)(entry), &z);
		printf("<%s>\n", z);
		if(i) {
			const int cmp = PB_(compare)(PB_(entry_to_key)(entry), last);
			assert(cmp > 0);
		}
		last = PB_(entry_to_key)(entry);
		if(++i > n_size) assert(0); /* Avoids loops. */
		B_(tree_remove)(&tree, last);
		{
			sprintf(fn, "graph/" QUOTE(TREE_NAME) "-rm-%lu.gv", i);
			PB_(graph)(&tree, fn);
		}
	}
	assert(i == n_unique);

	/* Destroy. */
	B_(tree_)(&tree), assert(!tree.root.node), PB_(valid)(&tree);
	assert(!errno);
}

/** Will be tested on stdout. Requires `TRIE_TEST`, and not `NDEBUG` while
 defining `assert`. @allow */
static void B_(tree_test)(void) {
	printf("<" QUOTE(TREE_NAME) ">trie"
		" of type <" QUOTE(TREE_KEY) ">"
		" was created using:"
#ifdef TREE_VALUE
		" TREE_VALUE<" QUOTE(TREE_VALUE) ">;"
#endif
		" TRIE_TEST <" QUOTE(TREE_TEST) ">;"
		" testing:\n");
	PB_(test)();
	fprintf(stderr, "Done tests of <" QUOTE(TREE_NAME) ">trie.\n\n");
}

#undef QUOTE
#undef QUOTE_

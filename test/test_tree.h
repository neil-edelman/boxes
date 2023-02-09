#include <stdio.h>
#include <string.h>
#include "orcish.h"

#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** This makes the key-value in the same place; will have to copy. */
struct PB_(tree_test) {
	int in;
	PB_(key) key;
#ifdef TREE_VALUE
	PB_(value) value;
#endif
};

#ifdef TREE_VALUE
typedef void (*PB_(action_fn))(PB_(key) *, PB_(value) *);
#else
typedef void (*PB_(action_fn))(PB_(key) *);
#endif

/** Recursively draws `outer` in `fp` with the actual `height`. */
static void PB_(subgraph)(const struct PB_(tree) *const sub, FILE *fp) {
	const struct PB_(branch) *branch;
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
		B_(to_string)(sub->node->key[i], sub->node->value + i, &z);
#else
		B_(to_string)(sub->node->key[i], &z);
#endif
		fprintf(fp, "\t<tr><td border=\"0\" align=\"left\""
			" port=\"%u\"%s>%s</td></tr>\n", i + 1, bgc, z);
	}
	fprintf(fp, "\t<hr/>\n"
		"\t<tr><td></td></tr>\n"
		"</table>>];\n");
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
		" fontname=modern, splines=false];\n"
		"\tnode [shape=none, fontname=modern];\n");
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
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td border=\"0\" colspan=\"%u\">"
		"<font color=\"Gray75\">%s</font></td></tr>\n"
		"\t<hr/>\n"
		"\t<tr>\n", (const void *)sub->node,
		sub->node->size ? sub->node->size : 1, orcify(sub->node));
	for(i = 0; i < sub->node->size; i++) {
		char z[12];
#ifdef TREE_VALUE
		B_(to_string)(sub->node->key[i], sub->node->value + i, &z);
#else
		B_(to_string)(sub->node->key[i], &z);
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
	branch = PB_(as_branch_c)(sub->node);
	for(i = 0; i <= branch->base.size; i++)
		fprintf(fp, "\ttrunk%p:%s -> trunk%p;\n",
		(const void *)sub->node, PB_(usual_port)(i),
		(const void *)branch->child[i]);
	/* Recurse. */
	for(i = 0; i <= branch->base.size; i++) {
		struct PB_(tree) child;
		child.node = branch->child[i], child.height = sub->height - 1;
		PB_(subgraph_usual)(&child, fp);
	}
}

/** Draw a graph of `tree` to `fn` in Graphviz format, the usual way, but too
 large for many labels. */
static void PB_(graph_horiz)(const struct B_(tree) *const tree,
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
	else PB_(subgraph_usual)(&tree->root, fp);
	fprintf(fp, "\tnode [color=\"Red\"];\n"
		"}\n");
	fclose(fp);
}

/** Makes sure the `tree` is in a valid state. */
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
static void PB_(sort)(struct PB_(tree_test) *a, const size_t size) {
	struct PB_(tree_test) temp;
	size_t i;
	for(i = 1; i < size; i++) {
		size_t j;
		for(j = i; j && B_(compare)(a[j - 1].key, a[i].key) > 0; j--);
		if(j == i) continue;
		temp = a[i];
		memmove(a + j + 1, a + j, sizeof *a * (i - j));
		a[j] = temp;
	}
}

static void PB_(test)(void) {
	struct B_(tree) tree = B_(tree)(), empty = B_(tree)();
	struct B_(tree_iterator) it;
	struct PB_(tree_test) test[800];
	const size_t test_size = sizeof test / sizeof *test;
#ifdef TREE_VALUE
	PB_(value) *v;
#endif
	PB_(key) k, k_prev;
	size_t i, n_unique = 0, n_unique2;
	char fn[64];
	int succ;

	errno = 0;

	/* Fill. */
	for(i = 0; i < test_size; i++) {
		struct PB_(tree_test) *const t = test + i;
		t->in = 0;
		/* This function must exist. */
#ifdef TREE_VALUE
		B_(filler)(&t->key, &t->value);
#else
		B_(filler)(&t->key);
#endif
	}
	PB_(sort)(test, test_size);
	for(i = 0; i < test_size; i++) {
		struct PB_(tree_test) *const t = test + i;
		char z[12];
#ifdef TREE_VALUE
		B_(to_string)(t->key, &t->value, &z);
#else
		B_(to_string)(t->key, &z);
#endif
		printf("%s\n", z);
	}

	/* Idle. */
	PB_(valid)(0);
	PB_(valid)(&tree);
	PB_(graph)(&tree, "graph/" QUOTE(TREE_NAME) "-idle.gv");
	B_(tree_)(&tree), PB_(valid)(&tree);
	/* Not valid anymore.
	it = B_(tree_less)(0, test[0].key), assert(!it._.root); */
	it = B_(tree_less)(&tree, test[0].key), assert(it._.root && !it._.ref.node);

	/* Bulk, (simple.) */
	for(i = 0; i < test_size; i++) {
		/*PB_(entry_c) e;
		char z[12];*/
		struct PB_(tree_test) *const t = test + i;
		/*e = PB_(test_to_entry_c)(t);
		PB_(to_string)(e, &z);
		printf("%lu -- bulk adding <%s>.\n", (unsigned long)i, z);*/
		switch(B_(tree_bulk_add)(&tree, t->key
#ifdef TREE_VALUE
			, &v
#endif
			)){
		case TREE_ERROR: perror("What?"); assert(0); break;
		case TREE_PRESENT:
			/*assert(B_(tree_get)(&tree, PB_(test_to_key)(t)));*/
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
		value = B_(tree_get)(&tree, PB_(test_to_key)(t));
		assert(value);
#else
		{
			PB_(key) *pk = B_(tree_get)(&tree, PB_(test_to_key)(t));
			assert(pk && !PB_(compare)(*pk, *t));
		}
#endif
#endif



		if(!(i & (i + 1)) || i == test_size - 1) {
			sprintf(fn, "graph/" QUOTE(TREE_NAME) "-bulk-%lu.gv", i + 1);
			PB_(graph)(&tree, fn);
		}
	}
	printf("Finalize.\n");
	B_(tree_bulk_finish)(&tree);
	printf("Finalize again. This should be idempotent.\n");
	B_(tree_bulk_finish)(&tree);
	PB_(graph)(&tree, "graph/" QUOTE(TREE_NAME) "-bulk-finish.gv");
	printf("Tree: %s.\n", B_(tree_to_string)(&tree));

	/* Iteration; checksum. */
	memset(&k_prev, 0, sizeof k_prev);
	it = B_(tree_iterator)(&tree), i = 0;
	while(B_(tree_next)(&it)) {
		/*char z[12];*/
		k = B_(tree_key)(&it);
/*#ifdef TREE_VALUE
		v = B_(tree_value)(&it);
		B_(to_string)(k, v, &z);
#else
		B_(to_string)(k, &z);
#endif
		printf("<%s>\n", z);*/
		if(i) { const int cmp = B_(compare)(k, k_prev); assert(cmp > 0); }
		k_prev = k;
		if(++i > test_size) assert(0); /* Avoids loops. */
	}
	assert(i == n_unique);
	/*printf("\n");*/

	/* Going the other way. */
	it = B_(tree_iterator)(&tree), i = 0;
	while(B_(tree_previous)(&it)) {
		/*char z[12];*/
		k = B_(tree_key)(&it);
/*#ifdef TREE_VALUE
		v = B_(tree_value)(&it);
		B_(to_string)(k, v, &z);
#else
		B_(to_string)(k, &z);
#endif
		printf("<%s>\n", z);*/
		if(i) { const int cmp = B_(compare)(k_prev, k); assert(cmp > 0); }
		k_prev = k;
		if(++i > test_size) assert(0); /* Avoids loops. */
	}
	assert(i == n_unique);

	/* Deleting while iterating. */
	it = B_(tree_iterator)(&tree);
	succ = B_(tree_previous)(&it);
	assert(succ);
	do {
		/*char z[12];*/
		PB_(key) key = B_(tree_key)(&it);
/*#ifdef TREE_VALUE
		B_(to_string)(key, B_(tree_value)(&it), &z);
#else
		B_(to_string)(key, &z);
#endif
		printf("removing <%s>\n", z);*/
		succ = B_(tree_remove)(&tree, key);
		assert(succ);
		succ = B_(tree_remove)(&tree, key);
		assert(!succ);
		it = B_(tree_less)(&tree, key);
	} while(B_(tree_has_element)(&it));
	printf("Individual delete tree: %s.\n", B_(tree_to_string)(&tree));
	assert(tree.root.height == UINT_MAX);

	/* Clear. */
	B_(tree_clear)(0);
	B_(tree_clear)(&empty), assert(!empty.root.node);
	B_(tree_clear)(&tree), assert(tree.root.node
		&& tree.root.height == UINT_MAX);
	n_unique = 0;

	/* Fill again, this time, don't sort. */
	for(i = 0; i < test_size; i++) {
		struct PB_(tree_test) *const t = test + i;
		t->in = 0;
		/* This function must exist. */
#ifdef TREE_VALUE
		B_(filler)(&t->key, &t->value);
#else
		B_(filler)(&t->key);
#endif
	}

	/* Add. */
	for(i = 0; i < test_size; i++) {
		char z[12];
		struct PB_(tree_test) *const t = test + i;
#ifdef TREE_VALUE
		B_(to_string)(t->key, &t->value, &z);
#else
		B_(to_string)(t->key, &z);
#endif
		/*printf("%lu -- adding <%s>.\n", (unsigned long)i, z);*/
#ifdef TREE_VALUE
		switch(B_(tree_try)(&tree, t->key, &v))
#else
		switch(B_(tree_try)(&tree, t->key))
#endif
		{
		case TREE_ERROR: perror("unexpected"); assert(0); return;
		case TREE_PRESENT: /*printf("<%s> already in tree\n", z);*/ break;
		case TREE_ABSENT:
			n_unique++;
#ifdef TREE_VALUE
			*v = t->value;
#endif
			t->in = 1;
			printf("<%s> added\n", z); break;
		}
		if(!(i & (i + 1)) || i == test_size - 1) {
			sprintf(fn, "graph/" QUOTE(TREE_NAME) "-add-%lu.gv", i + 1);
			PB_(graph)(&tree, fn);
		}
	}
	printf("Number of entries in the tree: %lu/%lu.\n",
		(unsigned long)n_unique, (unsigned long)test_size);

	/* Delete all. Removal invalidates iterator. */
	it = B_(tree_iterator)(&tree), i = 0;
	B_(tree_next)(&it);
	assert(B_(tree_has_element)(&it));
	do {
		/*char z[12];*/
		k = B_(tree_key)(&it); /*
#ifdef TREE_VALUE
		v = B_(tree_value)(&it);
		B_(to_string)(k, v, &z);
#else
		B_(to_string)(k, &z);
#endif
		printf("Targeting <%s> for removal.\n", z);*/
		if(i) { const int cmp = B_(compare)(k, k_prev); assert(cmp > 0); }
		k_prev = k;
		if(++i > test_size) assert(0); /* Avoids loops. */
		assert(B_(tree_contains)(&tree, k));
		B_(tree_remove)(&tree, k);
		assert(!B_(tree_contains)(&tree, k));
		it = B_(tree_more)(&tree, k);
		/*printf("Iterator now %s:h%u:i%u.\n",
			orcify(it._.ref.node), it._.ref.height, it._.ref.idx);*/
		if(!(i & (i + 1)) || i == test_size - 1) {
			sprintf(fn, "graph/" QUOTE(TREE_NAME) "-rm-%lu.gv", i);
			PB_(graph)(&tree, fn);
		}
	} while(B_(tree_has_element)(&it));
	assert(i == n_unique);

	/* Add all back. */
	n_unique2 = 0;
	for(i = 0; i < test_size; i++) {
		struct PB_(tree_test) *const t = test + i;
		/*char z[12];*/
		if(!t->in) continue;
/*#ifdef TREE_VALUE
		B_(to_string)(t->key, &t->value, &z);
#else
		B_(to_string)(t->key, &z);
#endif
		printf("Adding %s.\n", z);*/
#ifdef TREE_VALUE
		switch(B_(tree_try)(&tree, t->key, &v))
#else
		switch(B_(tree_try)(&tree, t->key))
#endif
		{
		case TREE_ERROR:
		case TREE_PRESENT: perror("unexpected"); assert(0); return;
		case TREE_ABSENT: n_unique2++; break;
		}
#ifdef TREE_VALUE
		*v = t->value;
#endif
	}
	printf("Re-add tree: %lu\n", (unsigned long)n_unique2);
	PB_(graph)(&tree, "graph/" QUOTE(TREE_NAME) "-re-add.gv");
	assert(n_unique == n_unique2);
	i = B_(tree_count)(&tree);
	printf("tree count: %lu; add count: %lu\n",
		(unsigned long)i, (unsigned long)n_unique2);
	assert(i == n_unique);

	/* Remove every 2nd. */
	it = B_(tree_iterator)(&tree);
	while(B_(tree_next)(&it)) {
		PB_(key) key = B_(tree_key)(&it);
		const int ret = B_(tree_remove)(&tree, key);
		assert(ret);
		n_unique--;
		it = B_(tree_more)(&tree, key); /* Move past the erased keys. */
		if(!B_(tree_has_element)(&it)) break;
	}
	i = B_(tree_count)(&tree);
	printf("remove every 2nd: %lu\n", (unsigned long)i);
	assert(i == n_unique);

	printf("clear, destroy\n");
	B_(tree_clear)(&tree);
	assert(tree.root.height == UINT_MAX && tree.root.node);

	/* Destroy. */
	B_(tree_)(&tree), assert(!tree.root.node), PB_(valid)(&tree);
	assert(!errno);
}

/** Will be tested on stdout. Requires `TREE_TEST`, and not `NDEBUG` while
 defining `assert`. @allow */
static void B_(tree_test)(void) {
	printf("<" QUOTE(TREE_NAME) ">tree"
		" of key <" QUOTE(TREE_KEY) ">;"
#ifdef TREE_VALUE
		" value <" QUOTE(TREE_VALUE) ">;"
#endif
		" testing:\n");
	PB_(test)();
	fprintf(stderr, "Done tests of <" QUOTE(TREE_NAME) ">tree.\n\n");
	(void)PB_(graph_horiz); /* Not used in general. */
}

#undef QUOTE
#undef QUOTE_

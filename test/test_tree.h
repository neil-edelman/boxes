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
	PB_(key) x;
	PB_(value) value;
} PB_(entry_test);
static PB_(key) PB_(test_to_x)(struct B_(tree_test) *const t) { return t->x; }
#else
typedef PB_(key) PB_(entry_test);
static PB_(key) PB_(test_to_x)(PB_(key) *const x) { return *x; }
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
static void PB_(subgraph)(const struct B_(tree) *const sub, FILE *fp) {
	const struct PB_(branch) *branch;
	unsigned i;
	assert(sub->root && fp);
	/* It still has a margin, augh. */
	fprintf(fp, "\ttrunk%p [label = <\n"
		"<table border=\"1\" cellspacing=\"0\" bgcolor=\"Grey95\">\n"
		"\t<tr><td border=\"0\" port=\"0\">"
		"<font color=\"Gray75\">%s</font></td></tr>\n",
		(const void *)sub->root, orcify(sub->root));
	for(i = 0; i < sub->root->size; i++) {
		const char *const bgc = i & 1 ? "" : " bgcolor=\"Gray90\"";
		char z[12];
		PB_(entry_c) e = PB_(to_entry_c)(sub->root, i);
		PB_(to_string)(e, &z);
		fprintf(fp, "\t<tr><td border=\"0\" align=\"left\""
			" port=\"%u\"%s>%s</td></tr>\n", i + 1, bgc, z);
	}
	fprintf(fp, "</table>>];\n");
	if(!sub->height) return;
	/* Draw the lines between trees. */
	branch = PB_(branch_c)(sub->root);
	for(i = 0; i <= sub->root->size; i++)
		fprintf(fp, "\ttrunk%p:%u:se -> trunk%p;\n",
		(const void *)sub->root, i, (const void *)branch->child[i]);
	/* Recurse. */
	for(i = 0; i <= sub->root->size; i++) {
		struct B_(tree) subsub;
		subsub.root = branch->child[i], subsub.height = sub->height - 1;
		PB_(subgraph)(&subsub, fp);
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
		" fontname=\"Bitstream Vera Sans\"];\n"
		"\tnode [shape=none, fontname=\"Bitstream Vera Sans\"];\n"
		"\tedge [fontname=\"Bitstream Vera Sans\", style=dashed];\n"
		"\n");
	if(!tree->root)
		fprintf(fp, "\tidle [shape=plaintext];\n");
	else if(tree->height == UINT_MAX)
		fprintf(fp, "\tempty [shape=plaintext];\n");
	else PB_(subgraph)(tree, fp);
	fprintf(fp, "\tnode [color=\"Red\"];\n"
		"}\n");
	fclose(fp);
}

static void PB_(print_r)(const struct B_(tree) tree) {
	struct B_(tree) sub = { 0, 0 };
	const struct PB_(branch) *inner = 0;
	unsigned i;
	assert(tree.root);
	printf("\\");
	if(tree.height) {
		inner = PB_(branch_c)(tree.root);
		sub.height = tree.height - 1;
	}
	for(i = 0; ; i++) {
		char z[12];
		PB_(entry_c) e;
		if(tree.height) sub.root = inner->child[i], PB_(print_r)(sub);
		if(i == tree.root->size) break;
		e = PB_(to_entry_c)(tree.root, i);
		PB_(to_string)(e, &z);
		printf("%s%s", i ? ", " : "", z);
	}
	printf("/");
}
static void PB_(print)(const struct B_(tree) *const tree) {
	printf("Inorder: ");
	if(!tree) {
		printf("null");
	} else if(!tree->root) {
		assert(!tree->height);
		printf("idle");
	} else if(tree->height == UINT_MAX) {
		printf("empty");
	} else {
		PB_(print_r)(*tree);
	}
	printf("\n");
}

/** Makes sure the `trie` is in a valid state. */
static void PB_(valid)(const struct B_(tree) *const tree) {
	if(!tree) return; /* Null. */
	if(!tree->root) { assert(!tree->height); return; } /* Idle. */
	if(tree->height == UINT_MAX) { assert(tree->root); return; } /* Empty. */
	assert(tree->root);
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
		for(j = i; j && PB_(compare)(PB_(test_to_x)(a + j - 1),
			PB_(test_to_x)(a + i)) > 0; j--);
		if(j == i) continue;
		temp = a[i];
		memmove(a + j + 1, a + j, sizeof *a * (i - j));
		a[j] = temp;
	}
}

static int PB_(contents)(const PB_(entry) *const e) {
#ifdef TREE_VALUE
	return !!e->x;
#else
	return !!*e;
#endif
}

static PB_(entry_c) PB_(to_const)(const PB_(entry) e) {
#ifdef TREE_VALUE
	PB_(entry_c) c = { e.x, e.value };
	return c;
#else
	return e;
#endif
}

static void PB_(test)(void) {
	struct B_(tree) tree = B_(tree)();
	struct B_(tree_iterator) it;
	PB_(entry_test) n[20];
	const size_t n_size = sizeof n / sizeof *n;
	PB_(entry) entry;
	PB_(value) *value;
	size_t i;
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
	it = B_(tree_lower)(0, PB_(test_to_x)(n + 0)), assert(!it._.tree);
	value = B_(tree_get)(0, PB_(test_to_x)(n + 0)), assert(!value);
	it = B_(tree_lower)(&tree, PB_(test_to_x)(n + 0)), assert(!it._.tree);
	value = B_(tree_get)(&tree, PB_(test_to_x)(n + 0)), assert(!value);

	/* Test. */
	for(i = 0; i < n_size; i++) {
		PB_(entry_test) *const e = n + i;
		value = B_(tree_bulk_add)(&tree, PB_(test_to_x)(e));
		assert(value);
#ifdef TREE_VALUE
		*value = e->value;
#endif
		sprintf(fn, "graph/" QUOTE(TREE_NAME) "-%u.gv", ++PB_(no));
		PB_(graph)(&tree, fn);
	}
	B_(tree_bulk_finish)(&tree);
	printf("Finalize again.\n");
	B_(tree_bulk_finish)(&tree); /* This should be idempotent. */
	sprintf(fn, "graph/" QUOTE(TREE_NAME) "-%u-finalized.gv", ++PB_(no));
	PB_(graph)(&tree, fn);

	//printf("Tree: %s.\n", PB_(tree_to_string)(&tree));
	/* Iteration. */
	it = B_(tree_begin)(&tree), i = 0;
	while(entry = B_(tree_next)(&it), PB_(contents)(&entry)) {
		char z[12];
		i++;
		PB_(to_string)(PB_(to_const)(entry), &z);
		printf("<%s>\n", z);
		if(i > 100) assert(0);
	}
	assert(i == n_size);

	B_(tree_)(&tree), assert(!tree.root), PB_(valid)(&tree);
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

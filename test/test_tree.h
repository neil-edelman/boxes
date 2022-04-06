#include <stdio.h>
#include <string.h>
#include "orcish.h"

#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** Works by side-effects, _ie_ fills the type with data. Only defined if
 `TREE_TEST`. */
typedef void (*PB_(action_fn))(PB_(entry) *);

/* `TREE_TEST` must be a function that implements <typedef:<PT>action_fn>. */
static const PB_(action_fn) PB_(filler) = (TREE_TEST);

/* Debug number, which is the number printed next to the graphs, _etc_. */
static unsigned PB_(no);

/** Recursively draws `outer` in `fp` with the actual `height`. */
static void PB_(subgraph)(const struct B_(tree) tree, FILE *fp) {
	const struct PB_(inner) *inner;
	unsigned i;
	assert(tree.node && fp);
	fprintf(fp, "\ttrunk%p [shape = box, "
		"style = filled, fillcolor = Gray95, label = <\n"
		"<TABLE BORDER=\"0\">\n"
		"\t<TR><TD ALIGN=\"LEFT\">"
		"<FONT COLOR=\"Gray85\">%s</FONT></TD></TR>\n"
		"\t<TR><TD ALIGN=\"LEFT\" PORT=\"0\"><FONT FACE=\"Times-Italic\">"
		"height %u</FONT></TD></TR>\n",
		(const void *)tree.node, orcify(tree.node), tree.height);
	for(i = 0; i < tree.node->size; i++) {
		const char *const bgc = i & 1 ? "" : " BGCOLOR=\"Gray90\"";
		char z[12];
		PB_(to_string)(tree.node->x + i, &z);
		fprintf(fp, "\t<TR><TD ALIGN=\"LEFT\" PORT=\"%u\"%s>%s</TD></TR>\n",
			i + 1, bgc, z);
	}
	fprintf(fp, "</TABLE>>];\n");
	if(!tree.height) return;
	/* Draw the lines between trees. */
	inner = PB_(inner_c)(tree.node);
	for(i = 0; i <= tree.node->size; i++)
		fprintf(fp, "\ttrunk%p:%u:se -> trunk%p;\n",
		(const void *)tree.node, i, (const void *)inner->link[i]);
	/* Recurse. */
	for(i = 0; i <= tree.node->size; i++) {
		const struct B_(tree) sub = { inner->link[i], tree.height - 1 };
		PB_(subgraph)(sub, fp);
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
		"\tnode [shape=record, style=filled, fillcolor=\"Grey95\","
		" fontname=\"Bitstream Vera Sans\"];\n"
		"\tedge [fontname=\"Bitstream Vera Sans\", style=dashed];\n"
		"\n");
	if(!tree->node) fprintf(fp, "\tidle;\n");
	else if(tree->height == UINT_MAX) fprintf(fp, "\tempty;\n");
	else PB_(subgraph)(*tree, fp);
	fprintf(fp, "\tnode [color = \"Red\"];\n"
		"}\n");
	fclose(fp);
}

static void PB_(print_r)(const struct B_(tree) tree) {
	struct B_(tree) sub = { 0, 0 };
	const struct PB_(inner) *inner = 0;
	unsigned i;
	assert(tree.node);
	printf("\\");
	if(tree.height) {
		inner = PB_(inner_c)(tree.node);
		sub.height = tree.height - 1;
	}
	for(i = 0; ; i++) {
		char z[12];
		const PB_(entry) *e;
		if(tree.height) sub.node = inner->link[i], PB_(print_r)(sub);
		if(i == tree.node->size) break;
		e = tree.node->x + i;
		PB_(to_string)(e, &z);
		printf("%s%s", i ? ", " : "", z);
	}
	printf("/");
}
static void PB_(print)(const struct B_(tree) *const tree) {
	unsigned h;
	printf("Inorder: ");
	if(!tree) {
		printf("null");
	} else if(!tree->node) {
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
	if(!tree || !tree->height_p1) return;
	assert(tree->node);
	/*...*/
}

static void PB_(sort)(PB_(entry) *a, const size_t size) {
	PB_(entry) temp;
	size_t i;
	for(i = 1; i < size; i++) {
		size_t j;
		for(j = i; j; j--) if(!(PB_(compare)(PB_(to_x)(a + j - 1),
			PB_(to_x)(a + i)) > 0)) break;
		if(j == i) continue;
		/* memcpy(&temp, a + i, sizeof *a);
		memcpy(a + j, &temp, sizeof *a); */
		temp = a[i];
		memmove(a + j + 1, a + j, sizeof *a * (i - j));
		a[j] = temp;
	}
}

static void PB_(test)(void) {
	char z[12];
	struct B_(tree) tree = TREE_IDLE;
	struct B_(tree_iterator) it;
	PB_(entry) n[20];
	const size_t n_size = sizeof n / sizeof *n;
	PB_(value) *value;
	size_t i;

	errno = 0;

	/* Fill. */
	for(i = 0; i < n_size; i++) PB_(filler)(n + i);
	PB_(sort)(n, n_size);
	for(i = 0; i < n_size; i++)
		PB_(to_string)(n + i, &z), printf("%s\n", z);

	/* Idle. */
	PB_(valid)(0);
	PB_(valid)(&tree);
	B_(tree)(&tree), PB_(valid)(&tree);
	PB_(graph)(&tree, "graph/" QUOTE(TREE_NAME) "-idle.gv");
	B_(tree_)(&tree), PB_(valid)(&tree);
	it = B_(tree_lower)(0, PB_(to_x)(n + 0)), assert(!it.i.tree);
	value = B_(tree_get)(0, PB_(to_x)(n + 0)), assert(!value);
	it = B_(tree_lower)(&tree, PB_(to_x)(n + 0)), assert(!it.i.tree);
	value = B_(tree_get)(&tree, PB_(to_x)(n + 0)), assert(!value);

	/* Test. */
	for(i = 0; i < n_size; i++) {
		char fn[64];
		PB_(to_string)(n + i, &z);
		printf("Adding %s.\n", z);
		value = B_(bulk_add)(&tree, PB_(to_x)(n + i));
		assert(value);
#ifdef TREE_VALUE
		assert(0); /* Copy. */
#endif
		sprintf(fn, "graph/" QUOTE(TREE_NAME) "-%u.gv", ++PB_(no));
		PB_(graph)(&tree, fn);
	}
	B_(tree_)(&tree), assert(!tree.node), PB_(valid)(&tree);
	assert(!errno);
}

/** Will be tested on stdout. Requires `TRIE_TEST`, and not `NDEBUG` while
 defining `assert`. @allow */
static void B_(tree_test)(void) {
	printf("<" QUOTE(TREE_NAME) ">trie"
		" of type <" QUOTE(TREE_TYPE) ">"
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

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

static void PB_(subgraph)(const struct PB_(outer) *const outer,
	unsigned height, FILE *fp) {
	unsigned i;
	assert(outer && height && fp);
	fprintf(fp, "\ttrunk%p [shape = box, "
		"style = filled, fillcolor = Gray95, label = <\n"
		"<TABLE BORDER=\"0\">\n"
		"\t<TR><TD COLSPAN=\"3\" ALIGN=\"LEFT\">"
		"<FONT COLOR=\"Gray85\">%s</FONT></TD></TR>\n"
		"\t<TR><TD ALIGN=\"LEFT\"><FONT FACE=\"Times-Italic\">"
		"height %u</FONT></TD></TR>\n",
		(const void *)outer, orcify(outer), height);
	for(i = 0; i < outer->size; i++) {
		const char *const bgc = i & 1 ? "" : " BGCOLOR=\"Gray90\"";
		char z[12];
		PB_(to_string)(outer->x + i, &z);
		fprintf(fp, "\t<TR><TD ALIGN=\"LEFT\" PORT=\"%u\"%s>%s</FONT>"
			"</TD></TR>\n", i, bgc, z);
	}
	/* . . . */
#if 0
	/* Draw the lines between trees. */
	if(!height) return;
	inner = trie_inner_c(tr);
	for(i = 0; i <= tr->bsize; i++)
		fprintf(fp, "\ttree%pbranch0:%u -> tree%pbranch0 "
		"[style = dashed, arrowhead = %snormal];\n", (const void *)inner, i,
		(const void *)inner->leaf[i].link, PB_(leaf_to_dir)(tr, i));
	/* Recurse. */
	for(i = 0; i <= tr->bsize; i++) {
		struct { unsigned br0, br1, lf; } in_tree;
		size_t bit = treebit;
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
		PB_(graph_tree_mem)(inner->leaf[i].link, height, bit, fp);
	}
#endif
}

/** Draw a graph of `tree` to `fn` in Graphviz format. */
static void PB_(graph)(const struct B_(tree) *const tree,
	const char *const fn) {
	FILE *fp;
	unsigned h;
	assert(tree && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\tgraph [truecolor=true, bgcolor=transparent];\n"
		"\tfontface=modern;\n"
		"\tnode [shape=none];\n"
		"\n");
	if(!tree->root) fprintf(fp, "\tidle;\n");
	else if(!(h = tree->height)) fprintf(fp, "\tempty;\n");
	else PB_(subgraph)(tree->root, tree->height, fp);
	fprintf(fp, "</TABLE>>];\n");
	fprintf(fp, "\tnode [color = \"Red\"];\n"
		"}\n");
	fclose(fp);
}

/** Makes sure the `trie` is in a valid state. */
static void PB_(valid)(const struct B_(tree) *const tree) {
	if(!tree || !tree->height) return;
	assert(tree->root);
	/*...*/
}

static void PB_(test)(void) {
	struct B_(tree) tree = TREE_IDLE;
	struct B_(tree_iterator) it;
	PB_(entry) n[2];
	const size_t n_size = sizeof n / sizeof *n;
	PB_(type) x;
	PB_(value) *value;
	size_t i;

	/* Idle. */
	PB_(valid)(0);
	PB_(valid)(&tree);
	B_(tree)(&tree), PB_(valid)(&tree);
	PB_(graph)(&tree, "graph/" QUOTE(TRIE_NAME) "-idle.gv");
	B_(tree_)(&tree), PB_(valid)(&tree);
	for(i = 0; i < n_size; i++) PB_(filler)(n + i);
	it = B_(tree_lower)(&tree, PB_(to_x)(n + 0)), assert(!it.i.tree);
	//x = B_(trie_get)(&trie, PB_(to_x)(n + 0)), assert(!data);
	B_(tree_)(&tree), assert(!tree.root), PB_(valid)(&tree);
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

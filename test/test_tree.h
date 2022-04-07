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

/** `TREE_TEST` must be a function that implements <typedef:<PT>action_fn>.
 Requires an added temporary buffer for the value if defined; _ie_, creates
 the new entry, and if successful, copies the value into it. This is a little
 unintuitive, but the simplest we could think of. */
static const PB_(action_fn) PB_(filler) = (TREE_TEST);

/* Debug number, which is the number printed next to the graphs, _etc_. */
static unsigned PB_(no);

/** Recursively draws `outer` in `fp` with the actual `height`. */
static void PB_(subgraph)(const struct B_(tree) sub, FILE *fp) {
	const struct PB_(inner) *inner;
	unsigned i;
	assert(sub.node && fp);
	fprintf(fp, "\ttrunk%p [shape = box, "
		"style = filled, fillcolor = Gray95, label = <\n"
		"<TABLE BORDER=\"0\">\n"
		"\t<TR><TD ALIGN=\"LEFT\">"
		"<FONT COLOR=\"Gray85\">%s</FONT></TD></TR>\n"
		"\t<TR><TD ALIGN=\"LEFT\" PORT=\"0\"><FONT FACE=\"Times-Italic\">"
		"height %u</FONT></TD></TR>\n",
		(const void *)sub.node, orcify(sub.node), sub.height);
	for(i = 0; i < sub.node->size; i++) {
		const char *const bgc = i & 1 ? "" : " BGCOLOR=\"Gray90\"";
		char z[12];
		PB_(entry) e;
		PB_(fill_entry)(&e, sub.node, i), PB_(to_string)(&e, &z);
		fprintf(fp, "\t<TR><TD ALIGN=\"LEFT\" PORT=\"%u\"%s>%s</TD></TR>\n",
			i + 1, bgc, z);
	}
	fprintf(fp, "</TABLE>>];\n");
	if(!sub.height) return;
	/* Draw the lines between trees. */
	inner = PB_(inner_c)(sub.node);
	for(i = 0; i <= sub.node->size; i++)
		fprintf(fp, "\ttrunk%p:%u:se -> trunk%p;\n",
		(const void *)sub.node, i, (const void *)inner->link[i]);
	/* Recurse. */
	for(i = 0; i <= sub.node->size; i++) {
		struct B_(tree) subsub;
		subsub.node = inner->link[i], subsub.height = sub.height - 1;
		PB_(subgraph)(subsub, fp);
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
	if(!tree->node)
		fprintf(fp, "\tidle [shape=plaintext, fillcolor=\"none\"];\n");
	else if(tree->height == UINT_MAX)
		fprintf(fp, "\tempty [shape=plaintext, fillcolor=\"none\"];\n");
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
		PB_(entry) e;
		if(tree.height) sub.node = inner->link[i], PB_(print_r)(sub);
		if(i == tree.node->size) break;
#ifdef TREE_VALUE
		e.x = tree.node->x[i], e.value = tree.node->value + i;
#else
		e = tree.node->x[i];
#endif
		PB_(to_string)(&e, &z);
		printf("%s%s", i ? ", " : "", z);
	}
	printf("/");
}
static void PB_(print)(const struct B_(tree) *const tree) {
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
	if(!tree) return; /* Null. */
	if(!tree->node) { assert(!tree->height); return; } /* Idle. */
	if(tree->height == UINT_MAX) { assert(tree->node); return; } /* Empty. */
	assert(tree->node);
	/*...*/
}

/** Ca'n't use `qsort` with `size` because we don't have a comparison;
 <data:<PB>compare> only has to separate it into two, not three. (One can use
 `qsort` compare in this compare, but generally not the other way around.) */
static void PB_(sort)(PB_(entry) *a, const size_t size) {
	PB_(entry) temp;
	size_t i;
	for(i = 1; i < size; i++) {
		size_t j;
		for(j = i; j; j--) {
			char n[12], m[12];
			PB_(to_string)(a + j - 1, &n);
			PB_(to_string)(a + i, &m);
			/*printf("cmp %s and %s\n", n, m);*/
			if(!(PB_(compare)(PB_(to_x)(a + j - 1),
			PB_(to_x)(a + i)) > 0)) break;
		}
		if(j == i) continue;
		temp = a[i]; /*memcpy(&temp, a + i, sizeof *a);*/
		memmove(a + j + 1, a + j, sizeof *a * (i - j));
		a[j] = temp; /*memcpy(a + j, &temp, sizeof *a);*/
	}
}

static void PB_(test)(void) {
	char z[12];
	struct B_(tree) tree = B_(tree)();
	struct B_(tree_iterator) it;
	PB_(entry) n[3];
#ifdef TREE_VALUE
	/* Values go here for when they are needed, then they are copied. */
	PB_(value) copy_values[sizeof n / sizeof *n];
#endif
	const size_t n_size = sizeof n / sizeof *n;
	PB_(value) *value;
	size_t i;

	errno = 0;

	/* Fill. */
	for(i = 0; i < n_size; i++) {
#ifdef TREE_VALUE
		n[i].value = copy_values + i; /* Must have value a valid pointer. */
#endif
		PB_(filler)(n + i);
	}
	PB_(sort)(n, n_size);
	/*for(i = 0; i < n_size; i++)
		PB_(to_string)(n + i, &z), printf("%s\n", z);*/

	/* Idle. */
	PB_(valid)(0);
	PB_(valid)(&tree);
	PB_(graph)(&tree, "graph/" QUOTE(TREE_NAME) "-idle.gv");
	B_(tree_)(&tree), PB_(valid)(&tree);
	it = B_(tree_lower)(0, PB_(to_x)(n + 0)), assert(!it.it.tree);
	value = B_(tree_get)(0, PB_(to_x)(n + 0)), assert(!value);
	it = B_(tree_lower)(&tree, PB_(to_x)(n + 0)), assert(!it.it.tree);
	value = B_(tree_get)(&tree, PB_(to_x)(n + 0)), assert(!value);

	/* Test. */
	for(i = 0; i < n_size; i++) {
		PB_(entry) *const e = n + i;
		char fn[64];
		PB_(to_string)(e, &z);
		printf("Adding %s.\n", z);
		value = B_(tree_bulk_add)(&tree, PB_(to_x)(e));
		assert(value);
#ifdef TREE_VALUE
		memcpy(value, &e->value, sizeof e->value); /* Untested. */
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

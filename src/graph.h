#if defined BOX_ENTRY1 && !defined BOX_ENTRY2 /* Only make graphs for top. */

#	if defined(QUOTE) || defined(QUOTE_)
#		error QUOTE_? cannot be defined.
#	endif
#	define QUOTE_(name) #name
#	define QUOTE(name) QUOTE_(name)

#	define BOX_ALL /* Sanity check. */
#	include "box.h"

#	include <stdio.h>

#	ifdef BOX_NON_STATIC
#		define static
void T_(graph)(const pT_(box) *, FILE *);
void T_(graph_fn)(const pT_(box) *, const char *);
#	endif
#	ifndef BOX_DECLARE_ONLY

#		ifdef ARRAY_NAME

/** Draw a graph of `a` to `fn` in Graphviz format. */
static void T_(graph)(const struct t_(array) *const a, FILE *const fp) {
	size_t i;
	char z[12];
	fprintf(fp, "digraph {\n"
		"\tgraph [rankdir=LR, truecolor=true, bgcolor=transparent,"
		" fontname=modern];\n"
		"\tnode [shape=none, fontname=modern];\n"
		"\tarray [label=<\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td colspan=\"3\" align=\"left\">"
		"<font color=\"Gray75\">&lt;" QUOTE(ARRAY_NAME)
		"&gt;array: " QUOTE(ARRAY_TYPE) "</font></td></tr>\n"
		"\t<hr/>\n"
		"\t<tr>\n"
		"\t\t<td border=\"0\" align=\"right\">size</td>\n"
		"\t\t<td border=\"0\">%lu</td>\n"
		"\t\t<td border=\"0\" align=\"right\">%lu</td>\n"
		"\t</tr>\n"
		"\t<hr/>\n"
		"\t<tr><td></td></tr>\n"
		"</table>>];\n", (unsigned long)a->size, (unsigned long)a->capacity);
	if(!a->data) goto no_data;
	fprintf(fp, "\tarray -> data;\n"
		"\tdata [label=<\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td colspan=\"2\" align=\"left\">"
		"<font color=\"Gray75\">%s</font></td></tr>\n"
		"\t<hr/>\n", orcify(a->data));
	for(i = 0; i < a->size; i++) {
		const char *const bgc = i & 1 ? " bgcolor=\"Gray95\"" : "";
		t_(to_string)((void *)(a->data + i), &z);
		fprintf(fp, "\t<tr>\n"
			"\t\t<td align=\"right\"%s>"
			"<font face=\"Times-Italic\">%lu</font></td>\n"
			"\t\t<td align=\"left\"%s>%s</td>\n"
			"\t</tr>\n", bgc, (unsigned long)i, bgc, z);
	}
	fprintf(fp, "\t<hr/>\n"
		"\t<tr><td></td></tr>\n"
		"</table>>];\n");
no_data:
	fprintf(fp, "\tnode [colour=\"Red\"];\n"
		"}\n");
}

#		elif defined HEAP_NAME

/** Draw a graph of `heap` to `fn` in Graphviz format. */
static void T_(graph)(const struct t_(heap) *const heap, FILE *const fp) {
	char a[12];
	size_t i;
	assert(heap && fp);
	fprintf(fp, "digraph {\n"
		"\tgraph [truecolor=true, bgcolor=transparent, fontname=modern];\n"
		"\tnode [shape=box, style=filled, fillcolor=\"Gray95\","
		" fontname=modern];\n"
		/* Google search / Wikipedia says we should draw them with the top down.
		"\trankdir = BT;\n" */
		"\tedge [arrowhead = none];\n");
	for(i = 0; i < heap->as_array.size; i++) {
		const pT_(priority) *const p = heap->as_array.data + i;
		t_(to_string)(p, &a);
		fprintf(fp, "\t\tn%lu [label=\"%s\"];\n", (unsigned long)i, a);
		if(!i) continue;
		fprintf(fp, "\t\tn%lu -> n%lu;\n",
			(unsigned long)((i - 1) / 2), (unsigned long)i);
	}
	fprintf(fp, "\tnode [colour=red];\n"
		"}\n");
	fclose(fp);
}

#		else
#			error Not something we know how to make a graph out of?
#		endif

static void T_(graph_fn)(const pT_(box) *const box, const char *const fn) {
	FILE *fp;
	assert(box && fn && !errno);
	if(!(fp = fopen(fn, "w"))) { perror(fn); errno = 0; return; }
	T_(graph)(box, fp);
	fclose(fp);
}
#	endif
#	undef QUOTE
#	undef QUOTE_
#endif

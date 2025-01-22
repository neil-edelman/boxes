#if defined BOX_ENTRY1 && !defined BOX_ENTRY2 /* Only make graphs for top. */

#	define BOX_ALL /* Sanity check. */
#	include "box.h"

#	include <stdio.h>
#	include <errno.h>

#	ifdef BOX_NON_STATIC
void T_(graph)(const pT_(box) *, FILE *);
int T_(graph_fn)(const pT_(box) *, const char *);
#	endif
#	ifndef BOX_DECLARE_ONLY

/* Must link the file produced by compiling `orcish.c` to do tests or get
 graphs. (It is particularly useful for debugging—it translates gobbledygook
 pointer addresses into more semi-meaningful deterministic orc names. We have
 node "Trogdor" and "Gab-ukghash", instead of some numbers.) */
#		include "orcish.h"

#		if defined QUOTE || defined QUOTE_
#			error Cannot be defined.
#		endif
#		define QUOTE_(name) #name
#		define QUOTE(name) QUOTE_(name)

#		ifdef ARRAY_NAME

#			define BOX_PUBLIC_OVERRIDE
#			include "box.h"

/** Draw a graph of `array` to `fp` in Graphviz format. */
static void T_(graph)(const struct t_(array) *const array, FILE *const fp) {
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
		"</table>>];\n", (unsigned long)array->size, (unsigned long)array->capacity);
	if(!array->data) goto no_data;
	fprintf(fp, "\tarray -> data;\n"
		"\tdata [label=<\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td colspan=\"2\" align=\"left\">"
		"<font color=\"Gray75\">%s</font></td></tr>\n"
		"\t<hr/>\n", orcify(array->data));
	for(i = 0; i < array->size; i++) {
		const char *const bgc = i & 1 ? " bgcolor=\"Gray95\"" : "";
		t_(to_string)((void *)(array->data + i), &z);
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

#			define BOX_PUBLIC_OVERRIDE
#			include "box.h"

/** Draw a graph of `heap` to `fp` in Graphviz format. */
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
}

#		elif defined LIST_NAME

static const char *pT_(colour);
static size_t pT_(offset); /* The list's offset to the parent. */

/** Names `l`. `dir` is either 0, it names the node, or positive/negative to
 name edges. */
static char *pT_(name)(const struct t_(listlink) *const l) {
	static char z[8][64];
	static unsigned n;
	char *y = z[n];
	n = (n + 1) % (sizeof z / sizeof *z);
	assert(l);
	/* Normal or sentinel. */
	if(l->prev && l->next) {
		const void *node = (const void *)((const char *)l - pT_(offset));
		sprintf(y, "n%p", node);
	} else {
		sprintf(y, "list_%s:%s", pT_(colour), l->next ? "head" : "tail");
	}
	return y;
}

/** Print `list` to `fp`.
 @param[colour] A colour that can also have a 4 appended; _eg_ "royalblue",
 "firebrick", "darkseagreen", "orchid".
 @param[offset] For printing multiple lists, offset to the parent type.
 @param[is_nodes] Print nodes; if one is printing the same list, different
 order, then this would be off. */
static void pT_(subgraph)(const struct t_(list) *const list, FILE *const fp,
	const char *const colour, const size_t offset, const int is_nodes) {
	struct t_(listlink) *link;
	char a[12];
	assert(list && fp && colour);
	pT_(colour) = colour;
	pT_(offset) = offset;
	fprintf(fp, "\tlist_%s [label=<\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td align=\"left\" border=\"0\"><font color=\"Gray75\">&lt;"
		QUOTE(LIST_NAME) "&gt;list</font></td></tr>\n"
		"\t<hr/>\n"
		"\t<tr><td port=\"tail\" border=\"0\" align=\"left\">tail</td></tr>\n"
		"\t<tr><td port=\"head\" border=\"0\" align=\"left\""
		" bgcolor=\"Grey95\">head</td></tr>\n"
		"\t<hr/>\n"
		"\t<tr><td></td></tr>\n"
		"</table>>, style=none, shape=plain];\n", pT_(colour));
	assert(list->u.flat.next && !list->u.flat.zero && list->u.flat.prev);
	if(!list->u.flat.next->prev) { /* Empty: drawing has to make an exception. */
		assert(!list->u.flat.prev->next);
		fprintf(fp, "\tlist_%s:tail -> list_%s:head"
			" [color=\"%s4\", style=\"dotted\", arrowhead=\"empty\"];\n"
			"\tlist_%s:head -> list_%s:tail [color=\"%s\"];\n",
			pT_(colour), pT_(colour), pT_(colour),
			pT_(colour), pT_(colour), pT_(colour));
	} else {
		fprintf(fp, "\tlist_%s:tail -> %s"
			" [color=\"%s4\", style=\"dotted\", arrowhead=\"empty\"];\n"
			"\tlist_%s:head -> %s [color=\"%s\"];\n",
			pT_(colour), pT_(name)(list->u.flat.prev), colour,
			pT_(colour), pT_(name)(list->u.flat.next), colour);
	}
	for(link = T_(head)(list); link; link = T_(link_next)(link)) {
		if(is_nodes) {
			t_(to_string)(link, &a);
			fprintf(fp, "\t%s [label=\"%s\"];\n", pT_(name)(link), a);
		}
		fprintf(fp, "\t%s -> %s [color=\"%s\"];\n"
			"\t%s -> %s [color=\"%s4\", style=\"dotted\","
			" arrowhead=\"empty\"];\n",
			pT_(name)(link), pT_(name)(link->next), colour,
			pT_(name)(link), pT_(name)(link->prev), colour);
	}
}

#			define BOX_PUBLIC_OVERRIDE
#			include "box.h"

/** Graph `list` in `fp`. */
static void T_(graph)(const struct t_(list) *const list, FILE *const fp) {
	assert(list && fp);
	fprintf(fp, "digraph {\n"
		"\tgraph [truecolor=true, bgcolor=transparent, fontname=modern];\n"
		"\tnode [fillcolor=\"Gray95\", fontname=modern,"
		" style=filled, shape=box];\n");
	pT_(subgraph)(list, fp, "royalblue", 0, 1);
	fprintf(fp, "\tnode [colour=\"Red\"];\n"
		"}\n");
	assert(!errno);
}

#		elif defined POOL_NAME

#			define BOX_PUBLIC_OVERRIDE
#			include "box.h"

/** Graphs `pool` output to `fp`. */
static void T_(graph)(const struct t_(pool) *const pool,
	FILE *const fp) {
	char str[12];
	size_t i, j;
	struct pT_(slot) *slot;
	pT_(type) *slab;

	assert(pool && fp);
	fprintf(fp, "digraph {\n"
		"\tgraph [rankdir=LR, truecolor=true, bgcolor=transparent,"
		" fontname=modern];\n"
		"\tnode [shape=none, fontname=modern];\n");
	/* Free heap. */
	if(!pool->free0.as_array.size) goto no_free0;
	for(i = 0; i < pool->free0.as_array.size; i++) {
		fprintf(fp, "\tfree0_%lu [label=<<font color=\"Gray50\""
			" face=\"Times-Italic\">%lu</font>>, width=0, height=0,"
			" margin=0.03, shape=circle, style=filled, fillcolor=\"Gray95\"];\n",
			i, pool->free0.as_array.data[i]);
		if(i) fprintf(fp, "\tfree0_%lu -> free0_%lu [dir=back];\n",
			i, (unsigned long)((i - 1) / 2));
	}
	fprintf(fp, "\t{rank=same; free0_0; pool; slots; }\n"
		"\tfree0_0 -> pool:free [dir=back];\n");
no_free0:
	/* Top label. */
	fprintf(fp, "\tpool [label=<\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td colspan=\"3\" align=\"left\">"
		"<font color=\"Grey75\">&lt;" QUOTE(POOL_NAME)
		"&gt;pool: " QUOTE(POOL_TYPE) "</font></td></tr>\n"
		"\t<hr/>\n"
		"\t<tr>\n"
		"\t\t<td border=\"0\" align=\"right\">freeheap0</td>\n"
		"\t\t<td border=\"0\" align=\"right\">%lu</td>\n"
		"\t\t<td port=\"free\" border=\"0\" align=\"right\">%lu</td>\n"
		"\t</tr>\n"
		"\t<tr>\n"
		"\t\t<td border=\"0\" align=\"right\" bgcolor=\"Gray95\">capacity0</td>\n"
		"\t\t<td border=\"0\" bgcolor=\"Gray95\"></td>\n"
		"\t\t<td border=\"0\" align=\"right\" bgcolor=\"Gray95\">%lu</td>\n"
		"\t</tr>\n",
		(unsigned long)pool->free0.as_array.size,
		(unsigned long)pool->free0.as_array.capacity,
		(unsigned long)pool->capacity0);
	fprintf(fp, "\t<tr>\n"
		"\t\t<td border=\"0\" align=\"right\">slots</td>\n"
		"\t\t<td border=\"0\" align=\"right\">%lu</td>\n"
		"\t\t<td port=\"slots\" border=\"0\" align=\"right\">%lu</td>\n"
		"\t</tr>\n"
		"\t<hr/>\n"
		"\t<tr><td></td></tr>\n"
		"</table>>];\n",
		(unsigned long)pool->slots.size,
		(unsigned long)pool->slots.capacity);
	/* Slots. */
	if(!pool->slots.data) goto no_slots;
	fprintf(fp, "\tpool:slots -> slots;\n"
		"\tslots [label = <\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td></td></tr>\n"
		"\t<hr/>\n"
		"\t<tr>\n"
		"\t\t<td border=\"0\"><font face=\"Times-Italic\">i</font></td>\n"
		"\t\t<td border=\"0\"><font face=\"Times-Italic\">slab</font></td>\n"
		"\t\t<td border=\"0\"><font face=\"Times-Italic\">size</font></td>\n"
		"\t</tr>\n"
		"\t<hr/>\n");
	for(i = 0; i < pool->slots.size; i++) {
		const char *const bgc = i & 1 ? " bgcolor=\"Grey95\"" : "";
		fprintf(fp, "\t<tr>\n"
			"\t\t<td align=\"right\"%s>%lu</td>\n"
			"\t\t<td align=\"left\"%s>%s</td>\n"
			"\t\t<td port=\"%lu\" align=\"right\"%s>%lu</td>\n"
			"\t</tr>\n",
			bgc, (unsigned long)i, bgc, orcify(pool->slots.data[i].slab),
			(unsigned long)i, bgc, pool->slots.data[i].size);
	}
	fprintf(fp, "\t<hr/>\n"
		"\t<tr><td></td></tr>\n"
		"</table>>];\n");
	/* For each slot, there is a slab. */
	for(i = 0; i < pool->slots.size; i++) {
		char *bmp;
		slot = pool->slots.data + i;
		slab = slot->slab;
		fprintf(fp,
			"\tslots:%lu -> slab%lu;\n"
			"\tslab%lu [label=<\n"
			"<table border=\"0\" cellspacing=\"0\">\n"
			"\t<tr><td align=\"left\">"
			"<font color=\"Gray75\">%s</font></td></tr>\n"
			"\t<hr/>\n",
			(unsigned long)i, (unsigned long)i,
			(unsigned long)i, orcify(slab));
		if(i || !slot->size) {
			fprintf(fp, "\t<tr><td align=\"left\">"
				"<font face=\"Times-Italic\">count %lu</font></td></tr>\n",
				slot->size);
			goto no_slab_data;
		}
		/* Primary buffer: print rows. */
		if(!(bmp = calloc(slot->size, sizeof *bmp)))
			{ perror("temp bitmap"); assert(0); exit(EXIT_FAILURE); }
		for(j = 0; j < pool->free0.as_array.size; j++) {
			size_t *f0p = pool->free0.as_array.data + j;
			assert(f0p && *f0p < slot->size);
			bmp[*f0p] = 1;
		}
		for(j = 0; j < slot->size; j++) {
			const char *const bgc = j & 1 ? " bgcolor=\"Grey95\"" : "";
			fprintf(fp, "\t<tr><td port=\"%lu\" align=\"left\"%s>",
				(unsigned long)j, bgc);
			if(bmp[j]) {
				fprintf(fp, "<font color=\"Gray50\" face=\"Times-Italic\">%lu</font>",
					(unsigned long)j);
			} else {
				t_(to_string)(slab + j, &str);
				fprintf(fp, "%s", str);
			}
			fprintf(fp, "</td></tr>\n");
		}
		free(bmp);
no_slab_data:
		fprintf(fp, "\t<hr/>\n"
			"\t<tr><td></td></tr>\n"
			"</table>>];\n");
		/* Too crowded to draw heap.
		if(i) continue;
		for(j = 1; j < pool->free0._.size; j++) {
			const size_t *const f0low = pool->free0._.data + j / 2,
				*const f0high = pool->free0._.data + j;
			fprintf(fp, "\tslab0:%lu -> slab0:%lu;\n", *f0low, *f0high);
		} */
	}
no_slots:
	fprintf(fp, "\tnode [fillcolour=red];\n"
		"}\n");
}

#		elif defined TABLE_NAME

#			include <math.h> /* sqrt NAN? */
#			ifndef NAN /* <https://stackoverflow.com/questions/5714131/nan-literal-in-c> */
#				define NAN (0. / 0.)
#			endif

/** Counts linked buckets in `table` index `idx`. */
static size_t pT_(count_bucket)(const struct t_(table) *const table,
	pT_(uint) idx) {
	struct pT_(bucket) *bucket;
	pT_(uint) next;
	size_t no = 0;
	assert(table && idx < pT_(capacity)(table));
	bucket = table->buckets + idx;
	if((next = bucket->next) == TABLE_NULL
		|| idx != pT_(chain_head)(table, bucket->hash)) return 0;
	for( ; no++, next != TABLE_END;
		next = bucket->next, assert(next != TABLE_NULL)) {
		idx = next;
		assert(idx < pT_(capacity)(table)
			/* We want to count intermediates.
			 && pT_(in_stack_range)(hash, idx) */);
		bucket = table->buckets + idx;
	}
	return no;
}

#			ifndef TEST_TABLE_H
#				define TEST_TABLE_H
/** <Welford1962Note>: population variance: `ssdm/n`,
 sample variance: `ssdm/(n-1)`. */
struct table_stats { size_t n, max; double mean, ssdm; };
#			endif

/** Update one sample point `st` of `value`. */
static void pT_(update)(struct table_stats *const st, const size_t value) {
	double d, v = (double)value;
	if(st->max < value) st->max = value;
	d = v - st->mean;
	st->mean += d / (double)++st->n;
	st->ssdm += d * (v - st->mean);
}
/** Collect stats on `table`. */
static struct table_stats pT_(collect)(const struct t_(table) *const table) {
	struct table_stats st = { 0, 0, 0.0, 0.0 };
	pT_(uint) i, i_end;
	if(!table || !table->buckets) return st;
	for(i = 0, i_end = pT_(capacity)(table); i < i_end; i++) {
		size_t no;
		/* I'm sure there's a cheaper way to do it. */
		for(no = pT_(count_bucket)(table, i); no; no--) pT_(update)(&st, no);
	}
	return st;
}

#			define BOX_PUBLIC_OVERRIDE
#			include "box.h"

/** Draw a diagram of `table` written to `fp` in
 [Graphviz](https://www.graphviz.org/) format.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void T_(graph)(const struct t_(table) *const table, FILE *const fp) {
	size_t i, i_end;
	struct table_stats st = pT_(collect)(table);
	assert(table && fp);
	fprintf(fp, "digraph {\n"
		"\tgraph [rankdir=LR, truecolor=true, bgcolor=transparent,"
		" fontname=modern];\n"
		"\tnode [shape=none, fontname=modern];\n");
	if(!table->buckets) { fprintf(fp, "\tidle;\n"); goto end; }
	assert(table->size >= st.n); /* This is not obvious. */
	fprintf(fp,
		"\thash [label=<\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td colspan=\"2\" align=\"left\">"
		"<font color=\"Gray75\">&lt;" QUOTE(TABLE_NAME)
		"&gt;table: " QUOTE(TABLE_KEY) "</font></td></tr>\n"
		"\t<hr/>\n"
		"\t<tr>\n"
		"\t\t<td border=\"0\" align=\"right\">load factor</td>\n");
	fprintf(fp,
		"\t\t<td border=\"0\" align=\"right\">%lu/%lu</td>\n"
		"\t</tr>\n"
		"\t<tr>\n"
		"\t\t<td border=\"0\" align=\"right\" bgcolor=\"Gray95\">"
		"E[no bucket]</td>\n"
		"\t\t<td border=\"0\" align=\"right\" bgcolor=\"Gray95\">"
		"%.2f(%.1f)</td>\n"
		"\t</tr>\n"
		"\t<tr>\n"
		"\t\t<td border=\"0\" align=\"right\">max chain</td>\n"
		"\t\t<td border=\"0\" align=\"right\">%lu</td>\n"
		"\t</tr>\n"
		"\t<hr/>\n"
		"\t<tr><td></td></tr>\n"
		"</table>>];\n"
		"\thash -> data;\n"
		"\t{ rank=same; hash; data; }\n",
		(unsigned long)st.n,
		table->buckets ? (unsigned long)pT_(capacity)(table) : 0,
		st.n ? st.mean : (double)NAN, st.n > 1
		? sqrt(st.ssdm / (double)(st.n - 1)) : (double)NAN,
		(unsigned long)st.max);
	fprintf(fp,
		"\tdata [label=<\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td colspan=\"4\"></td></tr>\n"
		"\t<hr/>\n"
		"\t<tr>\n"
		"\t\t<td border=\"0\"><font face=\"Times-Italic\">i</font></td>\n"
		"\t\t<td border=\"0\"><font face=\"Times-Italic\">hash</font></td>\n"
		"\t\t<td border=\"0\"><font face=\"Times-Italic\">key</font></td>\n"
		"\t\t<td border=\"0\"><font face=\"Times-Italic\">next</font></td>\n"
		"\t</tr>\n"
		"\t<hr/>\n");
	for(i = 0, i_end = pT_(capacity)(table); i < i_end; i++) {
		const char *const bgc = i & 1 ? " bgcolor=\"Gray95\"" : "",
			*const top = (table->top & ~TABLE_HIGH) == i
			? (table->top & TABLE_HIGH) ? " border=\"1\"" : " border=\"2\"" : "";
		struct pT_(bucket) *b = table->buckets + i;
		fprintf(fp, "\t<tr>\n"
			"\t\t<td align=\"right\"%s%s>"
			"<font face=\"Times-Italic\">0x%lx</font></td>\n",
			top, bgc, (unsigned long)i);
		if(b->next != TABLE_NULL) {
			const char *const closed
				= pT_(chain_head)(table, b->hash) == i ? "⬤" : "◯";
			char z[12];
#			ifdef TABLE_VALUE
			t_(to_string)(pT_(bucket_key)(b), pT_(bucket_value)(b), &z);
#			else
			t_(to_string)(pT_(bucket_key)(b), &z);
#			endif
			fprintf(fp, "\t\t<td align=\"right\"%s>0x%lx</td>\n"
				"\t\t<td align=\"left\"%s>"
#			ifdef TABLE_UNHASH
				"<font face=\"Times-Italic\">"
#			endif
				"%s"
#			ifdef TABLE_UNHASH
				"</font>"
#			endif
				"</td>\n"
				"\t\t<td port=\"%lu\"%s>%s</td>\n",
				bgc, (unsigned long)b->hash,
				bgc, z,
				(unsigned long)i, bgc, closed);
		} else {
			fprintf(fp, "\t\t<td%s></td><td%s></td><td%s></td>\n",
				bgc, bgc, bgc);
		}
		fprintf(fp, "\t</tr>\n");
	}
	fprintf(fp, "\t<hr/>\n"
		"\t<tr><td colspan=\"4\"></td></tr>\n"
		"</table>>];\n");
	fprintf(fp, "\tnode [shape=plain, fillcolor=none]\n");
	for(i = 0, i_end = pT_(capacity)(table); i < i_end; i++) {
		struct pT_(bucket) *b = table->buckets + i;
		pT_(uint) left, right;
		if((right = b->next) == TABLE_NULL || right == TABLE_END) continue;
		if(pT_(chain_head)(table, b->hash) != i) {
			fprintf(fp, "\ti0x%lx [label=\"0x%lx\", fontcolor=\"Gray\"];\n"
				"\tdata:%lu -> i0x%lx [color=\"Gray\"];\n",
				(unsigned long)right, (unsigned long)right,
				i, (unsigned long)right);
			continue;
		}
		fprintf(fp,
			"\te%lu [label=\"0x%lx\"];\n"
			"\tdata:%lu -> e%lu [tailclip=false];\n",
			(unsigned long)right, (unsigned long)right,
			(unsigned long)i, (unsigned long)right);
		while(left = right, b = table->buckets + left,
			(right = b->next) != TABLE_END) {
			assert(right != TABLE_NULL);
			fprintf(fp,
				"\te%lu [label=\"0x%lx\"];\n"
				"\te%lu -> e%lu;\n",
				(unsigned long)right, (unsigned long)right,
				(unsigned long)left, (unsigned long)right);
		}
	}
end:
	fprintf(fp, "\tnode [color=red];\n"
		"}\n");
}

#		elif defined TREE_NAME

/** Recursively draws `sub` in `fp`. */
static void pT_(subgraph)(const struct pT_(tree) *const sub, FILE *fp) {
	const struct pT_(stem) *branch;
	unsigned i;
	assert(sub->bough && fp && sub->height);
	fprintf(fp, "\tbough%p [label = <\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td border=\"0\" port=\"0\">"
		"<font color=\"Gray75\">%s</font> (h %u)</td></tr>\n",
		(const void *)sub->bough, orcify(sub->bough), sub->height);
	if(sub->bough->size) fprintf(fp, "\t<hr/>\n");
	for(i = 0; i < sub->bough->size; i++) {
		const char *const bgc = i & 1 ? " bgcolor=\"Gray95\"" : "";
		char z[12];
#			ifdef TREE_VALUE
		t_(to_string)(sub->bough->key[i], sub->bough->value + i, &z);
#			else
		t_(to_string)(sub->bough->key[i], &z);
#			endif
		fprintf(fp, "\t<tr><td border=\"0\" align=\"left\""
			" port=\"%u\"%s>%s</td></tr>\n", i + 1, bgc, z);
	}
	fprintf(fp, "\t<hr/>\n"
		"\t<tr><td></td></tr>\n"
		"</table>>];\n");
	if(sub->height <= 1) return;
	/* Draw the lines between trees. */
	branch = pT_(as_branch_c)(sub->bough);
	for(i = 0; i <= branch->base.size; i++)
		fprintf(fp, "\tbough%p:%u:se -> bough%p;\n",
		(const void *)sub->bough, i, (const void *)branch->child[i]);
	/* Recurse. */
	for(i = 0; i <= branch->base.size; i++) {
		struct pT_(tree) child;
		child.bough = branch->child[i], child.height = sub->height - 1;
		pT_(subgraph)(&child, fp);
	}
}

/** Aligns the `port` in the right way between nodes. */
static char *pT_(usual_port)(unsigned port) {
	static char s[32];
	if(!port) sprintf(s, "0:sw");
	else sprintf(s, "%u:se", port - 1);
	return s;
}

/** Recursively draws `sub` in `fp`. */
static void pT_(subgraph_usual)(const struct pT_(tree) *const sub, FILE *fp) {
	const struct pT_(stem) *branch;
	unsigned i;
	assert(sub->bough && fp);
	fprintf(fp, "\ttrunk%p [label = <\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td border=\"0\" colspan=\"%u\">"
		"<font color=\"Gray75\">%s</font></td></tr>\n"
		"\t<hr/>\n"
		"\t<tr>\n", (const void *)sub->bough,
		sub->bough->size ? sub->bough->size : 1, orcify(sub->bough));
	for(i = 0; i < sub->bough->size; i++) {
		char z[12];
#			ifdef TREE_VALUE
		t_(to_string)(sub->bough->key[i], sub->bough->value + i, &z);
#			else
		t_(to_string)(sub->bough->key[i], &z);
#			endif
		fprintf(fp, "\t<td border=\"0\" align=\"center\""
			" port=\"%u\">%s</td>\n", i, z);
	}
	/* Dummy node when size is zero. */
	if(!sub->bough->size)
		fprintf(fp, "\t<td border=\"0\" port=\"0\">&nbsp;</td>\n");
	fprintf(fp, "\t</tr>\n");
	if(sub->height) fprintf(fp, "\t<hr/>\n"
		"\t<tr><td></td></tr>\n");
	fprintf(fp, "</table>>];\n");
	if(!sub->height) return;
	/* Draw the lines between trees. */
	branch = pT_(as_branch_c)(sub->bough);
	for(i = 0; i <= branch->base.size; i++)
		fprintf(fp, "\ttrunk%p:%s -> trunk%p;\n",
		(const void *)sub->bough, pT_(usual_port)(i),
		(const void *)branch->child[i]);
	/* Recurse. */
	for(i = 0; i <= branch->base.size; i++) {
		struct pT_(tree) child;
		child.bough = branch->child[i], child.height = sub->height - 1;
		pT_(subgraph_usual)(&child, fp);
	}
}

static void pT_(graph)(const struct pT_(tree) *const trunk, FILE *const fp) {
	assert(trunk);
	fprintf(fp, "digraph {\n"
		"\tgraph [rankdir=LR, truecolor=true, bgcolor=transparent,"
		" fontname=modern, splines=false];\n"
		"\tnode [shape=none, fontname=modern];\n");
	if(!trunk->bough)
		fprintf(fp, "\tidle [shape=plaintext];\n");
	else if(!trunk->height)
		fprintf(fp, "\tempty [shape=plaintext];\n");
	else pT_(subgraph)(trunk, fp);
	fprintf(fp, "\tnode [color=\"Red\"];\n"
		"}\n");
}

#			define BOX_PUBLIC_OVERRIDE
#			include "box.h"

/** Draw a graph of `tree` to `fp` in Graphviz format. */
static void T_(graph)(const struct t_(tree) *const tree, FILE *const fp) {
	assert(tree && fp);
	pT_(graph)(&tree->trunk, fp);
}

/** Draw a graph of `tree` to `fn` in Graphviz format, the usual way, but too
 large for many labels. The vertical graph is so much more compact, but this is
 very idiomatic when one needs to be. This is not used. */
static int T_(graph_horiz_fn)(const struct t_(tree) *const tree,
	const char *const fn) {
	FILE *fp;
	assert(tree && fn);
	/*printf("***(horizontal) %s.\n\n", fn);*/
	if(!(fp = fopen(fn, "w"))) { if(!errno) errno = ERANGE; return 0; }
	fprintf(fp, "digraph {\n"
		"\tgraph [truecolor=true, bgcolor=transparent,"
		" fontname=modern, splines=false];\n"
		"\tnode [shape=none, fontname=\"Bitstream Vera Sans\"];\n"
		"\n");
	if(!tree->trunk.bough)
		fprintf(fp, "\tidle [shape=plaintext];\n");
	else if(!tree->trunk.height)
		fprintf(fp, "\tempty [shape=plaintext];\n");
	else pT_(subgraph_usual)(&tree->trunk, fp);
	fprintf(fp, "\tnode [color=\"Red\"];\n"
		"}\n");
	fclose(fp);
	return 1;
}

static void pT_(unused_tree_coda)(void);
static void pT_(unused_tree)(void)
	{ T_(graph_horiz_fn)(0, 0); pT_(unused_tree_coda)(); }
static void pT_(unused_tree_coda)(void) { pT_(unused_tree)(); }

#		elif defined TRIE_NAME

/** Outputs a direction string for `lf` in `tr`, `{ "", "r", "l" }`. */
static const char *pT_(leaf_to_dir)(const struct pT_(tree) *const tr,
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
static unsigned pT_(right)(const struct pT_(tree) *const tr,
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
static unsigned pT_(left_leaf)(const struct pT_(tree) *const tr,
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
static void pT_(graph_tree_bits)(const struct pT_(tree) *const tree,
	const size_t treebit, FILE *const fp) {
	unsigned b, i;
	assert(tree && fp);
	fprintf(fp, "\ttree%pbranch0 [label = <\n"
		"<table border=\"0\" cellspacing=\"0\">\n", (const void *)tree);
	/*"<table BORDER=\"0\" CELLBORDER=\"0\">\n"*/
	for(i = 0; i <= tree->bsize; i++) {
		const char *key = pT_(sample)(tree, i);
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
		(const void *)tree->leaf[i].as_link, pT_(leaf_to_dir)(tree, i));
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
		pT_(graph_tree_bits)(tree->leaf[i].as_link, bit, fp);
	}
}

/** Graphs `tree` on `fp`. `treebit` is the number of bits currently
 (recursive.) */
static void pT_(graph_tree_mem)(const struct pT_(tree) *const tree,
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
		const char *key = pT_(sample)(tree, i);
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
		(const void *)tree->leaf[i].as_link, pT_(leaf_to_dir)(tree, i));
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
		pT_(graph_tree_mem)(tree->leaf[i].as_link, bit, fp);
	}
}

/** Graphs `tr` on `fp`.`treebit` is the number of bits currently
 (recursive.) */
static void pT_(graph_tree_logic)(const struct pT_(tree) *const tr,
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
			left = branch->left, right = pT_(right)(tr, b);
			fprintf(fp, "\ttree%pbranch%u [label=\"%u\", shape=circle,"
				" style=filled, fillcolor=Grey95];\n"
				"\ttree%pbranch%u -> ", (const void *)tr, b, branch->skip,
				(const void *)tr, b);
			if(left) {
				fprintf(fp, "tree%pbranch%u [arrowhead=rnormal];\n",
					(const void *)tr, b + 1);
			} else {
				unsigned leaf = pT_(left_leaf)(tr, b);
				if(trie_bmp_test(&tr->bmp, leaf)) {
					const struct pT_(tree) *const child =tr->leaf[leaf].as_link;
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
				unsigned leaf = pT_(left_leaf)(tr, b) + left + 1;
				if(trie_bmp_test(&tr->bmp, leaf)) {
					const struct pT_(tree) *const child =tr->leaf[leaf].as_link;
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

	for(i = 0; i <= tr->bsize; i++) if(!trie_bmp_test(&tr->bmp, i)) {
		union { const struct pT_(tree) *readonly; struct pT_(tree) *promise; }
			slybox;
		struct pT_(ref) ref;
		slybox.readonly = tr, ref.tree = slybox.promise, ref.lf = i;
		fprintf(fp,
			"\ttree%pleaf%u [label = <%s<font color=\"Gray75\">⊔</font>>];\n",
			(const void *)tr, i, pT_(ref_to_string)(&ref));
	}

	for(i = 0; i <= tr->bsize; i++) if(trie_bmp_test(&tr->bmp, i))
		pT_(graph_tree_logic)(tr->leaf[i].as_link, 0, fp);
}

typedef void (*pT_(tree_file_fn))(const struct pT_(tree) *, size_t, FILE *);

/** Draw a graph of `trie` to `fn` in Graphviz format with `callback` as it's
 tree-drawing output. */
static void pT_(graph_choose)(const struct t_(trie) *const trie,
	const char *const fn, const pT_(tree_file_fn) callback) {
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

#			define BOX_PUBLIC_OVERRIDE
#			include "box.h"

/** Draws `trie` on `fp` as a bit view. */
static void T_(graph)(const struct t_(trie) *const trie,
	FILE *const fp) { pT_(graph_tree_bits)(trie->root, 0, fp); }

/** Graphs logical, memory, and bit of `trie` output to `fn` using `no` as the
 filename index. */
static void T_(graph_all)(const struct t_(trie) *const trie,
	const char *const fn, const size_t no) {
	const char logic[] = "-tree", mem[] = "-mem", bits[] = "-bits";
	char copy[128], *dot;
	size_t fn_len = strlen(fn), i, i_copy;
	if(fn_len > sizeof copy - 30/*SIZE_MAX*/ - 1 || !(dot = strrchr(fn, '.'))) {
		fprintf(stderr, "Too long or doesn't have extension: <%s>.\n", fn);
		assert(0);
		return;
	}
	/*printf(" *** graph: \"%s\" %lu.\n", fn, (unsigned long)no);*/
	/* Insert number. */
	i = (size_t)(dot - fn), memcpy(copy, fn, i_copy = i);
	copy[i_copy++] = '-';
	sprintf(copy + i_copy, "%lu", (unsigned long)no),
		i_copy += strlen(copy + i_copy);

	memcpy(copy + i_copy, bits, sizeof bits - 1);
	memcpy(copy + i_copy + sizeof bits - 1, fn + i, fn_len - i + 1);
	pT_(graph_choose)(trie, copy, &pT_(graph_tree_bits));
	memcpy(copy + i_copy, logic, sizeof logic - 1);
	memcpy(copy + i_copy + sizeof logic - 1, fn + i, fn_len - i + 1);
	pT_(graph_choose)(trie, copy, &pT_(graph_tree_logic));
	memcpy(copy + i_copy, mem, sizeof mem - 1);
	memcpy(copy + i_copy + sizeof mem - 1, fn + i, fn_len - i + 1);
	pT_(graph_choose)(trie, copy, &pT_(graph_tree_mem));
}

#		else
#			error Not something we know how to make a graph out of?
#		endif
#		undef QUOTE
#		undef QUOTE_

/** Graphs `box` in GraphViz format to `fn`. */
static int T_(graph_fn)(const pT_(box) *const box, const char *const fn) {
	FILE *fp;
	assert(box && fn);
	assert(!errno); /* Catches bugs, but probably shouldn't be here. */
	if(!(fp = fopen(fn, "w")))
		{ if(!errno) errno = ERANGE; return 0; }
	T_(graph)(box, fp);
	fclose(fp);
	return 1;
}

#		define BOX_PRIVATE_AGAIN
#		include "box.h"
static void pT_(unused_graph_coda)(void);
static void pT_(unused_graph)(void)
	{ T_(graph)(0, 0); T_(graph_fn)(0, 0); pT_(unused_graph_coda)(); }
static void pT_(unused_graph_coda)(void) { pT_(unused_graph)(); }

#	endif /* Not declare-only. */
#endif /* Top. */

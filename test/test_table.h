/** A call with the container unknown. This is so that the function is free to
 return a key which is part of a larger aggregate structure. `fill` is
 initialized except if <typedef:<PN>entry> contains a value, the value will be
 a valid pointer to a temporary space for copying. */
typedef int (*const pT_(fill_fn))(void *, pT_(entry) *fill);


#	if defined QUOTE || defined QUOTE_
#		error Cannot be defined.
#	endif
#	define QUOTE_(name) #name
#	define QUOTE(name) QUOTE_(name)
#	ifdef static /* Private functions. */
#		undef static
#	endif


#include <stdio.h>  /* fprintf FILE */
#include <math.h>   /* sqrt NAN? */
#ifndef NAN /* <https://stackoverflow.com/questions/5714131/nan-literal-in-c> */
#	define NAN (0. / 0.)
#endif
#include <string.h> /* memset */

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

#ifndef TEST_TABLE_H
#	define TEST_TABLE_H
/** <Welford1962Note>: population variance: `ssdm/n`,
 sample variance: `ssdm/(n-1)`. */
struct table_stats { size_t n, max; double mean, ssdm; };
#endif
/** Update one sample point of `value`. */
static void pT_(update)(struct table_stats *const st, const size_t value) {
	double d, v = (double)value;
	if(st->max < value) st->max = value;
	d = v - st->mean;
	st->mean += d / (double)++st->n;
	st->ssdm += d * (v - st->mean);
}
/** Collect stats on `hash`. */
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

/** Draw a diagram of `hash` written to `fn` in
 [Graphviz](https://www.graphviz.org/) format.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void pT_(graph)(const struct t_(table) *const table,
	const char *const fn) {
	FILE *fp;
	size_t i, i_end;
	struct table_stats st = pT_(collect)(table);
	assert(table && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	printf("*** %s\n", fn);
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
#ifdef TABLE_VALUE
			t_(to_string)(pT_(bucket_key)(b), pT_(bucket_value)(b), &z);
#else
			t_(to_string)(pT_(bucket_key)(b), &z);
#endif
			fprintf(fp, "\t\t<td align=\"right\"%s>0x%lx</td>\n"
				"\t\t<td align=\"left\"%s>"
#ifdef TABLE_UNHASH
				"<font face=\"Times-Italic\">"
#endif
				"%s"
#ifdef TABLE_UNHASH
				"</font>"
#endif
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
	fclose(fp);
}

/** Draw a histogram of `hash` written to `fn` in
 [Gnuplot](http://www.gnuplot.info/) format.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void pT_(histogram)(const struct t_(table) *const table,
	const char *const fn) {
	FILE *fp;
	size_t histogram[64], hs, h;
	const size_t histogram_size = sizeof histogram / sizeof *histogram;
	/* Stats for fit. fixme: This is stats for expected value, I don't think
	 they are the same. */
	struct table_stats st = pT_(collect)(table);
	assert(table && fn);
	memset(histogram, 0, sizeof histogram);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	if(table->buckets) {
		pT_(uint) i, i_end = pT_(capacity)(table);
		for(i = 0; i < i_end; i++) {
			size_t bucket = pT_(count_bucket)(table, i);
			/* The bins are `0,1,2,...,[histogram_size - 1, \infty]`. */
			if(bucket >= histogram_size) bucket = histogram_size - 1;
			histogram[bucket]++;
		}
	}
	/* `historgram_size` is much larger than it has to be, usually. */
	for(hs = histogram_size - 1; !(histogram[hs] && (hs++, 1)) && hs; hs--);
	fprintf(fp, "# Size: %lu.\n"
		"set term postscript eps enhanced color\n"
		"set output \"%s.eps\"\n"
		"set grid\n"
		"set xlabel \"bucket occupancy\"\n"
		"set ylabel \"frequency\"\n"
		"set style histogram\n"
		"set xrange [0:]\n"
		"unset key\n"
		"poisson(x) = buckets*lambda**x/int(x)!*exp(-lambda)\n"
		"lambda = %f\n"
		"buckets = %lu\n"
		"# what I really want is the Gamma distribution\n"
		"plot \"-\" using ($1+0.5):2 with boxes lw 3 title \"Histogram\", \\\n"
		"\tpoisson(int(x)) with lines linestyle 2 title \"Fit\"\n",
		(unsigned long)table->size, fn,
		st.mean, (unsigned long)st.n);
	for(h = 0; h < hs; h++) fprintf(fp, "%lu\t%lu\n",
		(unsigned long)h, (unsigned long)histogram[h]);
	fclose(fp);
}

/** @return Equality of entries `a` and `b`. */
static int pT_(eq_key)(/*pT_(key_c) a, pT_(key_c) b*/
	const pT_(key) a, const pT_(key) b) {
#ifdef TABLE_UNHASH /* Compare in <typedef:<PN>uint> space. */
	return t_(hash)(a) == t_(hash)(b);
#else
	return t_(is_equal)(a, b);
#endif
}

/** @return Key from `e`. */
static pT_(key) pT_(entry_key)(pT_(entry) e) {
#ifdef TABLE_VALUE
	return e.key;
#else
	return e;
#endif
}

/** Assertion function for seeing if `hash` is in a valid state.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void pT_(legit)(const struct t_(table) *const table) {
	pT_(uint) i, i_end;
	size_t size = 0, end = 0, start = 0;
	if(!table) return; /* Null. */
	if(!table->buckets) { /* Idle. */
		assert(!table->log_capacity && !table->size);
		return;
	}
	assert(table->log_capacity >= 3);
	for(i = 0, i_end = pT_(capacity)(table); i < i_end; i++) {
		struct pT_(bucket) *b = table->buckets + i;
		if(b->next == TABLE_NULL) continue;
		size++;
		if(b->next == TABLE_END) end++;
		if(i == pT_(chain_head)(table, b->hash)) start++;
	}
	assert(table->size == size && end == start && size >= start);
}

/** Passed `parent_new` and `parent` from <fn:<N>hash_test>. */
static void pT_(test_basic)(void *const parent) {
	struct {
		struct sample {
			pT_(entry) entry;
			int is_in;
		} sample[1000];
		size_t count;
	} trials;
	const size_t trial_size = sizeof trials.sample / sizeof *trials.sample,
		max_graph = ((pT_(uint))~0) > 1000 ? 1000 : ((pT_(uint))~0);
	size_t i;
	pT_(uint) b, b_end, count1, count2;
	char z[12];
	struct t_(table) table = t_(table)();
	struct T_(cursor) it;
	int success;
	assert(trial_size > 1);
	/* Pre-computation. O(element_size*(element_size-1)/2); this places a limit
	 on how much a reasonable test is. */
	for(i = 0; i < trial_size; i++) {
		struct sample *s = trials.sample + i;
		size_t j;
/*#ifdef TABLE_VALUE
		s->entry.value = &s.temp_value;
#endif*/
		t_(filler)(parent, &s->entry);
#ifdef TABLE_VALUE
		t_(to_string)(pT_(entry_key)(s->entry), s->entry.value, &z);
#else
		t_(to_string)(pT_(entry_key)(s->entry), &z);
#endif
		/* Is supposed to be in set. */
		s->is_in = 0;
		for(j = 0; j < i && !pT_(eq_key)(pT_(entry_key)(s->entry),
			pT_(entry_key)(trials.sample[j].entry)); j++);
		if(j == i) s->is_in = 1;
	}
	/* Test idle. */
	pT_(legit)(&table);
	assert(!table.buckets && !table.log_capacity && !table.size);
	pT_(legit)(&table);
	pT_(graph)(&table, "graph/table/" QUOTE(TABLE_NAME) "-0.gv");
	success = T_(buffer)(&table, 1);
	assert(success && table.buckets && table.log_capacity == 3 && !table.size);
	success = T_(buffer)(&table, 1);
	assert(success && table.buckets && table.log_capacity == 3 && !table.size);
	T_(clear)(&table);
	assert(table.buckets && table.log_capacity == 3 && !table.size);
	{ /* Test negative `get_or`. */
		pT_(key) key = pT_(entry_key)(trials.sample[0].entry);
		pT_(value) ret, def;
		int cmp;
		memset(&def, 1, sizeof def);
		ret = T_(get_or)(&table, key, def);
		cmp = memcmp(&ret, &def, sizeof def);
		assert(!cmp);
	}
	t_(table_)(&table);
	assert(!table.buckets && table.log_capacity == 0 && !table.size);
	/* Test placing items. */
	for(i = 0; i < trial_size; i++) {
		struct { pT_(uint) before, after; } size;
		enum table_result result;
		const struct sample *s = trials.sample + i;
		pT_(key) key, eject, zero;
		memset(&eject, 0, sizeof eject);
		memset(&zero, 0, sizeof zero);
		size.before = table.size;
		result = T_(policy)(&table, pT_(entry_key)(s->entry), &eject, 0);
		size.after = table.size;
		if(s->is_in) {
			assert(!memcmp(&eject, &zero, sizeof zero)
				&& result == TABLE_ABSENT && size.after == size.before + 1);
		} else {
			assert(result == TABLE_PRESENT && size.before == size.after);
		}
#ifdef TABLE_VALUE
		success = T_(query)(&table, pT_(entry_key)(s->entry), &key, 0);
#else
		success = T_(query)(&table, pT_(entry_key)(s->entry), &key);
#endif
		assert(success && pT_(eq_key)(pT_(entry_key)(s->entry), key));
		if(table.size < max_graph && !(i & (i - 1)) || i + 1 == trial_size) {
			char fn[64];
#ifdef TABLE_VALUE
			t_(to_string)(pT_(entry_key)(s->entry), s->entry.value, &z);
#else
			t_(to_string)(pT_(entry_key)(s->entry), &z);
#endif
			/*fix printf("%lu. Store \"%s\" result %s, table %s.\n", (unsigned long)i,
				z, table_result_str[result], T_(table_to_string)(&table));*/
			sprintf(fn, "graph/table/" QUOTE(TABLE_NAME) "-%lu.gv",
				(unsigned long)i+1);
			pT_(graph)(&table, fn);
		}
		pT_(legit)(&table);
	}
	{
		char fn[64];
		sprintf(fn, "graph/table/histogram-" QUOTE(TABLE_NAME) "-%u.gnu",
			(unsigned)trial_size);
		pT_(histogram)(&table, fn);
	}
	printf("Go through the table and see if we can get all the items out.\n");
	{ pT_(value) def; memset(&def, 0, sizeof def);
		for(i = 0; i < trial_size; i++) {
		const struct sample *s = trials.sample + i;
		const pT_(key) key = pT_(entry_key)(s->entry);
		pT_(key) result;
		pT_(value) value;
		const pT_(value) *sample_value;
		int /*cmp,*/ is;
		is = T_(contains)(&table, key);
		assert(is);
#ifdef TABLE_VALUE
		is = T_(query)(&table, key, &result, &value);
#else
		is = T_(query)(&table, key, &result);
#endif
		assert(is && pT_(eq_key)(pT_(entry_key)(s->entry), result));
		value = T_(get_or)(&table, key, def);
#ifdef TABLE_VALUE
		sample_value = &s->entry.value;
#else
		sample_value = &s->entry;
#endif
		/* Any unused bytes will possibly be different. Can't do this. */
		/*cmp = memcmp(&value, array_value, sizeof value);
		printf("sizeof %lu\n", sizeof value);
		printf("%u, %u\n", (unsigned)value, (unsigned)*array_value);
		assert(!cmp);*/ /* <- not doing what I think in vect4 */
		(void)value, (void)sample_value;
	}}
	printf("Table: %s.\n", T_(to_string)(&table));
	printf("Count:\n");
	for(it = T_(begin)(&table), count1 = 0; T_(exists)(&it);
		T_(next)(&it)) count1++;
	assert(count1 == table.size);
	/* Is it kosher? Yes, but it doesn't test anything. Use iterator. */
	for(it = T_(begin)(&table), count2 = 0; T_(exists)(&it);
		T_(next)(&it)) {
		count2++;
		T_(cursor_remove)(&it);
		/*sprintf(fn, "graph/table/" QUOTE(TABLE_NAME) "-end-%u.gv", ++count);
		pT_(graph)(&table, fn);*/
	}
	pT_(graph)(&table, "graph/table/" QUOTE(TABLE_NAME) "-end.gv");
	assert(count1 == count2);
	/* Clear. */
	T_(clear)(&table);
	pT_(graph)(&table, "graph/table/" QUOTE(TABLE_NAME) "-clear.gv");
	pT_(legit)(&table);
	for(b = 0, b_end = b + pT_(capacity)(&table); b < b_end; b++)
		assert(table.buckets[b].next == TABLE_NULL);
	assert(table.size == 0);
	printf("Clear: %s.\n", T_(to_string)(&table));
	for(i = 0; i < trial_size; i++) { /* Make sure to test it again. */
		const struct sample *s = trials.sample + i;
		enum table_result result;
		result = T_(policy)(&table, pT_(entry_key)(s->entry), 0, 0);
		assert(result == TABLE_PRESENT || result == TABLE_ABSENT);
	}
	t_(table_)(&table);
	assert(!table.buckets && !table.log_capacity && !table.size);
}

/** The list will be tested on `stdout`. Requires `TABLE_TEST` to be a
 <typedef:<PN>action_fn> and `TABLE_TO_STRING`.
 @param[parent_new] Specifies the dynamic up-level creator of the parent
 `struct`. Could be null; then testing will be done statically on an array of
 <tag:<N>hashlink> and `TABLE_TEST` is not allowed to go over the limits of the
 data key. @param[parent] The parameter passed to `parent_new`. Ignored if
 `parent_new` is null. @allow */
static void T_(test)(void *const parent) {
	printf("<" QUOTE(TABLE_NAME) ">table of key <" QUOTE(TABLE_KEY)
		"> was created using: "
		"TABLE_UINT <" QUOTE(TABLE_UINT) ">; "
#ifdef TABLE_UNHASH
		"TABLE_UNHASH; "
#endif
#ifdef TABLE_VALUE
		"TABLE_VALUE <" QUOTE(TABLE_VALUE) ">; "
#endif
		"testing%s:\n", parent ? "(pointer)" : "");
	pT_(test_basic)(parent);
	fprintf(stderr, "Done tests of <" QUOTE(TABLE_NAME) ">hash.\n\n");
}

#undef QUOTE
#undef QUOTE_

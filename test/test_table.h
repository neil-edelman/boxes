/** A call with the container unknown. This is so that the function is free to
 return a key which is part of a larger aggregate structure. `fill` is
 initialized except if <typedef:<PN>entry> contains a value, the value will be
 a valid pointer to a temporary space for copying. */
typedef int (*const PN_(fill_fn))(void *, PN_(entry) *fill);

#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

#include <stdio.h>  /* fprintf FILE */
#include <math.h>   /* sqrt NAN? */
#ifndef NAN /* <https://stackoverflow.com/questions/5714131/nan-literal-in-c> */
#define NAN (0. / 0.)
#endif
#include <string.h> /* memset */

static size_t PN_(count_bucket)(const struct N_(table) *const table,
	PN_(uint) idx) {
	struct PN_(bucket) *bucket;
	PN_(uint) next;
	size_t no = 0;
	assert(table && idx < PN_(capacity)(table));
	bucket = table->buckets + idx;
	if((next = bucket->next) == TABLE_NULL
		|| idx != PN_(to_bucket_no)(table, bucket->hash)) return 0;
	for( ; no++, next != TABLE_END;
		next = bucket->next, assert(next != TABLE_NULL)) {
		idx = next;
		assert(idx < PN_(capacity)(table)
			/* We want to count intermediates.
			 && PN_(in_stack_range)(hash, idx) */);
		bucket = table->buckets + idx;
	}
	return no;
}

/** Mean: `mean`, population variance: `ssdm/n`, sample variance: `ssdm/(n-1)`.
 <Welford1962Note>. */
static struct { size_t n, max; double mean, ssdm; }
	PN_(stats) = { 0, 0, 0.0, 0.0 };
static void PN_(rehash)(void) {
	PN_(stats).n = PN_(stats).max = 0;
	PN_(stats).mean = PN_(stats).ssdm = 0.0;
}
/** Update one sample point of `value`. */
static void PN_(update)(const size_t value) {
	double d, v = (double)value;
	if(PN_(stats).max < value) PN_(stats).max = value;
	d = v - PN_(stats).mean;
	PN_(stats).mean += d / (double)++PN_(stats).n;
	PN_(stats).ssdm += d * (v - PN_(stats).mean);
}
/** Collect stats on `hash`. */
static void PN_(collect)(const struct N_(table) *const table) {
	PN_(uint) i, i_end;
	PN_(rehash)();
	if(!table || !table->buckets) return;
	for(i = 0, i_end = PN_(capacity)(table); i < i_end; i++) {
		size_t no;
		/* I'm sure there's a cheaper way to do it. */
		for(no = PN_(count_bucket)(table, i); no; no--) PN_(update)(no);
	}
}

/** Draw a diagram of `hash` written to `fn` in
 [Graphviz](https://www.graphviz.org/) format.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void PN_(graph)(const struct N_(table) *const table,
	const char *const fn) {
	FILE *fp;
	size_t i, i_end;
	assert(table && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	printf("*** %s\n", fn);
	fprintf(fp, "digraph {\n"
		"\trankdir=LR;\n"
		"\tgraph [truecolor=true, bgcolor=transparent];\n"
		"\tfontface=modern;\n");
	if(!table->buckets) { fprintf(fp, "\tidle [shape=none]\n"); goto end; }
	PN_(collect)(table), assert((size_t)table->size >= PN_(stats).n);
	fprintf(fp,
		"\tnode [shape=box, style=filled, fillcolor=\"Gray95\"];\n"
		"\thash [label=<<TABLE BORDER=\"0\">\n"
		"\t<TR><TD COLSPAN=\"3\" ALIGN=\"LEFT\"><FONT COLOR=\"Gray85\">&lt;"
		QUOTE(TABLE_NAME) "&gt;table: " QUOTE(TABLE_KEY) "</FONT></TD></TR>\n"
		"\t<TR>\n"
		"\t\t<TD>&nbsp;</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">load"
		" factor</TD>\n");
	fprintf(fp,
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">%lu/%lu</TD>\n"
		"\t</TR>\n"
		"\t<TR>\n"
		"\t\t<TD>&nbsp;</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\">E[no bucket]</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\">%.2f(%.1f)</TD>\n"
		"\t</TR>\n"
		"\t<TR>\n"
		"\t\t<TD>&nbsp;</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">max"
		" bucket</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">%lu</TD>\n"
		"\t</TR>\n",
		(unsigned long)PN_(stats).n,
		table->buckets ? (unsigned long)PN_(capacity)(table) : 0,
		PN_(stats).n ? PN_(stats).mean : (double)NAN, PN_(stats).n > 1
		? sqrt(PN_(stats).ssdm / (double)(PN_(stats).n - 1)) : (double)NAN,
		(unsigned long)PN_(stats).max);
	fprintf(fp, "\t<TR>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Italic\">i</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Bold\">hash</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"%s\">key</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Bold\">next</FONT></TD>\n"
		"\t</TR>\n",
#ifdef TABLE_INVERSE
		"Times-Italic"
#else
		"Times-Bold"
#endif
		);
	for(i = 0, i_end = PN_(capacity)(table); i < i_end; i++) {
		const char *const bgc = i & 1 ? "" : " BGCOLOR=\"Gray90\"",
			*const top = (table->top & ~TABLE_HIGH) == i
			? (table->top & TABLE_HIGH) ? " BORDER=\"1\"" : " BORDER=\"2\"" : "";
		struct PN_(bucket) *b = table->buckets + i;
		fprintf(fp, "\t<TR>\n"
			"\t\t<TD ALIGN=\"RIGHT\"%s%s>0x%lx</TD>\n",
			top, bgc, (unsigned long)i);
		if(b->next != TABLE_NULL) {
			const char *const closed
				= PN_(to_bucket_no)(table, b->hash) == i ? "⬤" : "◯";
			char z[12];
			N_(to_string)(PN_(bucket_key)(b), &z);
			fprintf(fp, "\t\t<TD ALIGN=\"RIGHT\"%s>0x%lx</TD>\n"
				"\t\t<TD ALIGN=\"LEFT\"%s>%s</TD>\n"
				"\t\t<TD PORT=\"%lu\"%s>%s</TD>\n",
				bgc, (unsigned long)b->hash,
				bgc, z,
				(unsigned long)i, bgc, closed);
		}
		fprintf(fp, "\t</TR>\n");
	}
	fprintf(fp, "</TABLE>>];\n");
	fprintf(fp, "\tnode [shape=plain, fillcolor=none]\n");
	for(i = 0, i_end = PN_(capacity)(table); i < i_end; i++) {
		struct PN_(bucket) *b = table->buckets + i;
		PN_(uint) left, right;
		if((right = b->next) == TABLE_NULL || right == TABLE_END) continue;
		if(PN_(to_bucket_no)(table, b->hash) != i) {
			fprintf(fp, "\ti0x%lx [label=\"0x%lx\", fontcolor=\"Gray\"];\n"
				"\thash:%lu -> i0x%lx [color=\"Gray\"];\n",
				(unsigned long)right, (unsigned long)right,
				i, (unsigned long)right);
			continue;
		}
		fprintf(fp,
			"\te%lu [label=\"0x%lx\"];\n"
			"\thash:%lu -> e%lu [tailclip=false];\n",
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
static void PN_(histogram)(const struct N_(table) *const table,
	const char *const fn) {
	FILE *fp;
	size_t histogram[64], hs, h;
	const size_t histogram_size = sizeof histogram / sizeof *histogram;
	assert(table && fn);
	memset(histogram, 0, sizeof histogram);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	if(table->buckets) {
		PN_(uint) i, i_end = PN_(capacity)(table);
		for(i = 0; i < i_end; i++) {
			size_t bucket = PN_(count_bucket)(table, i);
			/* The bins are `0,1,2,...,[histogram_size - 1, \infty]`. */
			if(bucket >= histogram_size) bucket = histogram_size - 1;
			histogram[bucket]++;
		}
	}
	/* `historgram_size` is much larger than it has to be, usually. */
	for(hs = histogram_size - 1; !(histogram[hs] && (hs++, 1)) && hs; hs--);
	/* Stats for fit. fixme: This is stats for expected value, I don't think
	 they are the same. */
	PN_(collect)(table);
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
		PN_(stats).mean, (unsigned long)PN_(stats).n);
	for(h = 0; h < hs; h++) fprintf(fp, "%lu\t%lu\n",
		(unsigned long)h, (unsigned long)histogram[h]);
	fclose(fp);
}

/** @return Equality of entries `a` and `b`. */
static int PN_(eq_en)(PN_(entry) a, PN_(entry) b) {
	PN_(key_c) ka = PN_(entry_key)(a), kb = PN_(entry_key)(b);
#ifdef TABLE_INVERSE /* Compare in <typedef:<PN>uint> space. */
	return N_(hash)(ka) == N_(hash)(kb);
#else
	return PN_(equal)(ka, kb);
#endif
}

/** Assertion function for seeing if `hash` is in a valid state.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void PN_(legit)(const struct N_(table) *const table) {
	PN_(uint) i, i_end;
	size_t size = 0, end = 0, start = 0;
	if(!table) return; /* Null. */
	if(!table->buckets) { /* Idle. */
		assert(!table->log_capacity && !table->size);
		return;
	}
	assert(table->log_capacity >= 3);
	for(i = 0, i_end = PN_(capacity)(table); i < i_end; i++) {
		struct PN_(bucket) *b = table->buckets + i;
		if(b->next == TABLE_NULL) continue;
		size++;
		if(b->next == TABLE_END) end++;
		if(i == PN_(to_bucket_no)(table, b->hash)) start++;
	}
	assert(table->size == size && end == start && size >= start);
}

/** Passed `parent_new` and `parent` from <fn:<N>hash_test>. */
static void PN_(test_basic)(void *const parent) {
	struct {
		struct sample {
			PN_(entry) entry;
/*#ifdef TABLE_VALUE
			PN_(value) temp_value;
#endif*/
			int is_in;
		} sample[1000];
		size_t count;
	} trials;
	const size_t trial_size = sizeof trials.sample / sizeof *trials.sample,
		max_graph = ((PN_(uint))~0) > 1000 ? 1000 : ((PN_(uint))~0);
	size_t i;
	PN_(uint) b, b_end;
	char z[12];
	struct N_(table) table = N_(table)();
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
		if(!N_(filler)(parent, &s->entry)) { assert(0); return; }
		N_(to_string)(PN_(entry_key)(s->entry), &z);
		s->is_in = 0;
		for(j = 0; j < i && !PN_(eq_en)(s->entry, trials.sample[j].entry);
			j++);
		if(j == i) s->is_in = 1;
	}
	/* Test idle. */
	PN_(legit)(&table);
	assert(!table.buckets && !table.log_capacity && !table.size);
	PN_(legit)(&table);
	PN_(graph)(&table, "graph/" QUOTE(TABLE_NAME) "-0.gv");
	success = N_(table_buffer)(&table, 1);
	assert(success && table.buckets && table.log_capacity == 3 && !table.size);
	success = N_(table_buffer)(&table, 1);
	assert(success && table.buckets && table.log_capacity == 3 && !table.size);
	N_(table_clear)(&table);
	assert(table.buckets && table.log_capacity == 3 && !table.size);
	{ /* Test negative `get_or`. */
		PN_(key) key = PN_(entry_key)(trials.sample[0].entry);
		PN_(value) ret, def;
		int cmp;
		memset(&def, 1, sizeof def);
		ret = N_(table_get_or)(&table, key, def);
		cmp = memcmp(&ret, &def, sizeof def);
		assert(!cmp);
	}
	N_(table_)(&table);
	assert(!table.buckets && table.log_capacity == 0 && !table.size);
	/* Test placing items. */
	for(i = 0; i < trial_size; i++) {
		struct { PN_(uint) before, after; } size;
		enum table_result result;
		const struct sample *s = trials.sample + i;
		PN_(entry) eject, zero, entry;
		memset(&eject, 0, sizeof eject);
		memset(&zero, 0, sizeof zero);
		size.before = table.size;
		result = N_(table_update)(&table, s->entry, &eject, 0);
		size.after = table.size;
		assert(s->is_in && !memcmp(&eject, &zero, sizeof zero)
			&& result == TABLE_UNIQUE && size.after == size.before + 1
			|| !s->is_in && result == TABLE_YIELD && size.before == size.after);
		success = N_(table_query)(&table, PN_(entry_key)(s->entry), &entry);
		assert(success && PN_(eq_en)(s->entry, entry));
		if(table.size < max_graph && !(i & (i - 1)) || i + 1 == trial_size) {
			char fn[64];
			N_(to_string)(PN_(entry_key)(s->entry), &z);
			printf("%lu. Store \"%s\" result %s, table %s.\n", (unsigned long)i,
				z, table_result_str[result], N_(table_to_string)(&table));
			sprintf(fn, "graph/" QUOTE(TABLE_NAME) "-%lu.gv",
				(unsigned long)i+1);
			PN_(graph)(&table, fn);
		}
		PN_(legit)(&table);
	}
	{
		char fn[64];
		sprintf(fn, "graph/histogram-" QUOTE(TABLE_NAME) "-%u.gnu",
			(unsigned)trial_size);
		PN_(histogram)(&table, fn);
	}
	printf("Go through the table and see if we can get all the items out.\n");
	{ PN_(value) def; memset(&def, 0, sizeof def);
		for(i = 0; i < trial_size; i++) {
		const struct sample *s = trials.sample + i;
		const PN_(key) key = PN_(entry_key)(s->entry);
		PN_(entry) result;
		PN_(value) value;
		const PN_(value) *sample_value;
		int /*cmp,*/ is;
		is = N_(table_is)(&table, key);
		assert(is);
		is = N_(table_query)(&table, key, &result);
		assert(is && PN_(eq_en)(s->entry, result));
		value = N_(table_get_or)(&table, key, def);
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
	printf("Remove:\n");
	{
		struct N_(table_iterator) it;
		PN_(entry) entry;
		/*char fn[64];
		unsigned count = 0;*/
		b = 0;
		for(it = N_(table_begin)(&table); N_(table_next)(&it, &entry); b++);
		assert(b == table.size);
		for(it = N_(table_begin)(&table); N_(table_next)(&it, &entry); ) {
			b++;
			N_(table_remove)(&table, PN_(entry_key)(entry));
			/*sprintf(fn, "graph/" QUOTE(TABLE_NAME) "-end-%u.gv", ++count);
			PN_(graph)(&table, fn);*/
		}
		PN_(graph)(&table, "graph/" QUOTE(TABLE_NAME) "-end.gv");
	}
	/* Clear. */
	N_(table_clear)(&table);
	PN_(graph)(&table, "graph/" QUOTE(TABLE_NAME) "-clear.gv");
	PN_(legit)(&table);
	for(b = 0, b_end = b + PN_(capacity)(&table); b < b_end; b++)
		assert(table.buckets[b].next == TABLE_NULL);
	assert(table.size == 0);
	printf("Clear: %s.\n", N_(table_to_string)(&table));
	for(i = 0; i < trial_size; i++) { /* Make sure to test it again. */
		const struct sample *s = trials.sample + i;
		enum table_result result;
		result = N_(table_update)(&table, s->entry, 0, 0);
		assert(result == TABLE_YIELD || result == TABLE_UNIQUE);
	}
	N_(table_)(&table);
	assert(!table.buckets && !table.log_capacity && !table.size);
}

/** The list will be tested on `stdout`. Requires `TABLE_TEST` to be a
 <typedef:<PN>action_fn> and `TABLE_TO_STRING`.
 @param[parent_new] Specifies the dynamic up-level creator of the parent
 `struct`. Could be null; then testing will be done statically on an array of
 <tag:<N>hashlink> and `TABLE_TEST` is not allowed to go over the limits of the
 data key. @param[parent] The parameter passed to `parent_new`. Ignored if
 `parent_new` is null. @allow */
static void N_(table_test)(void *const parent) {
	printf("<" QUOTE(TABLE_NAME) ">table of key <" QUOTE(TABLE_KEY)
		"> was created using: "
		"TABLE_UINT <" QUOTE(TABLE_UINT) ">; "
		"TABLE_HASH <" QUOTE(TABLE_HASH) ">; "
#ifdef TABLE_IS_EQUAL
		"TABLE_IS_EQUAL <" QUOTE(TABLE_IS_EQUAL) ">; "
#else
		"TABLE_INVERSE <" QUOTE(TABLE_INVERSE) ">; "
#endif
#ifdef TABLE_VALUE
		"TABLE_VALUE <" QUOTE(TABLE_VALUE) ">; "
#endif
		"TABLE_TEST; "
		"testing%s:\n", parent ? "(pointer)" : "");
	PN_(test_basic)(parent);
	fprintf(stderr, "Done tests of <" QUOTE(TABLE_NAME) ">hash.\n\n");
}

#undef QUOTE
#undef QUOTE_

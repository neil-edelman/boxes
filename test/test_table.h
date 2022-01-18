/** A call with the container unknown. This is so that the function is free to
 return a key which is part of a larger aggregate structure. */
typedef int (*const PN_(fill_fn))(void *, PN_(entry) *);

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

static size_t PN_(count_bucket)(const struct N_(set) *const set,
	PN_(uint) idx) {
	struct PN_(bucket) *bucket;
	PN_(uint) next;
	size_t no = 0;
	assert(set && idx < PN_(capacity)(set));
	bucket = set->buckets + idx;
	if((next = bucket->next) == TABLE_NULL
		|| idx != PN_(to_bucket)(set, bucket->hash)) return 0;
	for( ; no++, next != TABLE_END; next = bucket->next, assert(next != TABLE_NULL)) {
		idx = next;
		assert(idx < PN_(capacity)(set)
			/* We want to count intermediates.
			 && PN_(in_stack_range)(hash, idx) */);
		bucket = set->buckets + idx;
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
	double d, v = value;
	if(PN_(stats).max < value) PN_(stats).max = value;
	d = v - PN_(stats).mean;
	PN_(stats).mean += d / ++PN_(stats).n;
	PN_(stats).ssdm += d * (v - PN_(stats).mean);
}
/** Collect stats on `hash`. */
static void PN_(collect)(const struct N_(set) *const set) {
	PN_(uint) i, i_end;
	PN_(rehash)();
	if(!set || !set->buckets) return;
	for(i = 0, i_end = PN_(capacity)(set); i < i_end; i++) {
		size_t no;
		/* I'm sure there's a cheaper way to do it. */
		for(no = PN_(count_bucket)(set, i); no; no--) PN_(update)(no);
	}
}

/** Draw a diagram of `hash` written to `fn` in
 [Graphviz](https://www.graphviz.org/) format.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void PN_(graph)(const struct N_(set) *const set, const char *const fn) {
	FILE *fp;
	size_t i, i_end;
	assert(set && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	printf("*** %s\n", fn);
	fprintf(fp, "digraph {\n"
		"\trankdir=LR;\n"
		"\tgraph [truecolor=true, bgcolor=transparent];\n"
		"\tfontface=modern;\n");
	if(!set->buckets) { fprintf(fp, "\tidle [shape=none]\n"); goto end; }
	PN_(collect)(set), assert((size_t)set->size >= PN_(stats).n /* Buckets. */);
	fprintf(fp,
		"\tnode [shape=box, style=filled, fillcolor=\"Gray95\"];\n"
		"\thash [label=<<TABLE BORDER=\"0\">\n"
		"\t<TR><TD COLSPAN=\"3\" ALIGN=\"LEFT\"><FONT COLOR=\"Gray85\">&lt;"
		QUOTE(TABLE_NAME) "&gt;hash: " QUOTE(TABLE_KEY) "</FONT></TD></TR>\n"
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
		set->buckets ? (unsigned long)PN_(capacity)(set) : 0,
		PN_(stats).n ? PN_(stats).mean : (double)NAN, PN_(stats).n > 1
		? sqrt(PN_(stats).ssdm / (double)(PN_(stats).n - 1)) : (double)NAN,
		(unsigned long)PN_(stats).max);
	fprintf(fp, "\t<TR>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Italic\">i</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"%s\">hash</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"%s\">key</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Bold\">next</FONT></TD>\n"
		"\t</TR>\n",
#ifdef TABLE_NO_CACHE
		"Times-Italic"
#else
		"Times-Bold"
#endif
		,
#ifdef TABLE_INVERSE
		"Times-Italic"
#else
		"Times-Bold"
#endif
		);
	for(i = 0, i_end = PN_(capacity)(set); i < i_end; i++) {
		const char *const bgc = i & 1 ? "" : " BGCOLOR=\"Gray90\"",
			*const top = set->top == i ? " BORDER=\"1\"" : "";
		struct PN_(bucket) *b = set->buckets + i;
		fprintf(fp, "\t<TR>\n"
			"\t\t<TD ALIGN=\"RIGHT\"%s%s>0x%lx</TD>\n",
			top, bgc, (unsigned long)i);
		if(b->next != TABLE_NULL) {
			const char *const closed
				= PN_(to_bucket)(set, b->hash) == i
				? "⬤" : "◯";
			char z[12];
			PN_(to_string)(PN_(bucket_key)(b), &z);
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
	for(i = 0, i_end = PN_(capacity)(set); i < i_end; i++) {
		struct PN_(bucket) *b = set->buckets + i;
		PN_(uint) left, right;
		if((right = b->next) == TABLE_NULL || right == TABLE_END) continue;
		if(PN_(to_bucket)(set, b->hash) != i) {
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
		while(left = right, b = set->buckets + left,
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
static void PN_(histogram)(const struct N_(set) *const set,
	const char *const fn) {
	FILE *fp;
	size_t histogram[64], hs, h;
	const size_t histogram_size = sizeof histogram / sizeof *histogram;
	assert(set && fn);
	memset(histogram, 0, sizeof histogram);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	if(set->buckets) {
		PN_(uint) i, i_end = PN_(capacity)(set);
		for(i = 0; i < i_end; i++) {
			size_t bucket = PN_(count_bucket)(set, i);
			/* The bins are `0,1,2,...,[histogram_size - 1, \infty]`. */
			if(bucket >= histogram_size) bucket = histogram_size - 1;
			histogram[bucket]++;
		}
	}
	/* `historgram_size` is much larger than it has to be, usually. */
	for(hs = histogram_size - 1; !(histogram[hs] && (hs++, 1)) && hs; hs--);
	/* Stats for fit. */
	PN_(collect)(set);
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
		(unsigned long)set->size, fn,
		PN_(stats).mean, (unsigned long)PN_(stats).n);
	for(h = 0; h < hs; h++) fprintf(fp, "%lu\t%lu\n",
		(unsigned long)h, (unsigned long)histogram[h]);
	fclose(fp);
}

/** @return Equality of entries `a` and `b`. */
static int PN_(eq_en)(PN_(entry) a, PN_(entry) b) {
	PN_(ckey) ka = PN_(entry_key)(a), kb = PN_(entry_key)(b);
#ifdef TABLE_INVERSE /* Compare in <typedef:<PN>uint> space. */
	return PN_(hash)(ka) == PN_(hash)(kb);
#else
	/*char sa[12], sb[12];
	PN_(to_string)(ka, &sa);
	PN_(to_string)(kb, &sb);
	printf("return %s, %s == %d\n", sa, sb, PN_(equal)(ka, kb));*/
	return PN_(equal)(ka, kb);
#endif
}

/** Assertion function for seeing if `hash` is in a valid state.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void PN_(legit)(const struct N_(set) *const set) {
	PN_(uint) i, i_end;
	size_t size = 0, end = 0, start = 0;
	if(!set) return; /* Null. */
	if(!set->buckets) { /* Idle. */
		assert(!set->log_capacity && !set->size);
		return;
	}
	assert(set->log_capacity >= 3);
	for(i = 0, i_end = PN_(capacity)(set); i < i_end; i++) {
		struct PN_(bucket) *b = set->buckets + i;
		if(b->next == TABLE_NULL) continue;
		size++;
		if(b->next == TABLE_END) end++;
		if(i == PN_(to_bucket)(set, b->hash)) start++;
	}
	assert(set->size == size && end == start && size >= start);
}

/** Passed `parent_new` and `parent` from <fn:<N>hash_test>. */
static void PN_(test_basic)(const PN_(fill_fn) fill, void *const parent) {
	struct {
		struct sample {
			union {
				void *unused;
				PN_(entry) entry;
			} _;
			int is_in, unused;
		} sample[1000];
		size_t count;
	} trials;
	const size_t trial_size = sizeof trials.sample / sizeof *trials.sample;
	size_t i;
	PN_(uint) b, b_end;
	char z[12];
	struct N_(set) set = TABLE_IDLE;
	int success;
	assert(fill && trial_size > 1);
	/* Pre-computation. O(element_size*(element_size-1)/2); this places a limit
	 on how much a reasonable test is. */
	for(i = 0; i < trial_size; i++) {
		struct sample *s = trials.sample + i;
		size_t j;
		if(!fill(parent, &s->_.entry)) { assert(0); return; }
		PN_(to_string)(PN_(entry_key)(s->_.entry), &z);
		s->is_in = 0;
		for(j = 0; j < i && !PN_(eq_en)(s->_.entry, trials.sample[j]._.entry);
			j++);
		if(j == i) s->is_in = 1;
	}
	/* Test empty. */
	PN_(legit)(&set);
	N_(set)(&set);
	assert(!set.buckets && !set.log_capacity && !set.size);
	PN_(legit)(&set);
	PN_(graph)(&set, "graph/" QUOTE(TABLE_NAME) "-0.gv");
	success = N_(set_buffer)(&set, 1);
	assert(success && set.buckets && set.log_capacity == 3 && !set.size);
	success = N_(set_buffer)(&set, 1);
	assert(success && set.buckets && set.log_capacity == 3 && !set.size);
	N_(set_clear)(&set);
	assert(set.buckets && set.log_capacity == 3 && !set.size);
	{ /* Test negative `get_or`. */
		PN_(key) key = PN_(entry_key)(trials.sample[0]._.entry);
		PN_(value) ret, def;
		int cmp;
		memset(&def, 1, sizeof def);
		ret = N_(set_get_or)(&set, key, def);
		cmp = memcmp(&ret, &def, sizeof def);
		assert(!cmp);
	}
	N_(set_)(&set);
	assert(!set.buckets && set.log_capacity == 0 && !set.size);
	/* Test placing items. */
	for(i = 0; i < trial_size; i++) {
		struct { PN_(uint) before, after; } size;
		enum set_result result;
		const struct sample *s = trials.sample + i;
		PN_(entry) eject, zero, entry;
		memset(&eject, 0, sizeof eject);
		memset(&zero, 0, sizeof zero);
		size.before = set.size;
		PN_(to_string)(PN_(entry_key)(s->_.entry), &z);
		result = N_(set_update)(&set, s->_.entry, &eject, 0);
		printf("Store \"%s\" in set, result: %s.\n", z, set_result_str[result]);
		size.after = set.size;
		assert(s->is_in && !memcmp(&eject, &zero, sizeof zero)
			&& result == TABLE_UNIQUE && size.after == size.before + 1
			|| !s->is_in && result == TABLE_YIELD && size.before == size.after);
		success = N_(set_query)(&set, PN_(entry_key)(s->_.entry), &entry);
		assert(success && PN_(eq_en)(s->_.entry, entry));
		if(set.size < 10000 && !(i & (i - 1)) || i + 1 == trial_size) {
			char fn[64];
			printf("Hash to far: %s.\n", PN_(set_to_string)(&set));
			sprintf(fn, "graph/" QUOTE(TABLE_NAME) "-%lu.gv", (unsigned long)i+1);
			PN_(graph)(&set, fn);
		}
		PN_(legit)(&set);
	}
	{
		char fn[64];
		sprintf(fn, "graph/histogram-" QUOTE(TABLE_NAME) "-%u.gnu",
			(unsigned)trial_size);
		PN_(histogram)(&set, fn);
	}
	printf("Go through the set and see if we can get all the items out.\n");
	{ PN_(value) def; memset(&def, 0, sizeof def);
		for(i = 0; i < trial_size; i++) {
		const struct sample *s = trials.sample + i;
		const PN_(key) key = PN_(entry_key)(s->_.entry);
		PN_(entry) result;
		PN_(value) value;
		const PN_(value) *array_value;
		int is, cmp;
		is = N_(set_is)(&set, key);
		assert(is);
		success = N_(set_query)(&set, key, &result);
		assert(success && PN_(eq_en)(s->_.entry, result));
		value = N_(set_get_or)(&set, key, def);
#ifdef TABLE_VALUE
		array_value = &s->_.entry.value;
#else
		array_value = &s->_.entry;
#endif
		cmp = memcmp(&value, array_value, sizeof value);
		assert(!cmp);
		/* printf("%u: %u\n", (unsigned *)*array_value, (unsigned *)value); */
	}}
	N_(set_clear)(&set);
	for(b = 0, b_end = b + PN_(capacity)(&set); b < b_end; b++)
		assert(set.buckets[b].next == TABLE_NULL);
	assert(set.size == 0);
	printf("Clear: %s.\n", PN_(set_to_string)(&set));
	N_(set_)(&set);
	assert(!set.buckets && !set.log_capacity && !set.size);
}

/** The list will be tested on `stdout`. Requires `TABLE_TEST` to be a
 <typedef:<PN>action_fn> and `TABLE_TO_STRING`.
 @param[parent_new] Specifies the dynamic up-level creator of the parent
 `struct`. Could be null; then testing will be done statically on an array of
 <tag:<N>hashlink> and `TABLE_TEST` is not allowed to go over the limits of the
 data key. @param[parent] The parameter passed to `parent_new`. Ignored if
 `parent_new` is null. @allow */
static void N_(set_test)(const PN_(fill_fn) fill, void *const parent) {
	printf("<" QUOTE(TABLE_NAME) ">hash of key <" QUOTE(TABLE_KEY)
		"> was created using: "
#ifdef TABLE_VALUE
		"TABLE_VALUE <" QUOTE(TABLE_VALUE) ">; "
#endif
		"TABLE_UINT <" QUOTE(TABLE_UINT) ">; "
		"TABLE_HASH <" QUOTE(TABLE_HASH) ">; "
		"TABLE_IS_EQUAL <" QUOTE(TABLE_IS_EQUAL) ">; "
#ifdef TABLE_NO_CACHE
		"TABLE_NO_CACHE; "
#endif
#ifdef TABLE_INVERT
		"TABLE_INVERT; "
#endif
		"TABLE_TEST; "
		"testing%s:\n", parent ? "(pointer)" : "");
	assert(fill);
	PN_(test_basic)(fill, parent);
	fprintf(stderr, "Done tests of <" QUOTE(TABLE_NAME) ">hash.\n\n");
}

#undef QUOTE
#undef QUOTE_

/** A call with the container unknown. This is so that the function is free to
 return a key which is part of a larger aggregate structure. */
typedef int (*const PS_(fill_fn))(void *, PS_(entry) *);

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

static size_t PS_(count_bucket)(const struct S_(set) *const set,
	PS_(uint) idx) {
	struct PS_(bucket) *bucket;
	PS_(uint) next;
	size_t no = 0;
	assert(set && idx < PS_(capacity)(set));
	bucket = set->buckets + idx;
	if((next = bucket->next) == SET_NULL
		|| idx != PS_(to_bucket)(set, PS_(bucket_hash)(bucket))) return 0;
	for( ; no++, next != SET_END; next = bucket->next, assert(next != SET_NULL)) {
		idx = next;
		assert(idx < PS_(capacity)(set)
			/* We want to count intermediates.
			 && PS_(in_stack_range)(hash, idx) */);
		bucket = set->buckets + idx;
	}
	return no;
}

/** Mean: `mean`, population variance: `ssdm/n`, sample variance: `ssdm/(n-1)`.
 <Welford1962Note>. */
static struct { size_t n, max; double mean, ssdm; }
	PS_(stats) = { 0, 0, 0.0, 0.0 };
static void PS_(rehash)(void) {
	PS_(stats).n = PS_(stats).max = 0;
	PS_(stats).mean = PS_(stats).ssdm = 0.0;
}
/** Update one sample point of `value`. */
static void PS_(update)(const size_t value) {
	double d, v = value;
	if(PS_(stats).max < value) PS_(stats).max = value;
	d = v - PS_(stats).mean;
	PS_(stats).mean += d / ++PS_(stats).n;
	PS_(stats).ssdm += d * (v - PS_(stats).mean);
}
/** Collect stats on `hash`. */
static void PS_(collect)(const struct S_(set) *const set) {
	PS_(uint) i, i_end;
	PS_(rehash)();
	if(!set || !set->buckets) return;
	for(i = 0, i_end = PS_(capacity)(set); i < i_end; i++) {
		size_t no;
		/* I'm sure there's a cheaper way to do it. */
		for(no = PS_(count_bucket)(set, i); no; no--) PS_(update)(no);
	}
}

/** Draw a diagram of `hash` written to `fn` in
 [Graphviz](https://www.graphviz.org/) format.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void PS_(graph)(const struct S_(set) *const set, const char *const fn) {
	FILE *fp;
	size_t i, i_end;
	assert(set && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	printf("*** %s\n", fn);
	fprintf(fp, "digraph {\n"
		"\trankdir=LR;\n"
		"\tfontface=modern;\n");
	if(!set->buckets) { fprintf(fp, "\tidle [shape=none]\n"); goto end; }
	PS_(collect)(set), assert((size_t)set->size >= PS_(stats).n /* Buckets. */);
	fprintf(fp,
		"\tnode [shape=box, style=filled, fillcolor=\"Gray95\"];\n"
		"\thash [label=<<TABLE BORDER=\"0\">\n"
		"\t<TR><TD COLSPAN=\"3\" ALIGN=\"LEFT\"><FONT COLOR=\"Gray85\">&lt;"
		QUOTE(SET_NAME) "&gt;hash: " QUOTE(SET_KEY) "</FONT></TD></TR>\n"
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
		(unsigned long)PS_(stats).n,
		set->buckets ? (unsigned long)PS_(capacity)(set) : 0,
		PS_(stats).n ? PS_(stats).mean : (double)NAN, PS_(stats).n > 1
		? sqrt(PS_(stats).ssdm / (double)(PS_(stats).n - 1)) : (double)NAN,
		(unsigned long)PS_(stats).max);
	fprintf(fp, "\t<TR>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Italic\">i</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"%s\">hash</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"%s\">key</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Bold\">next</FONT></TD>\n"
		"\t</TR>\n",
#ifdef SET_NO_CACHE
		"Times-Italic"
#else
		"Times-Bold"
#endif
		,
#ifdef SET_INVERSE
		"Times-Italic"
#else
		"Times-Bold"
#endif
		);
	for(i = 0, i_end = PS_(capacity)(set); i < i_end; i++) {
		const char *const bgc = i & 1 ? "" : " BGCOLOR=\"Gray90\"",
			*const top = set->top == i ? " BORDER=\"1\"" : "";
		struct PS_(bucket) *b = set->buckets + i;
		fprintf(fp, "\t<TR>\n"
			"\t\t<TD ALIGN=\"RIGHT\"%s%s>0x%lx</TD>\n",
			top, bgc, (unsigned long)i);
		if(b->next != SET_NULL) {
			const char *const closed
				= PS_(to_bucket)(set, PS_(bucket_hash)(b)) == i
				? "⬤" : "◯";
			char z[12];
			PS_(to_string)(PS_(bucket_key)(b), &z);
			fprintf(fp, "\t\t<TD ALIGN=\"RIGHT\"%s>0x%lx</TD>\n"
				"\t\t<TD ALIGN=\"LEFT\"%s>%s</TD>\n"
				"\t\t<TD PORT=\"%lu\"%s>%s</TD>\n",
				bgc, (unsigned long)PS_(bucket_hash)(b),
				bgc, z,
				(unsigned long)i, bgc, closed);
		}
		fprintf(fp, "\t</TR>\n");
	}
	fprintf(fp, "</TABLE>>];\n");
	fprintf(fp, "\tnode [shape=plain, fillcolor=none]\n");
	for(i = 0, i_end = PS_(capacity)(set); i < i_end; i++) {
		struct PS_(bucket) *b = set->buckets + i;
		PS_(uint) left, right;
		if((right = b->next) == SET_NULL || right == SET_END) continue;
		if(PS_(to_bucket)(set, PS_(bucket_hash)(b)) != i) {
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
			(right = b->next) != SET_END) {
			assert(right != SET_NULL);
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
static void PS_(histogram)(const struct S_(set) *const set,
	const char *const fn) {
	FILE *fp;
	size_t histogram[64], hs, h;
	const size_t histogram_size = sizeof histogram / sizeof *histogram;
	assert(set && fn);
	memset(histogram, 0, sizeof histogram);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	if(set->buckets) {
		PS_(uint) i, i_end = PS_(capacity)(set);
		for(i = 0; i < i_end; i++) {
			size_t bucket = PS_(count_bucket)(set, i);
			/* The bins are `0,1,2,...,[histogram_size - 1, \infty]`. */
			if(bucket >= histogram_size) bucket = histogram_size - 1;
			histogram[bucket]++;
		}
	}
	/* `historgram_size` is much larger than it has to be, usually. */
	for(hs = histogram_size - 1; !(histogram[hs] && (hs++, 1)) && hs; hs--);
	/* Stats for fit. */
	PS_(collect)(set);
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
		PS_(stats).mean, (unsigned long)PS_(stats).n);
	for(h = 0; h < hs; h++) fprintf(fp, "%lu\t%lu\n",
		(unsigned long)h, (unsigned long)histogram[h]);
	fclose(fp);
}

/** @return Equality of entries `a` and `b`. */
static int PS_(eq_en)(PS_(entry) a, PS_(entry) b) {
	PS_(ckey) ka = PS_(entry_key)(a), kb = PS_(entry_key)(b);
#ifdef SET_INVERSE /* Compare in <typedef:<PS>uint> space. */
	return PS_(hash)(ka) == PS_(hash)(kb);
#else
	/*char sa[12], sb[12];
	PS_(to_string)(ka, &sa);
	PS_(to_string)(kb, &sb);
	printf("return %s, %s == %d\n", sa, sb, PS_(equal)(ka, kb));*/
	return PS_(equal)(ka, kb);
#endif
}

/** Assertion function for seeing if `hash` is in a valid state.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void PS_(legit)(const struct S_(set) *const set) {
	PS_(uint) i, i_end;
	size_t size = 0, end = 0, start = 0;
	if(!set) return; /* Null. */
	if(!set->buckets) { /* Idle. */
		assert(!set->log_capacity && !set->size);
		return;
	}
	assert(set->log_capacity >= 3);
	for(i = 0, i_end = PS_(capacity)(set); i < i_end; i++) {
		struct PS_(bucket) *b = set->buckets + i;
		if(b->next == SET_NULL) continue;
		size++;
		if(b->next == SET_END) end++;
		if(i == PS_(to_bucket)(set, PS_(bucket_hash)(b))) start++;
	}
	assert(set->size == size && end == start && size >= start);
}

/** Passed `parent_new` and `parent` from <fn:<S>hash_test>. */
static void PS_(test_basic)(const PS_(fill_fn) fill, void *const parent) {
	struct {
		struct sample {
			union {
				void *unused;
				PS_(entry) entry;
			} _;
			int is_in, unused;
		} sample[1000];
		size_t count;
	} trials;
	const size_t trial_size = sizeof trials.sample / sizeof *trials.sample;
	size_t i;
	PS_(uint) b, b_end;
	char z[12];
	struct S_(set) set = SET_IDLE;
	int success;
	assert(fill && trial_size > 1);
	/* Pre-computation. O(element_size*(element_size-1)/2); this places a limit
	 on how much a reasonable test is. */
	for(i = 0; i < trial_size; i++) {
		struct sample *s = trials.sample + i;
		size_t j;
		if(!fill(parent, &s->_.entry)) { assert(0); return; }
		PS_(to_string)(PS_(entry_key)(s->_.entry), &z);
		s->is_in = 0;
		for(j = 0; j < i && !PS_(eq_en)(s->_.entry, trials.sample[j]._.entry);
			j++);
		if(j == i) s->is_in = 1;
	}
	/* Test empty. */
	PS_(legit)(&set);
	S_(set)(&set);
	assert(!set.buckets && !set.log_capacity && !set.size);
	PS_(legit)(&set);
	PS_(graph)(&set, "graph/" QUOTE(SET_NAME) "-0.gv");
	success = S_(set_buffer)(&set, 1);
	assert(success && set.buckets && set.log_capacity == 3 && !set.size);
	success = S_(set_buffer)(&set, 1);
	assert(success && set.buckets && set.log_capacity == 3 && !set.size);
	S_(set_clear)(&set);
	assert(set.buckets && set.log_capacity == 3 && !set.size);
	S_(set_)(&set);
	assert(!set.buckets && set.log_capacity == 0 && !set.size);
	/* Test placing items. */
	for(i = 0; i < trial_size; i++) {
		struct { PS_(uint) before, after; } size;
		enum set_result result;
		const struct sample *s = trials.sample + i;
		PS_(entry) eject, zero, entry;
		memset(&eject, 0, sizeof eject);
		memset(&zero, 0, sizeof zero);
		size.before = set.size;
		PS_(to_string)(PS_(entry_key)(s->_.entry), &z);
		result = S_(set_policy_put)(&set, s->_.entry, &eject, 0);
		printf("Store \"%s\" in set, result: %s.\n", z, set_result_str[result]);
		size.after = set.size;
		assert(s->is_in && !memcmp(&eject, &zero, sizeof zero)
			&& result == SET_GROW && size.after == size.before + 1
			|| !s->is_in && result == SET_YIELD && size.before == size.after);
		success = S_(set_query)(&set, PS_(entry_key)(s->_.entry), &entry);
		assert(success && PS_(eq_en)(s->_.entry, entry));
		if(set.size < 10000 && !(i & (i - 1)) || i + 1 == trial_size) {
			char fn[64];
			printf("Hash to far: %s.\n", PS_(set_to_string)(&set));
			sprintf(fn, "graph/" QUOTE(SET_NAME) "-%lu.gv", (unsigned long)i+1);
			PS_(graph)(&set, fn);
		}
		PS_(legit)(&set);
	}
	{
		char fn[64];
		sprintf(fn, "graph/histogram-" QUOTE(SET_NAME) "-%u.gnu",
			(unsigned)trial_size);
		PS_(histogram)(&set, fn);
	}
	printf("Go through the set and see if we can get all the items out.\n");
	for(i = 0; i < trial_size; i++) {
		const struct sample *s = trials.sample + i;
		PS_(entry) result;
		success = S_(set_query)(&set, PS_(entry_key)(s->_.entry), &result);
		assert(success && PS_(eq_en)(s->_.entry, result));
	}
	S_(set_clear)(&set);
	for(b = 0, b_end = b + PS_(capacity)(&set); b < b_end; b++)
		assert(set.buckets[b].next == SET_NULL);
	assert(set.size == 0);
	printf("Clear: %s.\n", PS_(set_to_string)(&set));
	S_(set_)(&set);
	assert(!set.buckets && !set.log_capacity && !set.size);
}

/** The list will be tested on `stdout`. Requires `SET_TEST` to be a
 <typedef:<PS>action_fn> and `SET_TO_STRING`.
 @param[parent_new] Specifies the dynamic up-level creator of the parent
 `struct`. Could be null; then testing will be done statically on an array of
 <tag:<S>hashlink> and `SET_TEST` is not allowed to go over the limits of the
 data key. @param[parent] The parameter passed to `parent_new`. Ignored if
 `parent_new` is null. @allow */
static void S_(set_test)(const PS_(fill_fn) fill, void *const parent) {
	printf("<" QUOTE(SET_NAME) ">hash of key <" QUOTE(SET_KEY)
		"> was created using: "
#ifdef SET_VALUE
		"SET_VALUE <" QUOTE(SET_VALUE) ">; "
#endif
		"SET_UINT <" QUOTE(SET_UINT) ">; "
		"SET_HASH <" QUOTE(SET_HASH) ">; "
		"SET_IS_EQUAL <" QUOTE(SET_IS_EQUAL) ">; "
#ifdef SET_NO_CACHE
		"SET_NO_CACHE; "
#endif
#ifdef SET_INVERT
		"SET_INVERT; "
#endif
		"SET_TEST; "
		"testing%s:\n", parent ? "(pointer)" : "");
	assert(fill);
	PS_(test_basic)(fill, parent);
	fprintf(stderr, "Done tests of <" QUOTE(SET_NAME) ">hash.\n\n");
}

#undef QUOTE
#undef QUOTE_

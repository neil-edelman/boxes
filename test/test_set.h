/** A call with the container unknown. This is so that the function is free to
 return a key which is part of a larger aggregate structure. */
typedef PS_(map) (*const PS_(parent_new_fn))(void *);

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

static size_t PS_(count_bucket)(const struct S_(set) *const hash,
	PS_(uint) idx) {
	struct PS_(bucket) *bucket;
	PS_(uint) next;
	size_t no = 0;
	assert(hash && idx < PS_(capacity)(hash));
	bucket = hash->buckets + idx;
	if((next = bucket->next) == SET_NULL
		|| idx != PS_(code_to_entry)(hash, PS_(entry_code)(bucket))) return 0;
	for( ; no++, next != SET_END; next = bucket->next, assert(next != SET_NULL)) {
		idx = next;
		assert(idx < PS_(capacity)(hash)
			/* We want to count intermediates.
			 && PS_(in_stack_range)(hash, idx) */);
		bucket = hash->buckets + idx;
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
static void PS_(collect)(const struct S_(set) *const hash) {
	PS_(uint) idx, idx_end;
	PS_(rehash)();
	if(!hash || !hash->buckets) return;
	for(idx = 0, idx_end = PS_(capacity)(hash); idx < idx_end; idx++) {
		size_t no;
		/* I'm sure there's a cheaper way to do it. */
		for(no = PS_(count_bucket)(hash, idx); no; no--) PS_(update)(no);
	}
}

/** Draw a diagram of `hash` written to `fn` in
 [Graphviz](https://www.graphviz.org/) format.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void PS_(graph)(const struct S_(set) *const hash, const char *const fn) {
	FILE *fp;
	size_t i, i_end;
	assert(hash && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	printf("*** %s\n", fn);
	fprintf(fp, "digraph {\n"
		"\trankdir=LR;\n"
		"\tfontface=modern;\n");
	if(!hash->buckets) { fprintf(fp, "\tidle [shape=none]\n"); goto end; }
	PS_(collect)(hash), assert((size_t)hash->size >= PS_(stats).n /* Buckets. */);
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
		hash->buckets ? (unsigned long)PS_(capacity)(hash) : 0,
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
	for(i = 0, i_end = 1 << hash->log_capacity; i < i_end; i++) {
		const char *const bgc = i & 1 ? "" : " BGCOLOR=\"Gray90\"",
			*const top = hash->top == i ? " BORDER=\"1\"" : "";
		struct PS_(bucket) *e = hash->buckets + i;
		fprintf(fp, "\t<TR>\n"
			"\t\t<TD ALIGN=\"RIGHT\"%s%s>0x%lx</TD>\n",
			top, bgc, (unsigned long)i);
		if(e->next != SET_NULL) {
			const char *const closed
				= PS_(code_to_entry)(hash, PS_(entry_code)(e)) == i
				? "⬤" : "◯";
			char z[12];
			PS_(to_string)(PS_(entry_key)(e), &z);
			fprintf(fp, "\t\t<TD ALIGN=\"RIGHT\"%s>0x%lx</TD>\n"
				"\t\t<TD ALIGN=\"LEFT\"%s>%s</TD>\n"
				"\t\t<TD PORT=\"%lu\"%s>%s</TD>\n",
				bgc, (unsigned long)PS_(entry_code)(e),
				bgc, z,
				(unsigned long)i, bgc, closed);
		}
		fprintf(fp, "\t</TR>\n");
	}
	fprintf(fp, "</TABLE>>];\n");
	fprintf(fp, "\tnode [shape=plain, fillcolor=none]\n");
	for(i = 0, i_end = 1 << hash->log_capacity; i < i_end; i++) {
		struct PS_(bucket) *e = hash->buckets + i;
		PS_(uint) left, right;
		if((right = e->next) == SET_NULL || right == SET_END) continue;
		if(PS_(code_to_entry)(hash, PS_(entry_code)(e)) != i) {
			fprintf(fp, "\thash:%lu -> i0x%lx;\n", i, (unsigned long)right);
			continue;
		}
		fprintf(fp,
			"\te%lu [label=\"0x%lx\"];\n"
			"\thash:%lu -> e%lu [tailclip=false];\n",
			(unsigned long)right, (unsigned long)right,
			(unsigned long)i, (unsigned long)right);
		while(left = right, e = hash->buckets + left,
			(right = e->next) != SET_END) {
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
static void PS_(histogram)(const struct S_(set) *const hash,
	const char *const fn) {
	FILE *fp;
	size_t histogram[64], hs, h;
	const size_t histogram_size = sizeof histogram / sizeof *histogram;
	assert(hash && fn);
	memset(histogram, 0, sizeof histogram);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	if(hash->buckets) {
		PS_(uint) i, i_end = PS_(capacity)(hash);
		for(i = 0; i < i_end; i++) {
			size_t bucket = PS_(count_bucket)(hash, i);
			/* The bins are `0,1,2,...,[histogram_size - 1, \infty]`. */
			if(bucket >= histogram_size) bucket = histogram_size - 1;
			histogram[bucket]++;
		}
	}
	/* `historgram_size` is much larger than it has to be, usually. */
	for(hs = histogram_size - 1; !(histogram[hs] && (hs++, 1)) && hs; hs--);
	/* Stats for fit. */
	PS_(collect)(hash);
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
		(unsigned long)hash->size, fn,
		PS_(stats).mean, (unsigned long)PS_(stats).n);
	for(h = 0; h < hs; h++) fprintf(fp, "%lu\t%lu\n",
		(unsigned long)h, (unsigned long)histogram[h]);
	fclose(fp);
}

/** Assertion function for seeing if `hash` is in a valid state.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void PS_(legit)(const struct S_(set) *const hash) {
	PS_(uint) i, i_end;
	size_t size = 0, end = 0, start = 0;
	if(!hash) return; /* Null. */
	if(!hash->buckets) { /* Idle. */
		assert(!hash->log_capacity && !hash->size);
		return;
	}
	assert(hash->log_capacity >= 3);
	for(i = 0, i_end = PS_(capacity)(hash); i < i_end; i++) {
		struct PS_(bucket) *e = hash->buckets + i;
		if(e->next == SET_NULL) continue;
		size++;
		if(e->next == SET_END) end++;
		if(i == PS_(code_to_entry)(hash, PS_(entry_code)(e))) start++;
	}
	assert(hash->size == size && end == start && size >= start);
}

/** Passed `parent_new` and `parent` from <fn:<S>hash_test>. */
static void PS_(test_basic)(PS_(key) (*const parent_new)(void *),
	void *const parent) {
	struct test { PS_(map) data; int is_in; }
		test[1000/*0*/], *t;
	const size_t test_size = sizeof test / sizeof *test;
	size_t i;
	char z[12];
	struct S_(set) set = SET_IDLE;
	assert(parent_new
		/*&& parent static tests are possible*/ && test_size > 1);
	/* Test empty. */
	PS_(legit)(&set);
	S_(set)(&set);
	assert(!set.buckets && !set.log_capacity && !set.size);
	PS_(legit)(&set);
	PS_(graph)(&set, "graph/" QUOTE(SET_NAME) "-0.gv");
	/* Test placing items. */
	for(i = 0; i < test_size; i++) {
		struct { PS_(uint) before, after; } size;
		int is_grow;
		int ret;
		t = test + i;
		PS_(map) eject, item;
		t->data = parent_new(parent); /* fixme: Completely unchecked! */
		PS_(to_string)(PS_(map_key)(t->data), &z);
		printf("%lu: came up with %s.\n", (unsigned long)i, z);
		/*success = S_(set_buffer)(&hash, 1);
		assert(success && hash.buckets);*/
		memset(&eject, 0, sizeof eject);
		memset(&item, 0, sizeof item);
		size.before = set.size;
		ret = S_(set_policy_put)(&set, t->data, &eject, 0);
		assert(ret && (i || set.size == 1
			&& !memcmp(&eject, &item, sizeof item)));
		size.after = set.size;
		assert(size.before == size.after || size.after == size.before + 1);
		is_grow = !!(size.after - size.before);
		ret = S_(set_query)(&set, PS_(map_key)(t->data), &item);
		assert(ret && PS_(equal)(t->data, item));
		/* If it replaced, `eject` must be equal to `data`. */
		assert(is_grow || PS_(equal)(t->data, eject));
		if(set.size < 10000 && !(i & (i - 1))) {
			char fn[64];
			printf("*** hash %s.\n", PS_(set_to_string)(&set));
			sprintf(fn, "graph/" QUOTE(SET_NAME) "-%lu.gv", (unsigned long)i);
			PS_(graph)(&set, fn);
		}
		PS_(legit)(&set);
	}
	{
		char fn[64];
		printf("\n");
		sprintf(fn, "graph/" QUOTE(SET_NAME) "-%u-final.gv",
			(unsigned)test_size);
		PS_(graph)(&set, fn);
		sprintf(fn, "graph/histogram-" QUOTE(SET_NAME) "-%u.gnu",
			(unsigned)test_size);
		PS_(histogram)(&set, fn);
	}
	printf("Testing get from hash.\n");
	/* This is more debug info.
	printf("[ ");
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		PS_(to_string)(&t->elem->data, &a);
		printf("%s[%lu-%s]%s", t == test ? "" : ", ",
			(unsigned long)(t - test), t->is_in ? "yes" : "no", a);
	}
	printf(" ]\n");*/
#if 0
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		const size_t n = (size_t)(t - test);
		struct S_(setlink) *r;
		if(!(n & (n - 1))) {
			PS_(to_string)(&t->elem->key, &a);
			fprintf(stderr, "%lu: retrieving %s.\n", (unsigned long)n, a);
		}
		element = S_(set_get)(&hash, PS_(pointer)(&t->elem->key));
		assert(element);
		if(t->is_in) {
			assert(element == t->elem);
			if(rand() < RAND_MAX / 8) {
				removed++;
				r = S_(set_remove)(&hash, PS_(pointer)(&t->elem->key));
				assert(r);
				r = S_(set_remove)(&hash, PS_(pointer)(&t->elem->key));
				assert(!r);
				r = S_(set_policy_put)(&hash, t->elem, 0);
				assert(!r);
				r = S_(set_policy_put)(&hash, t->elem, 0);
				assert(r == t->elem);
				r = S_(set_remove)(&hash, PS_(pointer)(&t->elem->key));
				assert(r);
			}
		} else {
			const size_t count = hash.size;
			collision++;
			assert(t && element != t->elem);
			r = S_(set_policy_put)(&hash, t->elem, 0);
			assert(r == t->elem && count == hash.size);
		}
	}
	printf("Collisions: %lu; removed: %lu.\n",
		(unsigned long)collision, (unsigned long)removed);
	PS_(legit)(&hash);
	PS_(stats)(&hash, "\n", stdout);
	S_(set_clear)(&hash);
	for(b = hash.buckets, b_end = b + (1 << hash.log_capacity); b < b_end; b++)
		assert(!PS_(count)(b));
	assert(hash.size == 0);
#endif
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
static void S_(set_test)(const PS_(parent_new_fn) parent_new,
	void *const parent) {
	printf("<" QUOTE(SET_NAME) ">hash of key <" QUOTE(SET_KEY)
		"> was created using: "
#ifdef SET_VALUE
		"SET_VALUE <" QUOTE(SET_VALUE) ">; "
#endif
		"SET_UINT <" QUOTE(SET_UINT) ">; "
		"SET_CODE <" QUOTE(SET_CODE) ">; "
		"SET_IS_EQUAL <" QUOTE(SET_IS_EQUAL) ">; "
#ifdef SET_NO_CACHE
		"SET_NO_CACHE; "
#endif
#ifdef SET_INVERT
		"SET_INVERT; "
#endif
		"SET_TEST; "
		"testing%s:\n", parent ? "(pointer)" : "");
	PS_(test_basic)(parent_new, parent);
	fprintf(stderr, "Done tests of <" QUOTE(SET_NAME) ">hash.\n\n");
}

#undef QUOTE
#undef QUOTE_

#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

#include <stddef.h> /* offhashof */
#include <stdio.h>  /* fprintf FILE */
#include <math.h>   /* sqrt NAN? */
#ifndef NAN /* <https://stackoverflow.com/questions/5714131/nan-literal-in-c> */
#define NAN (0. / 0.)
#endif
#include <string.h> /* memset */

static size_t PM_(count_bucket)(const struct M_(hash) *const hash,
	PM_(uint) idx) {
	struct PM_(entry) *entry;
	PM_(uint) next;
	size_t no = 0;
	assert(hash && idx < PM_(capacity)(hash));
	entry = hash->entries + idx;
	if((next = entry->next) == SETnull
		|| idx != PM_(code_to_bucket)(hash, PM_(entry_code)(entry))) return 0;
	for( ; no++, next != SETend; next = entry->next, assert(next != SETnull)) {
		idx = next;
		assert(idx < PM_(capacity)(hash)
			/* We want to count intermediates.
			 && PM_(in_stack_range)(hash, idx) */);
		entry = hash->entries + idx;
	}
	return no;
}

/** Mean: `mean`, population variance: `ssdm/n`, sample variance: `ssdm/(n-1)`.
 <Welford1962Note>. */
static struct { size_t n, max; double mean, ssdm; }
	PM_(stats) = { 0, 0, 0.0, 0.0 };
static void PM_(rehash)(void) {
	PM_(stats).n = PM_(stats).max = 0;
	PM_(stats).mean = PM_(stats).ssdm = 0.0;
}
/** Update one sample point of `value`. */
static void PM_(update)(const size_t value) {
	double d, v = value;
	if(PM_(stats).max < value) PM_(stats).max = value;
	d = v - PM_(stats).mean;
	PM_(stats).mean += d / ++PM_(stats).n;
	PM_(stats).ssdm += d * (v - PM_(stats).mean);
}
/** Collect stats on `hash`. */
static void PM_(collect)(const struct M_(hash) *const hash) {
	PM_(uint) idx, idx_end;
	PM_(rehash)();
	if(!hash || !hash->entries) return;
	for(idx = 0, idx_end = PM_(capacity)(hash); idx < idx_end; idx++) {
		size_t no;
		/* I'm sure there's a cheaper way to do it. */
		for(no = PM_(count_bucket)(hash, idx); no; no--) PM_(update)(no);
	}
}

/** Draw a diagram of `hash` written to `fn` in
 [Graphviz](https://www.graphviz.org/) format.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void PM_(graph)(const struct M_(hash) *const hash, const char *const fn) {
	FILE *fp;
	size_t i, i_end;
	assert(hash && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	printf("*** %s\n", fn);
	fprintf(fp, "digraph {\n"
		"\trankdir=LR;\n"
		"\tfontface=modern;\n");
	if(!hash->entries) { fprintf(fp, "\tidle [shape=none]\n"); goto end; }
	PM_(collect)(hash), assert((size_t)hash->size >= PM_(stats).n /* Buckets. */);
	fprintf(fp,
		"\tnode [shape=box, style=filled, fillcolor=\"Gray95\"];\n"
		"\thash [label=<<TABLE BORDER=\"0\">\n"
		"\t<TR><TD COLSPAN=\"3\" ALIGN=\"LEFT\"><FONT COLOR=\"Gray85\">&lt;"
		QUOTE(HASH_NAME) "&gt;hash: " QUOTE(HASH_KEY) "</FONT></TD></TR>\n"
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
		(unsigned long)PM_(stats).n,
		hash->entries ? (unsigned long)PM_(capacity)(hash) : 0,
		PM_(stats).n ? PM_(stats).mean : (double)NAN, PM_(stats).n > 1
		? sqrt(PM_(stats).ssdm / (double)(PM_(stats).n - 1)) : (double)NAN,
		(unsigned long)PM_(stats).max);
	fprintf(fp, "\t<TR>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Italic\">i</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"%s\">hash</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"%s\">key</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Bold\">next</FONT></TD>\n"
		"\t</TR>\n",
#ifdef HASH_NO_CACHE
		"Times-Italic"
#else
		"Times-Bold"
#endif
		,
#ifdef HASH_INVERSE
		"Times-Italic"
#else
		"Times-Bold"
#endif
		);
	for(i = 0, i_end = 1 << hash->log_capacity; i < i_end; i++) {
		const char *const bgc = i & 1 ? "" : " BGCOLOR=\"Gray90\"",
			*const top = hash->top == i ? " BORDER=\"1\"" : "";
		struct PM_(entry) *e = hash->entries + i;
		fprintf(fp, "\t<TR>\n"
			"\t\t<TD ALIGN=\"RIGHT\"%s%s>0x%lx</TD>\n",
			top, bgc, (unsigned long)i);
		if(e->next != SETnull) {
			const char *const closed
				= PM_(code_to_bucket)(hash, PM_(entry_code)(e)) == i
				? "⬤" : "◯";
			char z[12];
			PM_(to_string)(PM_(entry_key)(e), &z);
			fprintf(fp, "\t\t<TD ALIGN=\"RIGHT\"%s>0x%lx</TD>\n"
				"\t\t<TD ALIGN=\"LEFT\"%s>%s</TD>\n"
				"\t\t<TD PORT=\"%lu\"%s>%s</TD>\n",
				bgc, (unsigned long)PM_(entry_code)(e),
				bgc, z,
				(unsigned long)i, bgc, closed);
		}
		fprintf(fp, "\t</TR>\n");
	}
	fprintf(fp, "</TABLE>>];\n");
	fprintf(fp, "\tnode [shape=plain, fillcolor=none]\n");
	for(i = 0, i_end = 1 << hash->log_capacity; i < i_end; i++) {
		struct PM_(entry) *e = hash->entries + i;
		PM_(uint) left, right;
		if((right = e->next) == SETnull || right == SETend) continue;
		if(PM_(code_to_bucket)(hash, PM_(entry_code)(e)) != i) {
			fprintf(fp, "\thash:%lu -> i0x%lx;\n", i, (unsigned long)right);
			continue;
		}
		fprintf(fp,
			"\te%lu [label=\"0x%lx\"];\n"
			"\thash:%lu -> e%lu [tailclip=false];\n",
			(unsigned long)right, (unsigned long)right,
			(unsigned long)i, (unsigned long)right);
		while(left = right, e = hash->entries + left,
			(right = e->next) != SETend) {
			assert(right != SETnull);
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
static void PM_(histogram)(const struct M_(hash) *const hash,
	const char *const fn) {
	FILE *fp;
	size_t histogram[64], hs, h;
	const size_t histogram_size = sizeof histogram / sizeof *histogram;
	assert(hash && fn);
	memset(histogram, 0, sizeof histogram);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	if(hash->entries) {
		PM_(uint) i, i_end = PM_(capacity)(hash);
		for(i = 0; i < i_end; i++) {
			size_t bucket = PM_(count_bucket)(hash, i);
			/* The bins are `0,1,2,...,[histogram_size - 1, \infty]`. */
			if(bucket >= histogram_size) bucket = histogram_size - 1;
			histogram[bucket]++;
		}
	}
	/* `historgram_size` is much larger than it has to be, usually. */
	for(hs = histogram_size - 1; !(histogram[hs] && (hs++, 1)) && hs; hs--);
	/* Stats for fit. */
	PM_(collect)(hash);
	fprintf(fp, "# Size: %lu.\n"
		"hash term postscript eps enhanced color\n"
		"hash output \"%s.eps\"\n"
		"hash grid\n"
		"hash xlabel \"bucket occupancy\"\n"
		"hash ylabel \"frequency\"\n"
		"hash style histogram\n"
		"hash xrange [0:]\n"
		"unhash key\n"
		"poisson(x) = entries*lambda**x/int(x)!*exp(-lambda)\n"
		"lambda = %f\n"
		"entries = %lu\n"
		"# what I really want is the Gamma distribution\n"
		"plot \"-\" using ($1+0.5):2 with boxes lw 3 title \"Histogram\", \\\n"
		"\tpoisson(int(x)) with lines linestyle 2 title \"Fit\"\n",
		(unsigned long)hash->size, fn,
		PM_(stats).mean, (unsigned long)PM_(stats).n);
	for(h = 0; h < hs; h++) fprintf(fp, "%lu\t%lu\n",
		(unsigned long)h, (unsigned long)histogram[h]);
	fclose(fp);
}

/** Assertion function for seeing if `hash` is in a valid state.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void PM_(legit)(const struct M_(hash) *const hash) {
	PM_(uint) i, i_end;
	size_t size = 0, end = 0, start = 0;
	if(!hash) return; /* Null. */
	if(!hash->entries) { /* Idle. */
		assert(!hash->log_capacity && !hash->size);
		return;
	}
	assert(hash->log_capacity >= 3);
	for(i = 0, i_end = PM_(capacity)(hash); i < i_end; i++) {
		struct PM_(entry) *e = hash->entries + i;
		if(e->next == SETnull) continue;
		size++;
		if(e->next == SETend) end++;
		if(i == PM_(code_to_bucket)(hash, PM_(entry_code)(e))) start++;
	}
	assert(hash->size == size && end == start && size >= start);
}

/** Passed `parent_new` and `parent` from <fn:<M>hash_test>. */
static void PM_(test_basic)(PM_(key) (*const parent_new)(void *),
	void *const parent) {
	struct test { PM_(key) data; int is_in; }
		test[1000/*0*/], *t;
	const size_t test_size = sizeof test / sizeof *test;
	size_t i;
	char z[12];
	struct M_(hash) hash = HASH_IDLE;
	PM_(key) eject, item;
	assert(parent_new
		/*&& parent static tests are possible*/ && test_size > 1);
	/* Test empty. */
	PM_(legit)(&hash);
	M_(hash)(&hash);
	assert(!hash.entries && !hash.log_capacity && !hash.size);
	PM_(legit)(&hash);
	PM_(graph)(&hash, "graph/" QUOTE(HASH_NAME) "-0.gv");
	/* Test placing items. */
	for(i = 0; i < test_size; i++) {
		struct { PM_(uint) before, after; } size;
		int is_grow;
		t = test + i;
		/*if(!(t->data = parent_new(parent))) { assert(0); return; }*/
		t->data = parent_new(parent); /* fixme: Completely unchecked! */
		PM_(to_string)(t->data, &z);
		printf("%lu: came up with %s.\n", (unsigned long)i, z);
		/*success = M_(hash_reserve)(&hash, 1);
		assert(success && hash.entries);
		if(n == 0) assert(hash.log_capacity == 3 && !hash.size
			&& !hash.entries[0].first && !hash.entries[1].first
			&& !hash.entries[2].first && !hash.entries[3].first
			&& !hash.entries[4].first && !hash.entries[5].first
			&& !hash.entries[6].first && !hash.entries[7].first); */
		size.before = hash.size;
		eject = M_(hash_replace)(&hash, t->data);
		assert(t - test || hash.size == 1 && !eject);
		/* We can't even verify that the eject is good, it has no null. */
		item = M_(hash_get)(&hash, t->data);
		/* Read back the item was something that was equal. */
		assert(PM_(equal)(t->data, item));
		/* How can we get a parent pointer? Probably have to have a pointer.
		 Tried; I don't think that worked. */
		size.after = hash.size;
		/* Either it grew or replaced. */
		assert(size.before == size.after || size.after == size.before + 1);
		is_grow = !!(size.after - size.before);
		/* If it replaced, `eject` must be equal to `data`. */
		assert(is_grow || PM_(equal)(t->data, eject));
		if(is_grow)
#if 0
		else if(eject) {
			if(!parent_new) {
				((struct Test *)(void *)((char *)eject
					- offhashof(struct Test, space)))->is_in = 0;
			} else {
				struct Test *sub_t, *sub_t_end;
				/* Slow way; we have got a one-way `test -> elem` so it
				 necessitates a linear search if we want to clear `is_in`. */
				for(sub_t = test, sub_t_end = t; sub_t < sub_t_end; sub_t++) {
					if(!sub_t->is_in
						|| !PM_(equal)(PM_(pointer)(&eject->key),
						PM_(pointer)(&sub_t->elem->key))) continue;
					sub_t->is_in = 0;
					break;
				}
				if(t == t_end) assert(0);
			}
		}
		t->is_in = 1;
#endif
		if(hash.size < 10000 && !(i & (i - 1))) {
			char fn[64];
			printf("*** hash %s.\n", PM_(hash_to_string)(&hash));
			sprintf(fn, "graph/" QUOTE(HASH_NAME) "-%lu.gv", (unsigned long)i);
			PM_(graph)(&hash, fn);
		}
		PM_(legit)(&hash);
	}
	{
		char fn[64];
		printf("\n");
		sprintf(fn, "graph/" QUOTE(HASH_NAME) "-%u-final.gv",
			(unsigned)test_size);
		PM_(graph)(&hash, fn);
		sprintf(fn, "graph/histogram-" QUOTE(HASH_NAME) "-%u.gnu",
			(unsigned)test_size);
		PM_(histogram)(&hash, fn);
	}
	printf("Testing get from hash.\n");
	/* This is more debug info.
	printf("[ ");
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		PM_(to_string)(&t->elem->data, &a);
		printf("%s[%lu-%s]%s", t == test ? "" : ", ",
			(unsigned long)(t - test), t->is_in ? "yes" : "no", a);
	}
	printf(" ]\n");*/
#if 0
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		const size_t n = (size_t)(t - test);
		struct M_(hashlink) *r;
		if(!(n & (n - 1))) {
			PM_(to_string)(&t->elem->key, &a);
			fprintf(stderr, "%lu: retrieving %s.\n", (unsigned long)n, a);
		}
		element = M_(hash_get)(&hash, PM_(pointer)(&t->elem->key));
		assert(element);
		if(t->is_in) {
			assert(element == t->elem);
			if(rand() < RAND_MAX / 8) {
				removed++;
				r = M_(hash_remove)(&hash, PM_(pointer)(&t->elem->key));
				assert(r);
				r = M_(hash_remove)(&hash, PM_(pointer)(&t->elem->key));
				assert(!r);
				r = M_(hash_policy_put)(&hash, t->elem, 0);
				assert(!r);
				r = M_(hash_policy_put)(&hash, t->elem, 0);
				assert(r == t->elem);
				r = M_(hash_remove)(&hash, PM_(pointer)(&t->elem->key));
				assert(r);
			}
		} else {
			const size_t count = hash.size;
			collision++;
			assert(t && element != t->elem);
			r = M_(hash_policy_put)(&hash, t->elem, 0);
			assert(r == t->elem && count == hash.size);
		}
	}
	printf("Collisions: %lu; removed: %lu.\n",
		(unsigned long)collision, (unsigned long)removed);
	PM_(legit)(&hash);
	PM_(stats)(&hash, "\n", stdout);
	M_(hash_clear)(&hash);
	for(b = hash.entries, b_end = b + (1 << hash.log_capacity); b < b_end; b++)
		assert(!PM_(count)(b));
	assert(hash.size == 0);
#endif
	printf("Clear: %s.\n", PM_(hash_to_string)(&hash));
	M_(hash_)(&hash);
	assert(!hash.entries && !hash.log_capacity && !hash.size);
}

/** The list will be tested on `stdout`. Requires `HASH_TEST` to be a
 <typedef:<PM>action_fn> and `HASH_TO_STRING`.
 @param[parent_new] Specifies the dynamic up-level creator of the parent
 `struct`. Could be null; then testing will be done statically on an array of
 <tag:<M>hashlink> and `HASH_TEST` is not allowed to go over the limits of the
 data key. @param[parent] The parameter passed to `parent_new`. Ignored if
 `parent_new` is null. @allow */
static void M_(hash_test)(PM_(key) (*const parent_new)(void *),
	void *const parent) {
	printf("<" QUOTE(HASH_NAME) ">hash of key <" QUOTE(HASH_KEY)
		"> was created using: "
#ifdef HASH_VALUE
		"HASH_VALUE <" QUOTE(HASH_VALUE) ">; "
#endif
		"HASH_UINT <" QUOTE(HASH_UINT) ">; "
		"HASH_CODE <" QUOTE(HASH_CODE) ">; "
		"HASH_IS_EQUAL <" QUOTE(HASH_IS_EQUAL) ">; "
#ifdef HASH_NO_CACHE
		"HASH_NO_CACHE; "
#endif
#ifdef HASH_INVERT
		"HASH_INVERT; "
#endif
		"HASH_TEST; "
		"testing%s:\n", parent ? "(pointer)" : "");
	PM_(test_basic)(parent_new, parent);
	fprintf(stderr, "Done tests of <" QUOTE(HASH_NAME) ">hash.\n\n");
}

#undef QUOTE
#undef QUOTE_

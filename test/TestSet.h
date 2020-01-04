/* Intended to be included by Set.h on `SET_TEST`. */

#include <stdio.h>  /* fprintf FILE */
#include <math.h>   /* sqrt NAN? */
#ifndef NAN /* <https://stackoverflow.com/questions/5714131/nan-literal-in-c> */
#define NAN (0. / 0.)
#endif
#include <string.h> /* memset */

/* Check that `SET_TEST` is a function implementing `<PE>Action`. */
static const PE_(Action) PE_(filler) = (SET_TEST);



#ifdef QUOTE
#undef QUOTE
#endif
#ifdef QUOTE_
#undef QUOTE_
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)



/** Count how many are in the `bucket`.
 @order \O(`bucket.items`) */
static size_t PE_(count)(struct PE_(Bucket) *const bucket) {
	const struct E_(SetElement) *x;
	size_t c = 0;
	assert(bucket);
	for(x = bucket->first; x; x = x->next) c++;
	return c;
}

/** Collect stats; <Welford1962Note>, on `set` and output them to `fp` with
 `delim`.
 @order \O(|`set.bins`| + |`set.items`|) */
static void PE_(stats)(const struct E_(Set) *const set,
	const char *const delim, FILE *fp) {
	struct { size_t n, cost, max_bin; double mean, ssdm; }
		msr = { 0, 0, 0, 0.0, 0.0 };
	size_t size = 0;
	assert(delim);
	if(set && set->buckets) {
		struct PE_(Bucket) *b = set->buckets,
			*b_end = b + (1 << set->log_capacity);
		for( ; b < b_end; b++) {
			double delta, x;
			size_t items = PE_(count)(b);
			msr.cost += items * (items + 1) / 2;
			if(msr.max_bin < items) msr.max_bin = items;
			x = (double)items;
			delta = x - msr.mean;
			msr.mean += delta / ++msr.n;
			msr.ssdm += delta * (x - msr.mean);
		}
		size = set->size;
	}
	/* Sample std dev. */
	fprintf(fp, "entries = %lu%s"
		"buckets = %lu%s"
		"max bucket size = %lu%s"
		"load factor(stderr) = %.2f(%.1f)%s"
		"E(links traversed) = %.2f%s",
		(unsigned long)size, delim,
		(unsigned long)msr.n, delim,
		(unsigned long)msr.max_bin, delim,
		msr.mean, msr.n > 1 ? sqrt(msr.ssdm / (msr.n - 1)) : NAN, delim,
		msr.n ? 1.0 + 1.0 * msr.cost / size : NAN, delim);
}

/** Assertion function for seeing if `set` is in a valid state.
 @order \O(|`set.bins`| + |`set.items`|) */
static void PE_(legit)(const struct E_(Set) *const set) {
	struct PE_(Bucket) *b, *b_end;
	size_t size = 0;
	if(!set) return; /* Null state. */
	if(!set->buckets) { /* Empty state. */
		assert(!set->log_capacity && !set->size);
		return;
	}
	assert(set->log_capacity >= 3 && set->log_capacity < 32);
	for(b = set->buckets, b_end = b + (1 << set->log_capacity); b < b_end; b++)
		size += PE_(count)(b);
	assert(set->size == size);
}

/** Draw a diagram of `set` written to `fn` in
 [Graphviz](https://www.graphviz.org/) format.
 @order \O(|`set.bins`| + |`set.items`|) */
static void PE_(graph)(const struct E_(Set) *const set, const char *const fn) {
	FILE *fp;
	char a[12];
	assert(set && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\trankdir = LR;\n"
		"\tnode [shape = record, style = filled];\n");
	fprintf(fp, "\tSet [label=\"\\<" QUOTE(SET_NAME) "\\>Set: "
		QUOTE(SET_TYPE) "\\l|");
	PE_(stats)(set, "\\l", fp);
	fprintf(fp, "\"];\n");
	if(set->buckets) {
		struct PE_(Bucket) *b, *b_end;
		struct E_(SetElement) *x, *x_prev, *xt;
		fprintf(fp, "\tsubgraph cluster_buckets {\n"
			"\t\tstyle=filled;\n"
			"\t\tnode [fillcolor=lightpink];\n");
		for(b = set->buckets, b_end = b + (1 << set->log_capacity);
			b < b_end; b++) {
			fprintf(fp, "\t\tBucket0x%x;\n",
				(unsigned)(b - set->buckets));
		}
		fprintf(fp, "\t}\n"
			"\tSet -> Bucket0x0;\n");
		for(b = set->buckets, b_end = b + (1 << set->log_capacity);
			b < b_end; b++) {
			fprintf(fp, "\t// Bucket0x%x\n", (unsigned)(b - set->buckets));
			for(xt = x = b->first, x_prev = 0; x; x_prev = x, x = x->next) {
				int is_turtle = 0;
				PE_(to_string)(&x->key, &a);
				fprintf(fp, "\tSetElement%p [label=\"#0x%x\\l|%s\\l\"];\n",
					(void *)x, PE_(get_hash)(x), a);
				if(x_prev) {
					fprintf(fp, "\tSetElement%p -> SetElement%p;\n",
						(void *)x_prev, (void *)x);
				} else {
					fprintf(fp, "\tBucket0x%x -> SetElement%p;\n",
						(unsigned)(b - set->buckets), (void *)x);
				}
				if(is_turtle) xt = xt->next, is_turtle = 0; else is_turtle = 1;
				if(xt == x->next) {
					fprintf(fp, "\tLoop%p [color=red];\n"
						"\tSetElement%p -> Loop%p;\n",
						(void *)b, (void *)x, (void *)b);
					break;
				}
			}
		}
	}
	fprintf(fp, "\tnode [colour=red];\n");
	fprintf(fp, "}\n");
	fclose(fp);
}

/** Draw a histogram of `set` written to `fn` in
 [Gnuplot](http://www.gnuplot.info/) format.
 @order \O(|`set.bins`| + |`set.items`|) */
static void PE_(histogram)(const struct E_(Set) *const set,
	const char *const fn) {
	FILE *fp;
	size_t histogram[64], hs, h;
	const size_t histogram_size = sizeof histogram / sizeof *histogram;
	assert(set && fn);
	memset(histogram, 0, sizeof histogram);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	if(set->buckets) {
		struct PE_(Bucket) *b = set->buckets,
			*b_end = b + (1 << set->log_capacity);
		for( ; b < b_end; b++) {
			size_t items = PE_(count)(b);
			/* The bins are `0,1,2,...,[histogram_size - 1, \infty]`. */
			if(items >= histogram_size) items = histogram_size - 1;
			histogram[items]++;
		}
	}
	/* Hopefully `historgram_size` is much larger then it has to be. */
	for(hs = histogram_size - 1; !(histogram[hs] && (hs++, 1)) && hs; hs--);
	fprintf(fp, "# Size: %lu.\n"
		"set term postscript eps enhanced color\n"
		"set output \"%s.eps\"\n"
		"set grid\n"
		"set xlabel \"bucket occupancy\"\n"
		"set ylabel \"frequency\"\n"
		"set style histogram\n"
		"set xrange [0:]\n"
		"plot \"-\" using 1:2 with boxes lw 3 title \"Histogram\"\n",
		set->size, fn);
	for(h = 0; h < hs; h++) fprintf(fp, "%lu\t%lu\n",
		(unsigned long)h, (unsigned long)histogram[h]);
	fclose(fp);
}

/** Passed `parent_new` and `parent` from <fn:<E>SetTest>. */
static void PE_(test_basic)(struct E_(SetElement) *(*const parent_new)(void *),
	void *const parent) {
	struct Test {
		struct E_(SetElement) space, *elem;
		int is_in;
	} test[10000], *t, *t_end;
	const size_t test_size = sizeof test / sizeof *test;
	int success;
	char a[12];
	size_t removed = 0, collision = 0;
	struct PE_(Bucket) *b, *b_end;
	struct E_(Set) set = SET_IDLE;
	struct E_(SetElement) *eject, *element;
	assert(test_size > 1);
	memset(&test, 0, sizeof test);
	/* Test empty. */
	PE_(legit)(&set);
	E_(Set)(&set);
	assert(!set.buckets && !set.log_capacity && !set.size);
	PE_(legit)(&set);
	PE_(graph)(&set, "graph/" QUOTE(SET_NAME) "-0.gv");
	/* Test placing items. */
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		size_t n = t - test;
		if(parent_new) {
			/* Ignore `space` and allocate a parent pointer. */
			if(!(t->elem = parent_new(parent))) { assert(0); return; }
		} else {
			t->elem = &t->space;
		}
		PE_(filler)(&t->elem->key);
		PE_(to_string)(&t->elem->key, &a);
		success = E_(SetReserve)(&set, 1);
		assert(success && set.buckets);
		if(n == 0) assert(set.log_capacity == 3 && !set.size
			&& !set.buckets[0].first && !set.buckets[1].first
			&& !set.buckets[2].first && !set.buckets[3].first
			&& !set.buckets[4].first && !set.buckets[5].first
			&& !set.buckets[6].first && !set.buckets[7].first);
		eject = E_(SetPut)(&set, t->elem);
		if(n == 0) assert(!eject && set.size == 1);
		else if(eject) {
			if(!parent_new) {
				((struct Test *)(void *)((char *)eject
					- offsetof(struct Test, space)))->is_in = 0;
			} else {
				struct Test *sub_t, *sub_t_end;
				/* Slow way; we have got a one-way `test -> elem` so it
				 necessitates a linear search if we want to clear `is_in`. */
				for(sub_t = test, sub_t_end = t; sub_t < sub_t_end; sub_t++) {
					if(!sub_t->is_in
						|| !PE_(equal)(PE_(pointer)(&eject->key),
						PE_(pointer)(&sub_t->elem->key))) continue;
					sub_t->is_in = 0;
					break;
				}
				if(t == t_end) assert(0);
			}
		}
		t->is_in = 1;
		if(E_(SetSize)(&set) < 1000000 && !(n & (n - 1))) {
			char fn[64];
			fprintf(stderr, "%lu: %s added to set %s.\n",
				(unsigned long)n, a, E_(SetToString)(&set));
			sprintf(fn, "graph/" QUOTE(SET_NAME) "-%u.gv",
				(unsigned)n + 1);
			PE_(graph)(&set, fn);
		}
		PE_(legit)(&set);
	}
	{
		char fn[64];
		PE_(stats)(&set, "\n", stdout);
		printf("\n");
		sprintf(fn, "graph/" QUOTE(SET_NAME) "-%u-final.gv",
			(unsigned)test_size + 1);
		PE_(graph)(&set, fn);
		sprintf(fn, "graph/histogram-" QUOTE(SET_NAME) "-%u.gnu",
			(unsigned)test_size + 1);
		PE_(histogram)(&set, fn);
	}
	printf("Testing get from set.\n");
	/* This is more debug info.
	printf("[ ");
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		PE_(to_string)(&t->elem->data, &a);
		printf("%s[%lu-%s]%s", t == test ? "" : ", ",
			(unsigned long)(t - test), t->is_in ? "yes" : "no", a);
	}
	printf(" ]\n");*/
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		const size_t n = t - test;
		struct E_(SetElement) *r;
		if(!(n & (n - 1))) {
			PE_(to_string)(&t->elem->key, &a);
			fprintf(stderr, "%lu: retrieving %s.\n", (unsigned long)n, a);
		}
		element = E_(SetGet)(&set, PE_(pointer)(&t->elem->key));
		assert(element);
		if(t->is_in) {
			assert(element == t->elem);
			if(rand() < RAND_MAX / 8) {
				removed++;
				r = E_(SetRemove)(&set, PE_(pointer)(&t->elem->key));
				assert(r);
				r = E_(SetRemove)(&set, PE_(pointer)(&t->elem->key));
				assert(!r);
				r = E_(SetPolicyPut)(&set, t->elem, 0);
				assert(!r);
				r = E_(SetPolicyPut)(&set, t->elem, 0);
				assert(r == t->elem);
				r = E_(SetRemove)(&set, PE_(pointer)(&t->elem->key));
				assert(r);
			}
		} else {
			const size_t count = E_(SetSize)(&set);
			collision++;
			assert(t && element != t->elem);
			r = E_(SetPolicyPut)(&set, t->elem, 0);
			assert(r == t->elem && count == E_(SetSize)(&set));
		}
	}
	printf("Collisions: %lu; removed: %lu.\n",
		(unsigned long)collision, (unsigned long)removed);
	PE_(legit)(&set);
	PE_(stats)(&set, "\n", stdout);
	E_(SetClear)(&set);
	for(b = set.buckets, b_end = b + (1 << set.log_capacity); b < b_end; b++)
		assert(!PE_(count)(b));
	assert(E_(SetSize)(&set) == 0);
	printf("Clear: %s.\n", E_(SetToString(&set)));
	E_(Set_)(&set);
	assert(!set.buckets && !set.log_capacity && !set.size);
}

/** The list will be tested on `stdout`. Requires `SET_TEST` to be a
 <typedef:<PE>Action> and `SET_TO_STRING`.
 @param[parent_new] Specifies the dynamic up-level creator of the parent
 `struct`. Could be null; then testing will be done statically on an array of
 <tag:<E>SetElement> and `SET_TEST` is not allowed to go over the limits of the
 data type.
 @param[parent] The parameter passed to `parent_new`. Ignored if `parent_new`
 is null.
 @allow */
static void E_(SetTest)(struct E_(SetElement) *(*const parent_new)(void *),
	void *const parent) {
	printf("<" QUOTE(SET_NAME) ">Set of type <" QUOTE(SET_TYPE)
		"> was created using: SET_HASH <" QUOTE(SET_HASH) ">; "
		"SET_IS_EQUAL <" QUOTE(SET_IS_EQUAL) ">; "
#ifdef SET_NO_CACHE
		"SET_NO_CACHE; "
#endif
		"SET_TO_STRING<" QUOTE(SET_TO_STRING) ">; "
		"SET_TEST<" QUOTE(SET_TEST) ">; "
		"%stesting:\n", parent_new ? "parent type specified; " : "");
	PE_(test_basic)(parent_new, parent);
	fprintf(stderr, "Done tests of <" QUOTE(SET_NAME) ">Set.\n\n");
}

#undef QUOTE
#undef QUOTE_

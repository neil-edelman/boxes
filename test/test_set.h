#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

#include <stddef.h> /* offsetof */
#include <stdio.h>  /* fprintf FILE */
#include <math.h>   /* sqrt NAN? */
#ifndef NAN /* <https://stackoverflow.com/questions/5714131/nan-literal-in-c> */
#define NAN (0. / 0.)
#endif
#include <string.h> /* memset */

/** Operates by side-effects. Used for `SET_TEST`. */
typedef void (*PS_(action_fn))(PS_(type) *);

/* Check that `SET_TEST` is a function implementing `<PS>action_fn`. */
static const PS_(action_fn) PS_(filler) = (SET_TEST);

/** Count how many are in the `bucket`. @order \O(`bucket.items`) */
static size_t PS_(count)(struct PS_(bucket) *const bucket) {
	const struct S_(set_node) *x;
	size_t c = 0;
	assert(bucket);
	for(x = bucket->first; x; x = x->next) c++;
	return c;
}

/** Collect stats; <Welford1962Note>, on `set` and output them to `fp` with
 `delim`. @order \O(|`set.bins`| + |`set.items`|) */
static void PS_(stats)(const struct S_(set) *const set,
	const char *const delim, FILE *fp) {
	struct { size_t n, cost, max_bin; double mean, ssdm; }
		msr = { 0, 0, 0, 0.0, 0.0 };
	size_t size = 0;
	assert(delim);
	if(set && set->buckets) {
		struct PS_(bucket) *b = set->buckets,
			*b_end = b + (1 << set->log_capacity);
		for( ; b < b_end; b++) {
			double delta, x;
			size_t items = PS_(count)(b);
			msr.cost += items * (items + 1) / 2;
			if(msr.max_bin < items) msr.max_bin = items;
			x = (double)items;
			delta = x - msr.mean;
			msr.mean += delta / (double)(++msr.n);
			msr.ssdm += delta * (x - msr.mean);
		}
		size = set->size;
	}
	/* Sample std dev. */
	fprintf(fp, "entries = %lu%s"
		"buckets = %lu%s"
		"max bucket size = %lu%s"
		"load factor(stderr) = %.2f(%.1f)%s"
		"S(links traversed) = %.2f%s",
		(unsigned long)size, delim,
		(unsigned long)msr.n, delim,
		(unsigned long)msr.max_bin, delim,
		msr.mean, msr.n > 1
		? sqrt(msr.ssdm / (double)(msr.n - 1)) : (double)NAN, delim,
		msr.n ? 1.0 + 1.0 * (double)msr.cost / (double)size
		: (double)NAN, delim);
}

/** Assertion function for seeing if `set` is in a valid state.
 @order \O(|`set.bins`| + |`set.items`|) */
static void PS_(legit)(const struct S_(set) *const set) {
	struct PS_(bucket) *b, *b_end;
	size_t size = 0;
	if(!set) return; /* Null state. */
	if(!set->buckets) { /* Empty state. */
		assert(!set->log_capacity && !set->size);
		return;
	}
	assert(set->log_capacity >= 3 && set->log_capacity < 32);
	for(b = set->buckets, b_end = b + (1 << set->log_capacity); b < b_end; b++)
		size += PS_(count)(b);
	assert(set->size == size);
}

/** Draw a diagram of `set` written to `fn` in
 [Graphviz](https://www.graphviz.org/) format.
 @order \O(|`set.bins`| + |`set.items`|) */
static void PS_(graph)(const struct S_(set) *const set, const char *const fn) {
	FILE *fp;
	char a[12];
	assert(set && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\trankdir = LR;\n"
		"\tnode [shape = record, style = filled];\n");
	fprintf(fp, "\tSet [label=\"\\<" QUOTE(SET_NAME) "\\>Set: "
		QUOTE(SET_TYPE) "\\l|");
	PS_(stats)(set, "\\l", fp);
	fprintf(fp, "\"];\n");
	if(set->buckets) {
		struct PS_(bucket) *b, *b_end;
		struct S_(set_node) *x, *x_prev, *xt;
		fprintf(fp, "\tsubgraph cluster_buckets {\n"
			"\t\tstyle=filled;\n"
			"\t\tnode [fillcolor=lightpink];\n");
		for(b = set->buckets, b_end = b + (1 << set->log_capacity);
			b < b_end; b++) {
			fprintf(fp, "\t\tbucket0x%x;\n",
				(unsigned)(b - set->buckets));
		}
		fprintf(fp, "\t}\n"
			"\tSet -> bucket0x0;\n");
		for(b = set->buckets, b_end = b + (1 << set->log_capacity);
			b < b_end; b++) {
			fprintf(fp, "\t// bucket0x%x\n", (unsigned)(b - set->buckets));
			for(xt = x = b->first, x_prev = 0; x; x_prev = x, x = x->next) {
				int is_turtle = 0;
				
				PS_(to_string)(&x->key, &a);
				/* May have to change the width if `SET_UINT`. */
				fprintf(fp, "\tSetElement%p [label=\"#0x%x\\l|%s\\l\"];\n",
					(void *)x, PS_(get_hash)(x), a);
				if(x_prev) {
					fprintf(fp, "\tSetElement%p -> SetElement%p;\n",
						(void *)x_prev, (void *)x);
				} else {
					fprintf(fp, "\tbucket0x%x -> SetElement%p;\n",
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
static void PS_(histogram)(const struct S_(set) *const set,
	const char *const fn) {
	FILE *fp;
	size_t histogram[64], hs, h;
	const size_t histogram_size = sizeof histogram / sizeof *histogram;
	assert(set && fn);
	memset(histogram, 0, sizeof histogram);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	if(set->buckets) {
		struct PS_(bucket) *b = set->buckets,
			*b_end = b + (1 << set->log_capacity);
		for( ; b < b_end; b++) {
			size_t items = PS_(count)(b);
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

/** Passed `parent_new` and `parent` from <fn:<S>set_test>. */
static void PS_(test_basic)(struct S_(set_node) *(*const parent_new)(void *),
	void *const parent) {
	struct Test {
		struct S_(set_node) space, *elem;
		int is_in;
	} test[10000], *t, *t_end;
	const size_t test_size = sizeof test / sizeof *test;
	int success;
	char a[12];
	size_t removed = 0, collision = 0;
	struct PS_(bucket) *b, *b_end;
	struct S_(set) set = SET_IDLE;
	struct S_(set_node) *eject, *element;
	assert(test_size > 1);
	memset(&test, 0, sizeof test);
	/* Test empty. */
	PS_(legit)(&set);
	S_(set)(&set);
	assert(!set.buckets && !set.log_capacity && !set.size);
	PS_(legit)(&set);
	PS_(graph)(&set, "graph/" QUOTE(SET_NAME) "-0.gv");
	/* Test placing items. */
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		size_t n = (size_t)(t - test);
		if(parent_new) {
			/* Ignore `space` and allocate a parent pointer. */
			if(!(t->elem = parent_new(parent))) { assert(0); return; }
		} else {
			t->elem = &t->space;
		}
		PS_(filler)(&t->elem->key);
		PS_(to_string)(&t->elem->key, &a);
		success = S_(set_reserve)(&set, 1);
		assert(success && set.buckets);
		if(n == 0) assert(set.log_capacity == 3 && !set.size
			&& !set.buckets[0].first && !set.buckets[1].first
			&& !set.buckets[2].first && !set.buckets[3].first
			&& !set.buckets[4].first && !set.buckets[5].first
			&& !set.buckets[6].first && !set.buckets[7].first);
		eject = S_(set_put)(&set, t->elem);
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
						|| !PS_(equal)(PS_(pointer)(&eject->key),
						PS_(pointer)(&sub_t->elem->key))) continue;
					sub_t->is_in = 0;
					break;
				}
				if(t == t_end) assert(0);
			}
		}
		t->is_in = 1;
		if(set.size < 1000000 && !(n & (n - 1))) {
			char fn[64];
			fprintf(stderr, "%lu: %s added to set %s.\n",
				(unsigned long)n, a, PS_(set_to_string)(&set));
			sprintf(fn, "graph/" QUOTE(SET_NAME) "-%u.gv",
				(unsigned)n + 1);
			PS_(graph)(&set, fn);
		}
		PS_(legit)(&set);
	}
	{
		char fn[64];
		PS_(stats)(&set, "\n", stdout);
		printf("\n");
		sprintf(fn, "graph/" QUOTE(SET_NAME) "-%u-final.gv",
			(unsigned)test_size + 1);
		PS_(graph)(&set, fn);
		sprintf(fn, "graph/histogram-" QUOTE(SET_NAME) "-%u.gnu",
			(unsigned)test_size + 1);
		PS_(histogram)(&set, fn);
	}
	printf("Testing get from set.\n");
	/* This is more debug info.
	printf("[ ");
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		PS_(to_string)(&t->elem->data, &a);
		printf("%s[%lu-%s]%s", t == test ? "" : ", ",
			(unsigned long)(t - test), t->is_in ? "yes" : "no", a);
	}
	printf(" ]\n");*/
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		const size_t n = (size_t)(t - test);
		struct S_(set_node) *r;
		if(!(n & (n - 1))) {
			PS_(to_string)(&t->elem->key, &a);
			fprintf(stderr, "%lu: retrieving %s.\n", (unsigned long)n, a);
		}
		element = S_(set_get)(&set, PS_(pointer)(&t->elem->key));
		assert(element);
		if(t->is_in) {
			assert(element == t->elem);
			if(rand() < RAND_MAX / 8) {
				removed++;
				r = S_(set_remove)(&set, PS_(pointer)(&t->elem->key));
				assert(r);
				r = S_(set_remove)(&set, PS_(pointer)(&t->elem->key));
				assert(!r);
				r = S_(set_policy_put)(&set, t->elem, 0);
				assert(!r);
				r = S_(set_policy_put)(&set, t->elem, 0);
				assert(r == t->elem);
				r = S_(set_remove)(&set, PS_(pointer)(&t->elem->key));
				assert(r);
			}
		} else {
			const size_t count = set.size;
			collision++;
			assert(t && element != t->elem);
			r = S_(set_policy_put)(&set, t->elem, 0);
			assert(r == t->elem && count == set.size);
		}
	}
	printf("Collisions: %lu; removed: %lu.\n",
		(unsigned long)collision, (unsigned long)removed);
	PS_(legit)(&set);
	PS_(stats)(&set, "\n", stdout);
	S_(set_clear)(&set);
	for(b = set.buckets, b_end = b + (1 << set.log_capacity); b < b_end; b++)
		assert(!PS_(count)(b));
	assert(set.size == 0);
	printf("Clear: %s.\n", PS_(set_to_string)(&set));
	S_(set_)(&set);
	assert(!set.buckets && !set.log_capacity && !set.size);
}

/** The list will be tested on `stdout`. Requires `SET_TEST` to be a
 <typedef:<PS>action_fn> and `SET_TO_STRING`.
 @param[parent_new] Specifies the dynamic up-level creator of the parent
 `struct`. Could be null; then testing will be done statically on an array of
 <tag:<S>set_node> and `SET_TEST` is not allowed to go over the limits of the
 data type. @param[parent] The parameter passed to `parent_new`. Ignored if
 `parent_new` is null. @allow */
static void S_(set_test)(struct S_(set_node) *(*const parent_new)(void *),
	void *const parent) {
	printf("<" QUOTE(SET_NAME) ">set of type <" QUOTE(SET_TYPE)
		"> was created using: SET_HASH <" QUOTE(SET_HASH) ">; "
		"SET_IS_EQUAL <" QUOTE(SET_IS_EQUAL) ">; "
#ifdef SET_NO_CACHE
		"SET_NO_CACHE; "
#endif
		"SET_TO_STRING<" QUOTE(SET_TO_STRING) ">; "
		"SET_TEST<" QUOTE(SET_TEST) ">; "
		"%stesting:\n", parent_new ? "parent type specified; " : "");
	PS_(test_basic)(parent_new, parent);
	fprintf(stderr, "Done tests of <" QUOTE(SET_NAME) ">set.\n\n");
}

#undef QUOTE
#undef QUOTE_

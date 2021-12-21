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
typedef void (*PS_(action_fn))(PS_(type));

/* Check that `SET_TEST` is a function implementing `<PS>action_fn`. */
//static const PS_(action_fn) PS_(filler) = (SET_TEST); ****

/** Count how many are in the `bucket`. @order \O(`bucket.items`) */
static size_t PS_(count)(struct PS_(bucket) *const bucket) {
	const struct PS_(entry) *x;
	size_t c = 0;
	assert(bucket);
	for(x = bucket->head; x; x = x->next) c++;
	return c;
}

/** Collect stats; <Welford1962Note>, on `hash` and output them to `fp`.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void PS_(stats)(const struct S_(hash) *const hash, FILE *fp) {
	struct { size_t n, cost, max_bin; double mean, ssdm; }
		msr = { 0, 0, 0, 0.0, 0.0 };
	size_t size = 0;
	if(hash && hash->buckets) {
		struct PS_(bucket) *b = hash->buckets,
			*b_end = b + (1 << hash->log_capacity);
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
		size = hash->size;
	}
	/* Sample std dev. */

	fprintf(fp,
		"\t<TR>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">entries</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">%lu</TD>\n"
		"\t</TR>\n"
		"\t<TR>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\">buckets</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\">%lu</TD>\n"
		"\t</TR>\n"
		"\t<TR>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">max bucket size</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">%lu</TD>\n"
		"\t</TR>\n"
		"\t<TR>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\">load factor(stderr)</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\">%.2f(%.1f)</TD>\n"
		"\t</TR>\n"
		"\t<TR>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">E(links traversed)</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">%.2f</TD>\n"
		"\t</TR>\n",
		(unsigned long)size,
		(unsigned long)msr.n,
		(unsigned long)msr.max_bin,
		msr.mean, msr.n > 1
		? sqrt(msr.ssdm / (double)(msr.n - 1)) : (double)NAN,
		msr.n ? 1.0 + 1.0 * (double)msr.cost / (double)size
		: (double)NAN);
}

/** Assertion function for seeing if `hash` is in a valid state.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void PS_(legit)(const struct S_(hash) *const hash) {
	struct PS_(bucket) *b, *b_end;
	size_t size = 0;
	if(!hash) return; /* Null state. */
	if(!hash->buckets) { /* Empty state. */
		assert(!hash->log_capacity && !hash->size);
		return;
	}
	assert(hash->log_capacity >= 3 && hash->log_capacity < 32);
	for(b = hash->buckets, b_end = b + (1 << hash->log_capacity); b < b_end; b++)
		size += PS_(count)(b);
	assert(hash->size == size);
}

#define PPS_(n) SET_CAT(pool, PS_(n))
/** Graphs `pool` output to `fn`. */
static void PS_(pool_graph)(const struct PS_(entry_pool) *const pool,
	FILE *fp) {
	char str[12];
	size_t i, j;
	struct PPS_(entry_slot) *slot;
	PPS_(entry_type) *chunk;

	if(!pool->free0.a.size) goto no_free0;
	for(i = 0; i < pool->free0.a.size; i++) {
		fprintf(fp, "\tfree0_%lu [label=<<FONT COLOR=\"Gray75\">%lu</FONT>>,"
			" shape=circle];\n", i, pool->free0.a.data[i]);
		if(i) fprintf(fp, "\tfree0_%lu -> free0_%lu [dir=back];\n",
			i, (unsigned long)((i - 1) / 2));
	}
	fprintf(fp, "\t{rank=same; pool; free0_0; }\n"
		"\tpool:free -> free0_0;\n");
no_free0:
	fprintf(fp, "\tpool [label=<\n"
		"<TABLE BORDER=\"0\">\n"
		"\t<TR><TD COLSPAN=\"3\" ALIGN=\"LEFT\">"
		"<FONT COLOR=\"Gray85\">&lt;" "entry"
		"&gt;pool: set_" QUOTE(SET_NAME) "_entry</FONT></TD></TR>\n"
		"\t<TR>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">"
		"capacity0</TD>\n"
		"\t\t<TD BORDER=\"0\" BGCOLOR=\"Gray90\">&#8205;</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">%lu</TD>\n"
		"\t</TR>\n", (unsigned long)pool->capacity0);
	fprintf(fp, "\t<TR>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\">slots"
		"</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\">%lu</TD>\n"
		"\t\t<TD PORT=\"slots\" BORDER=\"0\" ALIGN=\"RIGHT\">%lu</TD>\n"
		"\t</TR>\n"
		"\t<TR>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">"
		"freeheap0</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">%lu</TD>\n"
		"\t\t<TD PORT=\"free\" BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">"
		"%lu</TD>\n"
		"\t</TR>\n"
		"</TABLE>>];\n",
		(unsigned long)pool->slots.size,
		(unsigned long)pool->slots.capacity,
		(unsigned long)pool->free0.a.size,
		(unsigned long)pool->free0.a.capacity);
	if(!pool->slots.data) return;
	fprintf(fp, "\tpool:slots -> slots;\n"
		"\tslots [label = <\n"
		"<TABLE BORDER=\"0\">\n"
		"\t<TR>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Italic\">i</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Italic\">chunk"
		"</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\">"
		"<FONT FACE=\"Times-Italic\">size</FONT></TD>\n"
		"\t</TR>\n");
	for(i = 0; i < pool->slots.size; i++) {
		const char *const bgc = i & 1 ? "" : " BGCOLOR=\"Gray90\"";
		fprintf(fp, "\t<TR>\n"
			"\t\t<TD ALIGN=\"RIGHT\"%s>%lu</TD>\n"
			"\t\t<TD ALIGN=\"LEFT\"%s>%s</TD>\n"
			"\t\t<TD PORT=\"%lu\" ALIGN=\"RIGHT\"%s>%lu</TD>\n"
			"\t</TR>\n",
			bgc, (unsigned long)i, bgc, orcify(pool->slots.data[i].chunk),
			(unsigned long)i, bgc, pool->slots.data[i].size);
	}
	fprintf(fp, "</TABLE>>];\n");
	/* For each slot, there is a chunk array with data. */
	for(i = 0; i < pool->slots.size; i++) {
		char *bmp;
		slot = pool->slots.data + i;
		chunk = slot->chunk;
		fprintf(fp,
			"\tslots:%lu -> chunk%lu;\n"
			"\tchunk%lu [label=<\n"
			"<TABLE BORDER=\"0\">\n"
			"\t<TR><TD COLSPAN=\"2\" ALIGN=\"LEFT\">"
			"<FONT COLOR=\"Gray85\">%s</FONT></TD></TR>\n",
			(unsigned long)i, (unsigned long)i,
			(unsigned long)i, orcify(chunk));
		if(i || !slot->size) {
			fprintf(fp, "\t<TR><TD COLSPAN=\"2\" ALIGN=\"LEFT\">"
				"<FONT FACE=\"Times-Italic\">count %lu</FONT></TD></TR>\n",
				slot->size);
			goto no_chunk_data;
		}
		/* Primary buffer: print rows. */
		if(!(bmp = calloc(slot->size, sizeof *bmp)))
			{ perror("temp bitmap"); assert(0); exit(EXIT_FAILURE); };
		for(j = 0; j < pool->free0.a.size; j++) {
			size_t *f0p = pool->free0.a.data + j;
			assert(f0p && *f0p < slot->size);
			bmp[*f0p] = 1;
		}
		for(j = 0; j < slot->size; j++) {
			const char *const bgc = j & 1 ? "" : " BGCOLOR=\"Gray90\"";
			fprintf(fp, "\t<TR>\n"
				"\t\t<TD PORT=\"%lu\" ALIGN=\"RIGHT\"%s>%lu</TD>\n",
				(unsigned long)j, bgc, (unsigned long)j);
			if(bmp[j]) {
				fprintf(fp, "\t\t<TD ALIGN=\"LEFT\"%s>"
					"<FONT COLOR=\"Gray75\">deleted"
					"</FONT></TD>\n", bgc);
			} else {
				//PS_(to_string)(chunk + j, &str);
				str[0] = 'f', str[1] = '\0';
				fprintf(fp, "\t\t<TD ALIGN=\"LEFT\"%s>%s</TD>\n", bgc, str);
			}
			fprintf(fp, "\t</TR>\n");
		}
		free(bmp);
no_chunk_data:
		fprintf(fp, "</TABLE>>];\n");
	}
}
#undef PPS_

/** Draw a diagram of `hash` written to `fn` in
 [Graphviz](https://www.graphviz.org/) format.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void PS_(graph)(const struct S_(hash) *const hash, const char *const fn) {
	FILE *fp;
	char z[12];
	assert(hash && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	printf("*** %s\n", fn);
	fprintf(fp, "digraph {\n"
		"\trankdir=LR;\n"
		"\tfontface=modern;\n"
		"\tnode [shape=box, style=filled, fillcolor=\"Gray95\"];\n"
		"\tset [label=<\n"
		"<TABLE BORDER=\"0\">\n"
		"\t<TR><TD COLSPAN=\"2\" ALIGN=\"LEFT\">"
		"<FONT COLOR=\"Gray85\">&lt;" QUOTE(SET_NAME)
		"&gt;hash: " QUOTE(SET_TYPE) "</FONT></TD></TR>\n");
	PS_(stats)(hash, fp);
	fprintf(fp, "</TABLE>>]\n");
	if(hash->buckets) {
		size_t i, i_end;
		/*struct PS_(bucket) *b, *b_end;*/
		struct PS_(entry) *x, *x_prev, *xt;
		fprintf(fp, "\tset -> buckets;\n"
			"\tbuckets [label = <\n"
			"<TABLE BORDER=\"0\">\n"
			"\t<TR><TD BORDER=\"0\"><FONT FACE=\"Times-Italic\">"
			"bucket</FONT></TD></TR>\n");
		for(i = 0, i_end = 1 << hash->log_capacity; i < i_end; i++) {
			const char *const bgc = i & 1 ? "" : " BGCOLOR=\"Gray90\"";
			fprintf(fp, "\t<TR><TD PORT=\"%lu\" ALIGN=\"RIGHT\"%s>0x%lx"
				"</TD></TR>\n", (unsigned long)i, bgc, (unsigned long)i);
		}
		fprintf(fp, "</TABLE>>];\n");
#if 0
		fprintf(fp, "\tsubgraph cluster_buckets {\n"
			"\t\tstyle=filled;\n"
			"\t\tnode [fillcolor=lightpink];\n");
		fprintf(fp, "\t}\n"
			"\tset -> bucket0x0;\n");
		for(b = hash->buckets, b_end = b + (1 << hash->log_capacity);
			b < b_end; b++) {
			fprintf(fp, "\t// bucket0x%x\n", (unsigned)(b - hash->buckets));
			for(xt = x = b->head, x_prev = 0; x; x_prev = x, x = x->next) {
				int is_turtle = 0;
				
				PS_(to_string)(&x->key, &a);
				/* May have to change the width if `SET_UINT`. */
				fprintf(fp, "\tSetElement%p [label=\"#0x%lx\\l|%s\\l\"];\n",
					(void *)x, (unsigned long)PS_(hash_from_entry)(x), a);
				if(x_prev) {
					fprintf(fp, "\tSetElement%p -> SetElement%p;\n",
						(void *)x_prev, (void *)x);
				} else {
					fprintf(fp, "\tbucket0x%x -> SetElement%p;\n",
						(unsigned)(b - hash->buckets), (void *)x);
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
#endif
	}
	PS_(pool_graph)(&hash->entries, fp);
	fprintf(fp, "\tset -> pool;\n"
		"\tnode [colour=red];\n");
	fprintf(fp, "}\n");
	fclose(fp);
}

/** Draw a histogram of `hash` written to `fn` in
 [Gnuplot](http://www.gnuplot.info/) format.
 @order \O(|`hash.bins`| + |`hash.items`|) */
static void PS_(histogram)(const struct S_(hash) *const hash,
	const char *const fn) {
	FILE *fp;
	size_t histogram[64], hs, h;
	const size_t histogram_size = sizeof histogram / sizeof *histogram;
	assert(hash && fn);
	memset(histogram, 0, sizeof histogram);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	if(hash->buckets) {
		struct PS_(bucket) *b = hash->buckets,
			*b_end = b + (1 << hash->log_capacity);
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
		"hash term postscript eps enhanced color\n"
		"hash output \"%s.eps\"\n"
		"hash grid\n"
		"hash xlabel \"bucket occupancy\"\n"
		"hash ylabel \"frequency\"\n"
		"hash style histogram\n"
		"hash xrange [0:]\n"
		"plot \"-\" using 1:2 with boxes lw 3 title \"Histogram\"\n",
		hash->size, fn);
	for(h = 0; h < hs; h++) fprintf(fp, "%lu\t%lu\n",
		(unsigned long)h, (unsigned long)histogram[h]);
	fclose(fp);
}

/** Passed `parent_new` and `parent` from <fn:<S>set_test>. */
static void PS_(test_basic)(PS_(type) (*const parent_new)(void *),
	void *const parent) {
	struct test { PS_(type) elem; int is_in, unused; } test[10/*000*/], *t, *t_end;
	const size_t test_size = sizeof test / sizeof *test;
	int success;
	char z[12];
	size_t removed = 0, collision = 0;
	struct PS_(bucket) *b, *b_end;
	struct S_(hash) hash = SET_IDLE;
	PS_(type) eject;
	assert(test_size > 1);
	memset(&test, 0, sizeof test);
	/* Test empty. */
	PS_(legit)(&hash);
	S_(hash)(&hash);
	assert(!hash.buckets && !hash.log_capacity && !hash.size);
	PS_(legit)(&hash);
	PS_(graph)(&hash, "graph/" QUOTE(SET_NAME) "-0.gv");
	/* Test placing items. */
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		size_t n = (size_t)(t - test);
		if(parent_new && !(t->elem = parent_new(parent))) { assert(0); return; }
		/*PS_(filler)(t->elem);*/
		PS_(to_string)(&t->elem, &z);
		printf("%lu: came up with %s.\n", (unsigned long)n, z);
		/*success = S_(set_reserve)(&hash, 1);
		assert(success && hash.buckets);
		if(n == 0) assert(hash.log_capacity == 3 && !hash.size
			&& !hash.buckets[0].first && !hash.buckets[1].first
			&& !hash.buckets[2].first && !hash.buckets[3].first
			&& !hash.buckets[4].first && !hash.buckets[5].first
			&& !hash.buckets[6].first && !hash.buckets[7].first); */
		eject = S_(set_put)(&hash, t->elem);
		if(n == 0) assert(!eject && hash.size == 1);
#if 0
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
		if(hash.size < 1000000 && !(n & (n - 1))) {
			char fn[64];
			fprintf(stderr, "%lu: %s added to hash %s.\n",
				(unsigned long)n, a, PS_(set_to_string)(&hash));
			sprintf(fn, "graph/" QUOTE(SET_NAME) "-%u.gv",
				(unsigned)n + 1);
			PS_(graph)(&hash, fn);
		}
#endif
		PS_(legit)(&hash);
	}
	{
		char fn[64];
		/*PS_(stats)(&hash, "\n", stdout);*/
		printf("\n");
		sprintf(fn, "graph/" QUOTE(SET_NAME) "-%u-final.gv",
			(unsigned)test_size + 1);
		PS_(graph)(&hash, fn);
		sprintf(fn, "graph/histogram-" QUOTE(SET_NAME) "-%u.gnu",
			(unsigned)test_size + 1);
		PS_(histogram)(&hash, fn);
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
	printf("Clear: %s.\n", PS_(set_to_string)(&hash));
	S_(set_)(&hash);
	assert(!hash.buckets && !hash.log_capacity && !hash.size);
}

/** The list will be tested on `stdout`. Requires `SET_TEST` to be a
 <typedef:<PS>action_fn> and `SET_TO_STRING`.
 @param[parent_new] Specifies the dynamic up-level creator of the parent
 `struct`. Could be null; then testing will be done statically on an array of
 <tag:<S>setlink> and `SET_TEST` is not allowed to go over the limits of the
 data type. @param[parent] The parameter passed to `parent_new`. Ignored if
 `parent_new` is null. @allow */
static void S_(set_test)(PS_(type) (*const parent_new)(void *),
	void *const parent) {
	printf("<" QUOTE(SET_NAME) ">hash of type <" QUOTE(SET_TYPE)
		"> was created using: SET_HASH <" QUOTE(SET_HASH) ">; "
		"SET_IS_EQUAL <" QUOTE(SET_IS_EQUAL) ">; "
#ifdef SET_RECALCULATE
		"SET_RECALCULATE; "
#endif
		"SET_TEST<" QUOTE(SET_TEST) ">; "
		"%stesting:\n", parent_new ? "parent type specified; " : "");
	PS_(test_basic)(parent_new, parent);
	fprintf(stderr, "Done tests of <" QUOTE(SET_NAME) ">hash.\n\n");
}

#undef QUOTE
#undef QUOTE_

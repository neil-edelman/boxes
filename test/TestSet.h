/* Intended to be included by Set.h on `SET_TEST`. */

#include <stdio.h>  /* fprintf FILE */
#include <math.h>   /* sqrt inf */
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



/** Count how many are in the {bucket}.
 @order O(n) */
static size_t PE_(count)(struct PE_(Bucket) *const bucket) {
	const struct E_(SetElement) *x;
	size_t c = 0;
	assert(bucket);
	for(x = bucket->first; x; x = x->next) c++;
	return c;
}

/* Collect stats; \cite{Welford1962Note}. */
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

/** Assertion function for seeing if it is in a valid state.
 @order O(|{set.bins}| + |{set.items}|) */
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

/** Draw a graph of this {Set} to {fn} in Graphviz format.
 @order O(|{set.bins}| + |{set.items}|) */
static void PE_(graph)(const struct E_(Set) *const set, const char *const fn) {
	FILE *fp;
	char a[12];
	assert(set && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\trankdir = LR;\n"
		"\tnode [shape = record, style = filled];\n");
	fprintf(fp, "\tSet [label=\"\\<" QUOTE(SET_NAME) "\\>Set: "
		QUOTE(SET_TYPE) "\\l|", (unsigned long)set->size,
		set->log_capacity ? 1 << set->log_capacity : 0,
		(double)set->size / (1 << set->log_capacity));
	PE_(stats)(set, "\\l", fp);
	fprintf(fp, "\"];\n");
	if(set->buckets) {
		struct PE_(Bucket) *b, *b_end;
		struct E_(SetElement) *x, *x_prev, *xt;
		fprintf(fp, "\tsubgraph cluster_buckets {\n"
			"\t\tstyle=filled;\n"
			/*"\t\tnode [color=white];\n"*/);
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
				PE_(to_string)(&x->data, &a);
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

static void PE_(test_basic)(void) {
	struct Test {
		struct E_(SetElement) key;
		int is_in;
	} test[3000], *t, *t_end;
	const size_t test_size = sizeof test / sizeof *test;
	int success;
	char a[12];
	size_t removed = 0, collision = 0;
	struct PE_(Bucket) *b, *b_end;
	struct E_(Set) set = SET_ZERO;
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
		PE_(filler)(&t->key.data);
		PE_(to_string)(&t->key.data, &a);
		success = E_(SetReserve)(&set, 1);
		assert(success && set.buckets);
		if(n == 0) assert(set.log_capacity == 3 && !set.size
			&& !set.buckets[0].first && !set.buckets[1].first
			&& !set.buckets[2].first && !set.buckets[3].first
			&& !set.buckets[4].first && !set.buckets[5].first
			&& !set.buckets[6].first && !set.buckets[7].first);
		eject = E_(SetPut)(&set, &t->key);
		if(n == 0) assert(!eject && set.size == 1);
		else if(eject) ((struct Test *)(void *)((char *)eject
			- offsetof(struct Test, key)))->is_in = 0;
		t->is_in = 1;
		if(n == 2 || n == 15 || n == 150 || n == 300 || n == 1500) {
			char fn[512];
			fprintf(stderr, "%lu: %s added to set %s.\n",
				(unsigned long)n, a, E_(SetToString)(&set));
			sprintf(fn, "graph/" QUOTE(SET_NAME) "-%u.gv",
				(unsigned)n + 1);
			PE_(graph)(&set, fn);
		} else if(n % 750 == 375) {
			PE_(stats)(&set, "\n", stdout);
			printf("\n");
		}
		PE_(legit)(&set);
	}
	printf("Testing get from set.\n");
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		struct E_(SetElement) *r;
		PE_(to_string)(&t->key.data, &a);
		fprintf(stderr, "Retiving %s.\n", a);
		element = E_(SetGet)(&set, t->key.data);
		assert(element);
		if(t->is_in) {
			assert(element == &t->key);
			if(rand() < RAND_MAX / 8) {
				removed++;
				r = E_(SetRemove)(&set, t->key.data);
				assert(r);
				r = E_(SetRemove)(&set, t->key.data);
				assert(!r);
				r = E_(SetPolicyPut)(&set, &t->key, 0);
				assert(!r);
				r = E_(SetPolicyPut)(&set, &t->key, 0);
				assert(!r);
				r = E_(SetRemove)(&set, t->key.data);
				assert(r);
			}
		} else {
			collision++;
			assert(t && element != &t->key);
			r = E_(SetPolicyPut)(&set, &t->key, 0);
			assert(!r);
			r = E_(SetPolicyPut)(&set, &t->key, 0);
			assert(!r);
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

/* void *const base, const size_t size,
 const size_t width, size_t offset */

/** The list will be tested on stdout. */
static void E_(SetTest)(void) {
	printf("<" QUOTE(SET_NAME) ">Set was created using: "
		"SET_HASH <" QUOTE(SET_HASH) ">; "
		"SET_IS_EQUAL <" QUOTE(SET_IS_EQUAL) ">; "
#ifdef SET_NO_CACHE
		"SET_NO_CACHE; "
#endif
		"SET_TO_STRING<" QUOTE(SET_TO_STRING) ">; "
		"SET_TEST<" QUOTE(SET_TEST) ">; "
		"testing:\n");
	PE_(test_basic)();
	fprintf(stderr, "Done tests of <" QUOTE(SET_NAME) ">Set.\n\n");
}

#undef QUOTE
#undef QUOTE_

/* Intended to be included by Set.h on `SET_TEST`. */

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
	const struct E_(SetKey) *x;
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
		"load factor = %.2f(%.1f)%s"
		"E(links traversed) = %.2f%s",
		(unsigned long)size, delim,
		(unsigned long)msr.n, delim,
		(unsigned long)msr.max_bin, delim,
		msr.mean, msr.n > 1 ? sqrt(msr.ssdm / (msr.n - 1)) : NAN, delim,
		msr.n ? 1.0 * msr.cost / size : NAN, delim);
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
		"\trankdir = LR;\n");
	fprintf(fp, "\tSet [label=\"Set<" QUOTE(SET_NAME) ">: of type <"
		QUOTE(SET_KEY) ", " QUOTE(SET_VALUE) ">\\l", (unsigned long)set->size,
		set->log_capacity ? 1 << set->log_capacity : 0,
		(double)set->size / (1 << set->log_capacity));
	PE_(stats)(set, "\\l", fp);
	fprintf(fp, "\", shape=box];\n");
	if(set->buckets) {
		struct PE_(Bucket) *b, *b_end;
		struct E_(SetKey) *x, *x_prev, *xt;
		for(b = set->buckets, b_end = b + (1 << set->log_capacity);
			b < b_end; b++) {
			int is_turtle = 0;
			fprintf(fp, "\tsubgraph cluster_%p {\n"
				"\t\tstyle=filled;\n"
				"\t\tEntry%p [label=\"Bucket%u\", color=seagreen, shape=box];"
				"\n", (void *)b, (void *)x, (unsigned)(b - set->buckets));
			for(xt = x = b->first, x_prev = 0; x; x_prev = x, x = x->next) {
				PE_(to_string)(&x->data, &a);
				fprintf(fp, "\t\tEntry%p [label=\"%u\\l%s\\l\"];\n"
					"\t\tEntry%p -> Entry%p;\n",
					(void *)x, PE_(get_hash)(x), a, (void *)x_prev, (void *)x);
				if(is_turtle) xt = xt->next, is_turtle = 0; else is_turtle = 1;
				if(xt == x->next) {
					fprintf(fp, "\t\tLoop%p [color=red];\n"
						"\t\tEntry%p -> Loop%p;\n",
						(void *)b, (void *)x, (void *)b);
					break;
				}
			}
			fprintf(fp, "\t}\n");
		}
	}
	fprintf(fp, "\tnode [colour=red, style=filled];\n");
	fprintf(fp, "}\n");
	fclose(fp);
}

/* struct A
 30 test; 64 buckets; 3 max; 0.469(0.7) average bucket size.
 30 test; 64 buckets; 2 max; 0.469(0.7) average bucket size.
 300 test; 512 buckets; 4 max; 0.586(0.8) average bucket size.
 300 test; 512 buckets; 4 max; 0.586(0.8) average bucket size.
 3000 test; 8192 buckets; 4 max; 0.366(0.6) average bucket size.
 3000 test; 8192 buckets; 4 max; 0.366(0.6) average bucket size.
 30000 test; 65536 buckets; 6 max; 0.458(0.7) average bucket size.
 30000 test; 65536 buckets; 6 max; 0.458(0.7) average bucket size. */

static void PE_(test_basic)(void) {
	struct {
		struct E_(SetKey) key;
		int is_in;
	} test[30];
	const size_t test_size = sizeof test / sizeof *test;
	struct E_(Set) set = SET_ZERO;
	struct E_(SetKey) *key, *eject;
	int success;
	/* Test empty. */
	PE_(legit)(&set);
	E_(Set)(&set);
	assert(!set.buckets && !set.log_capacity && !set.size);
	PE_(legit)(&set);
	fprintf(stderr, "Empty: %s.\n", E_(SetToString)(&set));
	/* Test one item. */
	success = E_(SetReserve)(&set, 1);
	assert(success);
	PE_(filler)(&test[0].key.data);
	eject = E_(SetPut)(&set, key);
	assert(!eject);
	fprintf(stderr, "One: %s.\n", E_(SetToString)(&set));
#if 0
	for(t = test, t_end = t + test_size; t < t_end; t++) {
		PE_(filler)(&t->element.data);
		PE_(to_string)(&t->element.data, &a);
		printf("About to put %s into set.\n", a);
		PE_(legit)(&set);
		assert(E_(SetReserve)(&set, 1));
		eject = E_(SetPut)(&set, &t->element);
		PE_(legit)(&set);
		if(eject) ((struct Test *)(void *)((char *)eject
			- offsetof(struct Test, element)))->is_in = 0;
		t->is_in = 1;
		n = t - test;
		if(n == 15 || n == 150 || n == 300) {
			char fn[512];
			sprintf(fn, "graph/" QUOTE(SET_NAME) "-insert-%u.gv",
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
		struct E_(SetKey) *r;
		element = E_(SetGet)(&set, t->element.data);
		assert(element);
		if(t->is_in) {
			assert(element == &t->element.data);
			if(rand() < RAND_MAX / 8) {
				removed++;
				r = E_(SetRemove)(&set, t->element.data);
				assert(r);
				r = E_(SetRemove)(&set, t->element.data);
				assert(!r);
				r = E_(SetPutResolve)(&set, &t->element, 0);
				assert(!r);
				r = E_(SetPutResolve)(&set, &t->element, 0);
				assert(!r);
			}
		} else {
			collision++;
			assert(t && element != &t->element.data);
			r = E_(SetPutResolve)(&set, &t->element, 0);
			assert(!r);
			r = E_(SetPutResolve)(&set, &t->element, 0);
			assert(!r);
		}
	}
	printf("Collisions: %lu; removed: %lu.\n",
		(unsigned long)collision, (unsigned long)removed);
	PE_(legit)(&set);
	PE_(stats)(&set, "\n", stdout);
	printf("Testing clear.\n");
	E_(SetClear)(&set);
	for(b = set.buckets, b_end = b + (1 << set.log_capacity); b < b_end; b++)
		assert(!PE_(count)(b));
	assert(E_(SetSize)(&set) == 0);
	E_(Set_)(&set);
	assert(!set.buckets && !set.log_capacity && !set.size);
#endif
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
	fprintf(stderr, "Done tests of Set<" QUOTE(SET_NAME) ">.\n\n");
}

#undef QUOTE
#undef QUOTE_

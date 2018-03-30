/* Intended to be included by Map.h on {MAP_TEST}. */

#include <math.h> /* sqrt */

typedef void (*PKV_(Action))(struct KV_(Entry) *const);

/* Check that {MAP_TO_STRING} is a function implementing {<E>Action}. */
static const PKV_(Action) PKV_(filler) = (ENTRY_TEST);



#ifdef KV_NAME
#undef KV_NAME
#endif
#ifdef QUOTE
#undef QUOTE
#endif
#ifdef QUOTE_
#undef QUOTE_
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
#define KV_NAME QUOTE(ENTRY_NAME)



#if 0

/** Count how many are in the {bin}.
 @order O(n) */
static size_t PE_(count)(const struct PE_(EntryList) *const bin) {
	struct PTE_(X) *i;
	size_t c = 0;
	for(i = bin->head.anonymous_next; i->anonymous_next; i = i->anonymous_next)
		c++;
	return c;
}

/** Assertion function for seeing if it is in a valid state.
 @order O(|{map.bins}| + |{map.items}|) */
static void PE_(legit)(const struct E_(Map) *const map) {
	struct PE_(EntryList) *b, *end;
	size_t entries = 0;
	assert(map);
	assert(map->log_bins > 0 && map->log_bins < 32);
	for(b = map->bins, end = b + (1 << map->log_bins); b < end; b++)
		entries += PE_(count)(b);
	assert(map->entries == entries);
}

/** Draw a graph of this {Map} to {fn} in Graphviz format.
 @order O(|{map.bins}| + |{map.items}|) */
static void PE_(graph)(const struct E_(Map) *const map, const char *const fn) {
	FILE *fp;
	E *entry;
	struct PE_(EntryList) *b, *end;
	struct E_(MapNode) *mn;
	char str[12];
	assert(map && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n");
	for(b = map->bins, end = b + (1 << map->log_bins); b < end; b++) {
		/* Bins. */
		fprintf(fp, "\tsubgraph cluster_%p {\n"
			"\t\tnode [shape=box];\n"
			"\t\tp%p [label=\"head%u\"];\n"
			"\t\tp%p [label=\"tail%u\"];\n"
			"\t\tnode [shape=oval];\n"
			"\t\tp%p -> p%p;\n"
			"\t\tp%p -> p%p [style=dashed];\n",
			(void *)b, (void *)&b->head, (unsigned)(b - map->bins),
			(void *)&b->tail, (unsigned)(b - map->bins),
			(void *)&b->head, (void *)b->head.anonymous_next,
			(void *)&b->tail, (void *)b->tail.anonymous_prev);
		/* Draw all items. */
		for(entry = PE_(EntryListFirst(b); entry; entry = PE_(EntryListNext)(entry))){
			PE_(to_string)(entry, &str);
			mn = PE_(node_holds_item)(entry);
			fprintf(fp, "\t\tp%p [label=\"%u\\l%s\\l\"];\n"
				"\t\tp%p -> p%p;\n"
				"\t\tp%p -> p%p [style=dashed];\n",
				(void *)&mn->node.x, mn->hash, str,
				(void *)&mn->node.x, (void *)mn->node.x.anonymous_next,
				(void *)&mn->node.x, (void *)mn->node.x.anonymous_prev);
		}
		fprintf(fp, "\t}\n");
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
	struct E_(Map) *m = 0;
	struct Test {
		int is_in;
		struct E_(MapNode) node;
	} test[3000], *t, *end;
	/* \cite{Welford1962Note} */
	struct Measure { size_t n; double mean, ssdm; } msr = { 0, 0, 0 };
	struct PE_(EntryList) *b, *b_end;
	E *eject, *i;
	const size_t test_size = sizeof test / sizeof *test;
	size_t max_bin = 0, map_size = 0, test_in_size = 0;
	char a[12];
	memset(test, 0, sizeof test);
	m = E_(Map)();
	assert(m);
	PE_(legit)(m);
	printf("Testing put in map.\n");
	for(t = test, end = t + test_size; t < end; t++) {
		PE_(filler)(&t->node.node.data);
		PE_(to_string)(&t->node.node.data, &a);
		/*printf("About to put %s into map.\n", a);*/
		PE_(legit)(m);
		eject = E_(MapPut)(m, &t->node.node.data);
		t->is_in = 1;
		if(eject) {
			/*PE_(to_string)(eject, &a), printf("Ejected %s", a);*/
			((struct Test *)(void *)
				((char *)eject
				- offsetof(struct Test, node)
				- offsetof(struct E_(MapNode), node)
				- offsetof(struct PE_(EntryListNode), data)))->is_in = 0;
		}
		if((t - test) == 150) PE_(graph)(m, "graph/" E_NAME ".gv");
		/*printf("m: %s.\n", E_(MapToString(m)));*/
		PE_(legit)(m);
	}
	printf("Testing get from map.\n");
	for(t = test, end = t + test_size; t < end; t++) {
		if(!t->is_in) continue;
		i = E_(MapGet)(m, PE_(item_key)(&t->node.node.data));
		assert(i && i == &t->node.node.data);
		PE_(legit)(m);
	}
	/* Test size. */
	for(b = m->bins, b_end = b + (1 << m->log_bins); b < b_end; b++) {
		for(i = PE_(EntryListFirst(b); i; i = PE_(EntryListNext)(i))) {
			t = ((struct Test *)(void *)((char *)i
				- offsetof(struct Test, node)
				- offsetof(struct E_(MapNode), node)
				- offsetof(struct PE_(EntryListNode), data)));
			assert(t->is_in);
			map_size++;
		}
	}
	for(t = test, end = t + test_size; t < end; t++) {
		if(t->is_in) test_in_size++;
	}
	printf("Size reported by size: %lu; size counting bins: %lu; "
		"test in size: %lu.\n", (unsigned long)E_(MapSize)(m),
		(unsigned long)map_size, (unsigned long)test_in_size);
	assert(E_(MapSize)(m) == map_size && map_size == test_in_size);
	/* Collect stats. */
	for(b = m->bins, b_end = b + (1 << m->log_bins); b < b_end; b++) {
		double delta, x;
		size_t items = 0;
		for(i = PE_(EntryListFirst(b); i; i = PE_(EntryListNext)(i))) items++;
		if(max_bin < items) max_bin = items;
		x = (double)items;
		delta = x - msr.mean;
		msr.mean += delta / ++msr.n;
		msr.ssdm += delta * (x - msr.mean);
	}
	printf("Testing clear.\n");
	E_(MapClear)(m);
	for(b = m->bins, b_end = b + (1 << m->log_bins); b < b_end; b++) {
		for(i = PE_(EntryListFirst(b); i; i = PE_(EntryListNext)(i))) {
			assert(0);
		}
	}
	assert(E_(MapSize)(m) == 0);
	E_(Map_)(&m);
	assert(!m);
	printf("%lu test; %lu buckets; %lu max; %3.3f(%.1f) average bucket size.\n",
		(unsigned long)test_size, (unsigned long)msr.n, (unsigned long)max_bin,
		msr.mean, sqrt(msr.ssdm / (msr.n - 1)) /* Sample std dev. */);
}

#endif

/** The list will be tested on stdout. */
static void KV_(EntryTest)(void) {
	printf("<" KV_NAME ">Entry: of type <"
		QUOTE(ENTRY_KEY) "," QUOTE(ENTRY_VALUE) "> was created using: "
		"ENTRY_CMP <" QUOTE(ENTRY_CMP) ">; "
		"ENTRY_HASH <" QUOTE(ENTRY_HASH) ">; "
		"ENTRY_TO_STRING <" QUOTE(ENTRY_TO_STRING) ">; "
		"ENTRY_TEST<" QUOTE(ENTRY_TEST) ">; "
		"testing:\n");
	/*PE_(test_basic)();*/
	fprintf(stderr, "Done tests of Set<" KV_NAME ">.\n\n");
}

#undef KV_NAME
#undef QUOTE
#undef QUOTE_

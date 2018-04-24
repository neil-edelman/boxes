/* Intended to be included by Map.h on {MAP_TEST}. */

#include <math.h> /* sqrt */

/* Check that {ENTRY_TEST} is a function implementing {<PKV>Action}. */
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



static void PKV_(test_basic)(void) {
	struct KV_(Map) *m = 0;
	struct Test {
		int is_in;
		struct KV_(MapNode) node;
	} test[3000], *t, *end;
	/* \cite{Welford1962Note} */
	struct Measure { size_t n; double mean, ssdm; } msr = { 0, 0, 0 };
	struct PKV_(EntryList) *b, *b_end;
	struct KV_(Entry) *eject, *i;
	V *value;
	const size_t test_size = sizeof test / sizeof *test;
	size_t max_bin = 0, map_size = 0, test_in_size = 0;
	char a[12];
	memset(test, 0, sizeof test);
	m = KV_(Map)();
	assert(m);
	printf("Testing put in map.\n");
	for(t = test, end = t + test_size; t < end; t++) {
		PKV_(filler)(&t->node.node.data);
		PKV_(to_string)(&t->node.node.data, &a);
		printf("About to put %s into map.\n", a);
		eject = KV_(MapPut)(m, &t->node.node.data);
		t->is_in = 1;
		if(eject) {
			PKV_(to_string)(eject, &a), printf("Ejected %s.\n", a);
			((struct Test *)(void *)
				((char *)eject
				- offsetof(struct Test, node)
				- offsetof(struct KV_(MapNode), node)
				- offsetof(struct PKV_(EntryLink), data)))->is_in = 0;
		}
	}
	printf("Testing get from map.\n");
	for(t = test, end = t + test_size; t < end; t++) {
		if(!t->is_in) continue;
		value = KV_(MapGetValue)(m, PKV_(item_key)(&t->node.node.data));
		i = KV_(MapGet)(m, PKV_(item_key)(&t->node.node.data));
		assert(i && i == &t->node.node.data && value == &t->node);
	}
	/* Test size. */
	for(b = m->bins, b_end = b + (1 << m->log_bins); b < b_end; b++) {
		for(i = PKV_(EntryListFirst(b); i; i = PKV_(EntryListNext)(i))) {
			t = ((struct Test *)(void *)((char *)i
				- offsetof(struct Test, node)
				- offsetof(struct KV_(MapNode), node)
				- offsetof(struct PKV_(EntryLink), data)));
			assert(t->is_in);
			map_size++;
		}
	}
	for(t = test, end = t + test_size; t < end; t++) {
		if(t->is_in) test_in_size++;
	}
	printf("Size reported by size: %lu; size counting bins: %lu; "
		"test in size: %lu.\n", (unsigned long)KV_(MapSize)(m),
		(unsigned long)map_size, (unsigned long)test_in_size);
	assert(KV_(MapSize)(m) == map_size && map_size == test_in_size);
	/* Collect stats. */
	for(b = m->bins, b_end = b + (1 << m->log_bins); b < b_end; b++) {
		double delta, x;
		size_t items = 0;
		for(i = PKV_(EntryListFirst(b); i; i = PKV_(EntryListNext)(i))) items++;
		if(max_bin < items) max_bin = items;
		x = (double)items;
		delta = x - msr.mean;
		msr.mean += delta / ++msr.n;
		msr.ssdm += delta * (x - msr.mean);
	}
	printf("Testing clear.\n");
	KV_(MapClear)(m);
	for(b = m->bins, b_end = b + (1 << m->log_bins); b < b_end; b++) {
		for(i = PKV_(EntryListFirst(b); i; i = PKV_(EntryListNext)(i))) {
			assert(0);
		}
	}
	assert(KV_(MapSize)(m) == 0);
	KV_(Map_)(&m);
	assert(!m);
	printf("%lu test; %lu buckets; %lu max; %3.3f(%.1f) average bucket size.\n",
		(unsigned long)test_size, (unsigned long)msr.n, (unsigned long)max_bin,
		msr.mean, sqrt(msr.ssdm / (msr.n - 1)) /* Sample std dev. */);
}

/** The list will be tested on stdout. */
static void KV_(EntryTest)(void) {
	printf("<" KV_NAME ">Entry: of type <"
		QUOTE(ENTRY_KEY) "," QUOTE(ENTRY_VALUE) "> was created using: "
		"ENTRY_CMP <" QUOTE(ENTRY_CMP) ">; "
		"ENTRY_HASH <" QUOTE(ENTRY_HASH) ">; "
		"ENTRY_TO_STRING <" QUOTE(ENTRY_TO_STRING) ">; "
		"ENTRY_TEST<" QUOTE(ENTRY_TEST) ">; "
		"testing:\n");
	PKV_(test_basic)();
	fprintf(stderr, "Done tests of Set<" KV_NAME ">.\n\n");
}

#undef KV_NAME
#undef QUOTE
#undef QUOTE_

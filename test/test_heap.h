#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

#ifdef HEAP_TO_STRING /* <!-- to string: Only one, tests all base code. */

/* Copy functions for later includes. */
static void (*PH_(to_string))(const PH_(node) *, char (*)[12])
	= (HEAP_TO_STRING);
static const char *(*PH_(heap_to_string))(const struct H_(heap) *)
	= Z_(to_string);

/** Operates by side-effects. Used for `HEAP_TEST`. */
typedef void (*PH_(biaction_fn))(PH_(node) *, void *);

/* `HEAP_TEST` must be a function that implements `<PH>Action`. */
static const PH_(biaction_fn) PH_(filler) = (HEAP_TEST);

/** Draw a graph of `heap` to `fn` in Graphviz format. */
static void PH_(graph)(const struct H_(heap) *const heap,
	const char *const fn) {
	FILE *fp;
	char a[12];
	assert(heap && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\trankdir = BT;\n"
		"\tnode [shape = record, style = filled];\n"
		"\tHash [label=\"{\\<" QUOTE(HEAP_NAME) "\\>Hash: "
#ifdef HEAP_VALUE
		QUOTE(HEAP_VALUE)
#else
		"without value"
#endif
		"\\l|size: %lu\\lcapacity: %lu\\l}\"];\n", (unsigned long)heap->a.size,
		(unsigned long)heap->a.capacity);
	if(heap->a.data) {
		PH_(node) *const n0 = heap->a.data;
		size_t i;
		fprintf(fp, "\tnode [fillcolor=lightsteelblue];\n");
		if(heap->a.size) fprintf(fp, "\tn0 -> Hash [dir = back];\n");
		fprintf(fp, "\tedge [style = dashed];\n"
			"\tsubgraph cluster_data {\n"
			"\t\tstyle=filled;\n");
		for(i = 0; i < heap->a.size; i++) {
			PH_(to_string)(n0 + i, &a);
			fprintf(fp, "\t\tn%lu [label=\"%s\"];\n", (unsigned long)i, a);
			if(!i) continue;
			fprintf(fp, "\t\tn%lu -> n%lu;\n", (unsigned long)i,
				(unsigned long)((i - 1) >> 1));
		}
		fprintf(fp, "\t}\n");
	}
	fprintf(fp, "\tnode [colour=red];\n");
	fprintf(fp, "}\n");
	fclose(fp);
}

/** Makes sure the `heap` is in a valid state. */
static void PH_(valid)(const struct H_(heap) *const heap) {
	size_t i;
	PH_(node) *n0;
	if(!heap) return;
	if(!(n0 = heap->a.data)) { assert(!heap->a.size); return; }
	for(i = 1; i < heap->a.size; i++) {
		size_t iparent = (i - 1) >> 1;
		if(PH_(compare)(PH_(get_priority)(n0 + iparent),
			PH_(get_priority)(n0 + i)) <= 0) continue;
		PH_(graph)(heap, "graph/" QUOTE(HEAP_NAME) "-invalid.gv");
		assert(0);
		break;
	}
}

/** @param[param] The parameter to call <typedef:<PH>biaction_fn>
 `HEAP_TEST`. */
static void PH_(test_basic)(void *const param) {
	struct H_(heap) heap = HEAP_IDLE;
	PH_(node) *node, add, result;
	PH_(value) v;
	PH_(priority) last_priority = 0;
	const size_t test_size_1 = 11, test_size_2 = 31, test_size_3 = 4000/*0*/;
	size_t i;
	char fn[64];
	int success;

	printf("Test empty.\n");
	PH_(valid)(0);
	errno = 0;
	assert(!H_(heap_size)(&heap));
	H_(heap_)(&heap);
	assert(!H_(heap_size)(&heap));
	H_(heap)(&heap);
	assert(!H_(heap_size)(&heap));
	assert(!H_(heap_peek)(&heap));
	assert(!H_(heap_peek_value(&heap)));
	assert(!H_(heap_pop)(&heap));
	PH_(valid)(&heap);
	assert(!errno);

	printf("Test one.\n");
	PH_(filler)(&add, param);
	v = PH_(get_value)(&add);
	assert(H_(heap_add)(&heap, add));
	printf("Added one, %s.\n", PH_(heap_to_string)(&heap));
	assert(H_(heap_size)(&heap) == 1);
	node = H_(heap_peek)(&heap);
	PH_(valid)(&heap);
	assert(PH_(get_priority)(node) == PH_(get_priority)(&add));
	result = H_(heap_pop)(&heap);
	assert(v == PH_(get_value)(&result) && !H_(heap_size)(&heap));
	PH_(valid)(&heap);

	printf("Test many.\n");
	for(i = 0; i < test_size_1; i++) {
		if(!i || !(i & (i - 1))) {
			sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu.gv",
				(unsigned long)i);
			PH_(graph)(&heap, fn);
		}
		PH_(valid)(&heap);
		PH_(filler)(&add, param);
		success = H_(heap_add)(&heap, add);
		assert(success);
	}
	sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu-done-1.gv", (unsigned long)i);
	PH_(graph)(&heap, fn);
	assert(H_(heap_size)(&heap) == test_size_1);
	printf("Heap: %s.\n", PH_(heap_to_string)(&heap));
	printf("Heap buffered add, before size = %lu.\n",
		(unsigned long)H_(heap_size)(&heap));
	node = H_(heap_buffer)(&heap, test_size_2);
	assert(node);
	for(i = 0; i < test_size_2; i++) PH_(filler)(node + i, param);
	success = H_(heap_append)(&heap, test_size_2);
	printf("Now size = %lu.\n", (unsigned long)H_(heap_size)(&heap));
	assert(H_(heap_size)(&heap) == test_size_1 + test_size_2);
	sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu-buffer.gv",
		test_size_1 + test_size_2);
	PH_(graph)(&heap, fn);
	PH_(valid)(&heap);
	assert(H_(heap_size)(&heap) == test_size_1 + test_size_2);
	for(i = 0; i < test_size_3; i++) {
		if(!i || !(i & (i - 1))) {
			sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu.gv",
				test_size_1 + test_size_2 + (unsigned long)i);
			PH_(graph)(&heap, fn);
		}
		PH_(valid)(&heap);
		PH_(filler)(&add, param);
		success = H_(heap_add)(&heap, add);
		assert(success);
	}
	printf("Final heap: %s.\n", PH_(heap_to_string)(&heap));
	assert(H_(heap_size)(&heap) == test_size_1 + test_size_2 + test_size_3);
	for(i = test_size_1 + test_size_2 + test_size_3; i > 0; i--) {
		char a[12];
		node = H_(heap_peek)(&heap);
		assert(node);
		v = H_(heap_peek_value)(&heap);
		PH_(to_string)(node, &a);
		result = H_(heap_pop)(&heap);
		if(!i || !(i & (i - 1))) {
			printf("%lu: retreving %s.\n", (unsigned long)i, a);
			sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-remove-%lu.gv",
				(unsigned long)i);
			PH_(graph)(&heap, fn);
		}
		assert(v == PH_(get_value)(&result) && H_(heap_size)(&heap) == i - 1);
		PH_(valid)(&heap);
		if(i != test_size_1 + test_size_2 + test_size_3)
			assert(PH_(compare)(last_priority, PH_(get_priority)(node)) <= 0);
		last_priority = PH_(get_priority)(node);
	}
	printf("Destructor:\n");
	H_(heap_)(&heap);
	assert(!H_(heap_peek)(&heap));
}

/** Will be tested on stdout. Requires `HEAP_TEST`, `HEAP_TO_STRING`, and not
 `NDEBUG` while defining `assert`.
 @param[param] The parameter to call <typedef:<PH>biaction_fn> `HEAP_TEST`.
 @allow */
static void H_(heap_test)(void *const param) {
	printf("<" QUOTE(HEAP_NAME) ">heap"
		" of priority type <" QUOTE(HEAP_TYPE) ">"
		" was created using:"
		" HEAP_COMPARE<" QUOTE(HEAP_COMPARE) ">;"
#ifdef HEAP_VALUE
		" HEAP_VALUE<" QUOTE(HEAP_VALUE) ">;"
#endif
		" HEAP_TO_STRING <" QUOTE(HEAP_TO_STRING) ">;"
		" HEAP_TEST <" QUOTE(HEAP_TEST) ">;"
		" testing:\n");
	PH_(test_basic)(param);
	fprintf(stderr, "Done tests of <" QUOTE(HEAP_NAME) ">heap.\n\n");
}


#else /* compare --><!-- */
#error Test unsupported option; testing is out-of-sync?
#endif /* --> */

#undef QUOTE
#undef QUOTE_

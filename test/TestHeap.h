/* Intended to be included by `Heap.h` on `HEAP_TEST`. */

/* Define macros. */
#ifdef QUOTE
#undef QUOTE
#endif
#ifdef QUOTE_
#undef QUOTE_
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** Operates by side-effects. Used for `HEAP_TEST`. */
typedef void (*PH_(BiAction))(struct H_(HeapNode) *, void *);

/* `HEAP_TEST` must be a function that implements `<PH>Action`. */
static const PH_(BiAction) PH_(filler) = (HEAP_TEST);

/** Draw a graph of `heap` to `fn` in Graphviz format. */
static void PH_(graph)(const struct H_(Heap) *const heap,
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
		"\\l|size: %lu\\lcapacity: %lu\\l"
		"next capacity: %lu\\l}\"];\n", (unsigned long)heap->a.size,
		(unsigned long)heap->a.capacity, (unsigned long)heap->a.next_capacity);
	if(heap->a.data) {
		struct H_(HeapNode) *const n0 = heap->a.data;
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
static void PH_(valid)(const struct H_(Heap) *const heap) {
	size_t i;
	struct H_(HeapNode) *n0;
	if(!heap) return;
	if(!heap->a.data) { assert(!heap->a.size); return; }
	n0 = heap->a.data;
	for(i = 1; i < heap->a.size; i++) {
		size_t iparent = (i - 1) >> 1;
		if(PH_(compare)(n0[iparent].priority, n0[i].priority) <= 0) continue;
		PH_(graph)(heap, "graph/" QUOTE(HEAP_NAME) "-invalid.gv");
		assert(0);
		break;
	}
}

/** @param[param] The parameter to call <typedef:<PH>BiAction> `HEAP_TEST`. */
static void PH_(test_basic)(void *const param) {
	struct H_(Heap) heap = HEAP_IDLE;
	struct H_(HeapNode) *node, add;
	PH_(PValue) v, result;
	PH_(Priority) last_priority;
	const size_t test_size_1 = 11, test_size_2 = 31, test_size_3 = 4000/*0*/;
	size_t i;
	char fn[64];
	int success;

	printf("Test empty.\n");
	PH_(valid)(0);
	errno = 0;
	assert(!H_(HeapSize)(&heap));
	H_(Heap_)(&heap);
	assert(!H_(HeapSize)(&heap));
	H_(Heap)(&heap);
	assert(!H_(HeapSize)(&heap));
	assert(!H_(HeapPeek)(&heap));
	assert(!H_(HeapPeekValue(&heap)));
	assert(!H_(HeapPop)(&heap));
	PH_(valid)(&heap);
	assert(!errno);

	printf("Test one.\n");
	PH_(filler)(&add, param);
	v = PH_(value)(&add);
	assert(H_(HeapAdd)(&heap, add));
	printf("Added one, %s.\n", H_(HeapToString)(&heap));
	assert(H_(HeapSize)(&heap) == 1);
	node = H_(HeapPeek)(&heap);
	PH_(valid)(&heap);
	assert(node->priority == add.priority);
	result = H_(HeapPop(&heap));
	assert(v == result && !H_(HeapSize)(&heap));
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
		success = H_(HeapAdd)(&heap, add);
		assert(success);
	}
	sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu-done-1.gv", (unsigned long)i);
	PH_(graph)(&heap, fn);
	assert(H_(HeapSize)(&heap) == test_size_1);
	printf("Heap: %s.\n", H_(HeapToString)(&heap));
	printf("Heap buffered add, before size = %lu.\n",
		(unsigned long)H_(HeapSize)(&heap));
	node = H_(HeapReserve)(&heap, test_size_2);
	assert(node);
	for(i = 0; i < test_size_2; i++) PH_(filler)(node + i, param);
	success = H_(HeapBuffer)(&heap, test_size_2);
	printf("Now size = %lu.\n", (unsigned long)H_(HeapSize)(&heap));
	assert(H_(HeapSize)(&heap) == test_size_1 + test_size_2);
	sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu-buffer.gv",
		test_size_1 + test_size_2);
	PH_(graph)(&heap, fn);
	PH_(valid)(&heap);
	assert(H_(HeapSize)(&heap) == test_size_1 + test_size_2);
	for(i = 0; i < test_size_3; i++) {
		if(!i || !(i & (i - 1))) {
			sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu.gv",
				test_size_1 + test_size_2 + (unsigned long)i);
			PH_(graph)(&heap, fn);
		}
		PH_(valid)(&heap);
		PH_(filler)(&add, param);
		success = H_(HeapAdd)(&heap, add);
		assert(success);
	}
	printf("Final heap: %s.\n", H_(HeapToString)(&heap));
	assert(H_(HeapSize)(&heap) == test_size_1 + test_size_2 + test_size_3);
	for(i = test_size_1 + test_size_2 + test_size_3; i > 0; i--) {
		char a[12];
		node = H_(HeapPeek)(&heap);
		assert(node);
		v = H_(HeapPeekValue)(&heap);
		PH_(to_string)(node, &a);
		result = H_(HeapPop)(&heap);
		if(!i || !(i & (i - 1))) {
			printf("%lu: retreving %s.\n", (unsigned long)i, a);
			sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-remove-%lu.gv",
					(unsigned long)i);
			PH_(graph)(&heap, fn);
		}
		assert(v == result && H_(HeapSize)(&heap) == i - 1);
		PH_(valid)(&heap);
		if(i != test_size_1 + test_size_2 + test_size_3)
			assert(PH_(compare)(last_priority, node->priority) <= 0);
		last_priority = node->priority;
	}
	printf("Destructor:\n");
	H_(Heap_)(&heap);
	assert(!H_(HeapPeek)(&heap));
}

/** Will be tested on stdout. Requires `HEAP_TEST`, `HEAP_TO_STRING`, and not
 `NDEBUG` while defining `assert`.
 @param[param] The parameter to call <typedef:<PH>BiAction> `HEAP_TEST`.
 @allow */
static void H_(HeapTest)(void *const param) {
	printf("<" QUOTE(HEAP_NAME) ">Heap"
		" of type <" QUOTE(HEAP_TYPE) ">"
		" was created using:"
		" HEAP_COMPARE<" QUOTE(HEAP_COMPARE) ">;"
#ifdef HEAP_VALUE
		" HEAP_VALUE<" QUOTE(HEAP_VALUE) ">;"
#endif
		" HEAP_TO_STRING <" QUOTE(ARRAY_TO_STRING) ">;"
		" HEAP_TEST <" QUOTE(HEAP_TEST) ">;"
		" testing:\n");
	PH_(test_basic)(param);
	fprintf(stderr, "Done tests of <" QUOTE(HEAP_NAME) ">Heap.\n\n");
}

/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_

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
typedef void (*PH_(Action))(struct H_(HeapNode) *);

/* `HEAP_TEST` must be a function that implements `<PH>Action`. */
static const PH_(Action) PH_(filler) = (HEAP_TEST);

/** Draw a graph of `heap` to `fn` in Graphviz format. */
static void PH_(graph)(const struct H_(Heap) *const heap,
	const char *const fn) {
	FILE *fp;
	char a[12];
	assert(heap && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\trankdir = BT; #LR;\n"
		"\tnode [shape = record, style = filled];\n"
		"\tHash [label=\"{\\<" QUOTE(HEAP_NAME) "\\>Hash: "
#ifdef HEAP_TYPE
		QUOTE(HEAP_TYPE)
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
		if(heap->a.size) fprintf(fp, "\tHash -> n0;\n");
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
		if(PH_(compare)(n0[(i - 1) >> 1].priority, n0[i].priority) <= 0) {
			PH_(graph)(heap, "graph/" QUOTE(HEAP_NAME) "-invalid.gv");
			assert(0);
			break;
		}
	}
}

static void PH_(test_basic)(void) {
	struct H_(Heap) heap = HEAP_IDLE;
	struct H_(HeapNode) *node, add;
	PH_(Value) v, result;
	const size_t test_size = 20;
	size_t i;
	char fn[64];

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
	PH_(filler)(&add);
	v = PH_(value)(&add);
	assert(H_(HeapAdd)(&heap, add));
	printf("Added: %s.\n", H_(HeapToString)(&heap));
	assert(H_(HeapSize)(&heap) == 1);
	node = H_(HeapPeek)(&heap);
	PH_(valid)(&heap);
	assert(node->priority == add.priority);
	result = H_(HeapPop(&heap));
	assert(v == result && !H_(HeapSize)(&heap));
	PH_(valid)(&heap);

	printf("Test many.\n");
	for(i = 0; i < test_size; i++) {
		sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu.gv", (unsigned long)i);
		PH_(graph)(&heap, fn);
		PH_(filler)(&add);
		assert(H_(HeapAdd)(&heap, add));
		PH_(valid)(&heap);
	}
	PH_(graph)(&heap, "graph/" QUOTE(HEAP_NAME) "-little.gv");
	assert(H_(HeapSize)(&heap) == test_size);
	for(i = 0; i < test_size; i++) {
		char a[12];
		node = H_(HeapPeek)(&heap);
		assert(node);
		v = H_(HeapPeekValue)(&heap);
		PH_(to_string)(node, &a);
		printf("Retreving %s.\n", a);
		result = H_(HeapPop)(&heap);
		assert(v == result && H_(HeapSize)(&heap) == test_size - i - 1);
		PH_(valid)(&heap);
	}

	printf("Destructor:\n");
	H_(Heap_)(&heap);
	assert(!H_(HeapPeek)(&heap));
}

#if 0

static void PT_(test_random)(void) {
	struct T_(Array) a;
	const size_t mult = 1; /* For long tests. */
	/* This parameter controls how many iterations. */
	size_t i, i_end = 1000 * mult, size = 0;
	/* Random. */
	T_(Array)(&a);
	for(i = 0; i < i_end; i++) {
		T *data;
		char str[12];
		double r = rand() / (RAND_MAX + 1.0);
		printf("%lu: ", (unsigned long)i);
		/* This parameter controls how big the pool wants to be. */
		if(r > size / (100.0 * mult)) {
			if(!(data = T_(ArrayNew)(&a))) {
				perror("Error"), assert(0);
				return;
			}
			size++;
			PT_(filler)(data);
			PT_(to_string)(data, &str);
			printf("created %s.\n", str);
		} else {
#ifdef ARRAY_STACK /* <!-- stack */
			double t = 1.0;
#else /* stack --><!-- !stack */
			double t = 0.5;
#endif /* !stack --> */
			r = rand() / (RAND_MAX + 1.0);
			if(r < t) {
				data = T_(ArrayPeek)(&a);
				assert(data);
				PT_(to_string)(data, &str);
				printf("popping %s.\n", str);
				assert(data == T_(ArrayPop)(&a));
			} else {
#ifndef ARRAY_STACK /* <!-- !stack */
				size_t idx = rand() / (RAND_MAX + 1.0) * size;
				if(!(data = T_(ArrayGet)(&a) + idx)) continue;
				PT_(to_string)(data, &str);
				printf("removing %s at %lu.\n", str, (unsigned long)idx);
				{
					const int ret = T_(ArrayRemove)(&a, data);
					assert(ret || (perror("Removing"), 0));
				}
#endif /* !stack --> */
			}
			size--;
		}
		PT_(valid_state)(&a);
		if(T_(ArraySize)(&a) < 1000000 && !(i & (i - 1))) {
			char fn[32];
			printf("%s.\n", T_(ArrayToString)(&a));
			sprintf(fn, "graph/" QUOTE(ARRAY_NAME) "Array%lu.gv",
				(unsigned long)i);
			PT_(graph)(&a, fn);
		}
	}
	T_(Array_)(&a);
}

#endif




/** Will be tested on stdout. Requires `HEAP_TEST`, `HEAP_TO_STRING`, and not
 `NDEBUG` while defining `assert`. */
static void H_(HeapTest)(void) {
	printf("<" QUOTE(HEAP_NAME) ">Heap"
#ifdef HEAP_TYPE
		" of type <" QUOTE(HEAP_TYPE) ">"
#endif
		" was created using: HEAP_COMPARE<" QUOTE(HEAP_COMPARE) ">;"
		" HEAP_PRIORITY<" QUOTE(HEAP_PRIORITY) ">;"
		" HEAP_TO_STRING <" QUOTE(ARRAY_TO_STRING) ">;"
		" HEAP_TEST <" QUOTE(HEAP_TEST) ">;"
		" testing:\n");
	PH_(test_basic)();
	fprintf(stderr, "Done tests of <" QUOTE(HEAP_NAME) ">Heap.\n\n");
}

/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_

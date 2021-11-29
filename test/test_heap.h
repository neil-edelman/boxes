#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** Used for `HEAP_TEST`, operates by side-effects. */
typedef void (*PH_(biaction_fn))(PH_(node) *, void *);

/** `HEAP_TEST` must be a function that implements <typedef:<PH>biaction_fn>. */
static PH_(biaction_fn) PH_(filler) = (HEAP_TEST);

/** Draw a graph of `heap` to `fn` in Graphviz format. */
static void PH_(graph)(const struct H_(heap) *const heap,
	const char *const fn) {
	FILE *fp;
	char a[12];
	size_t i;
	assert(heap && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		/*"\trankdir = BT;\n"*/
		"\tnode [shape = box, style = filled, fillcolor = Grey95];\n");
	for(i = 0; i < heap->a.size; i++) {
		PH_(to_string)(heap->a.data + i, &a);
		fprintf(fp, "\t\tn%lu [label=\"%s\"];\n", (unsigned long)i, a);
		if(!i) continue;
		fprintf(fp, "\t\tn%lu -> n%lu;\n",
			(unsigned long)((i - 1) / 2), (unsigned long)i);
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

/** @param[param] The parameter used for `HEAP_TEST`. */
static void PH_(test_basic)(void *const param) {
	struct H_(heap) heap = HEAP_IDLE, merge = HEAP_IDLE;
	PH_(node) add, *node;
	PH_(value) v, w, x;
	PH_(priority) last_priority = 0;
	const size_t test_size_1 = 11, test_size_2 = 31, test_size_3 = 4000/*0*/;
	size_t i, cum_size = 0;
	char fn[64];
	int success;

	printf("Test empty.\n");
	PH_(valid)(0);
	errno = 0;
	assert(!heap.a.size);
	H_(heap_)(&heap);
	assert(!heap.a.size);
	H_(heap)(&heap);
	assert(!heap.a.size);
	assert(!H_(heap_peek)(&heap));
	assert(!H_(heap_pop)(&heap));
	PH_(valid)(&heap);
	assert(!errno);

	printf("Test one.\n");
	PH_(filler)(&add, param);
	v = PH_(get_value)(&add);
	assert(H_(heap_add)(&heap, add)), cum_size++;
	printf("Added one, %s.\n", PH_(heap_to_string)(&heap));
	assert(heap.a.size == cum_size);
	w = H_(heap_peek)(&heap);
	PH_(valid)(&heap);
	//FIXME
	//assert(PH_(get_priority)(&v) == PH_(get_priority)(&add));
	x = H_(heap_pop)(&heap), cum_size--;
	assert(v == x && w == x && heap.a.size == cum_size);
	PH_(valid)(&heap);

	printf("Test many.\n");
	for(i = 0; i < test_size_1; i++) {
		if(!i || !(i & (i - 1))) {
			sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu.gv",
				(unsigned long)cum_size);
			PH_(graph)(&heap, fn);
		}
		PH_(valid)(&heap);
		PH_(filler)(&add, param);
		success = H_(heap_add)(&heap, add), cum_size++;
		assert(success);
	}
	sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu-done-1.gv",
		(unsigned long)cum_size);
	PH_(graph)(&heap, fn);
	assert(heap.a.size == cum_size);
	printf("Heap: %s.\n", PH_(heap_to_string)(&heap));
	printf("Heap buffered add, before size = %lu.\n",
		(unsigned long)heap.a.size);
	node = H_(heap_buffer)(&heap, test_size_2);
	assert(node);
	for(i = 0; i < test_size_2; i++) PH_(filler)(node + i, param);
	success = H_(heap_append)(&heap, test_size_2), cum_size += test_size_2;
	printf("Now size = %lu.\n", (unsigned long)heap.a.size);
	assert(heap.a.size == cum_size);
	sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu-buffer.gv", cum_size);
	PH_(graph)(&heap, fn);
	PH_(valid)(&heap);
	for(i = 0; i < test_size_3; i++) {
		if(!i || !(i & (i - 1))) {
			sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu.gv",
				(unsigned long)cum_size);
			PH_(graph)(&heap, fn);
		}
		PH_(valid)(&heap);
		PH_(filler)(&add, param);
		success = H_(heap_add)(&heap, add), cum_size++;
		assert(success);
	}
	sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu-heap.gv",
		(unsigned long)cum_size);
	PH_(graph)(&heap, fn);
	printf("Setting up merge at %lu.\n", (unsigned long)cum_size);
	for(i = 0; i < test_size_1; i++) {
		PH_(filler)(&add, param);
		success = H_(heap_add)(&merge, add);
		assert(success);
	}
	sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu-merge.gv",
		(unsigned long)cum_size);
	PH_(graph)(&merge, fn);
	PH_(valid)(&merge);
	success = H_(heap_reproduce)(&heap, &merge), cum_size += merge.a.size;
	sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu-combined.gv",
		(unsigned long)cum_size);
	PH_(graph)(&heap, fn);
	assert(success && heap.a.size == cum_size);
	PH_(valid)(&heap);
	printf("Final heap: %s.\n", PH_(heap_to_string)(&heap));
	for(i = cum_size; i > 0; i--) {
		char a[12];
		v = H_(heap_peek)(&heap);
		assert(node);
		PH_(to_string)(node, &a);
		x = H_(heap_pop)(&heap);
		if(!i || !(i & (i - 1))) {
			printf("%lu: retreving %s.\n", (unsigned long)i, a);
			sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-remove-%lu.gv",
				(unsigned long)i);
			PH_(graph)(&heap, fn);
		}
		assert(v == x && heap.a.size == i - 1);
		PH_(valid)(&heap);
		if(i != cum_size)
			assert(PH_(compare)(last_priority, PH_(get_priority)(node)) <= 0);
		last_priority = PH_(get_priority)(node);
	}
	printf("Destructor:\n");
	H_(heap_)(&merge);
	H_(heap_)(&heap);
	assert(!H_(heap_peek)(&heap));
}

/** Will be tested on stdout. Requires `HEAP_TEST`, `HEAP_TO_STRING`, and not
 `NDEBUG` while defining `assert`.
 @param[param] The `void *` parameter in `HEAP_TEST`. Can be null. @allow */
static void H_(heap_test)(void *const param) {
	printf("<" QUOTE(HEAP_NAME) ">heap"
		" of priority type <" QUOTE(HEAP_TYPE) ">"
		" was created using:"
		" HEAP_COMPARE<" QUOTE(HEAP_COMPARE) ">;"
#ifdef HEAP_VALUE
		" HEAP_VALUE<" QUOTE(HEAP_VALUE) ">;"
#endif
		" HEAP_TEST <" QUOTE(HEAP_TEST) ">;"
		" testing:\n");
	PH_(test_basic)(param);
	fprintf(stderr, "Done tests of <" QUOTE(HEAP_NAME) ">heap.\n\n");
}

#undef QUOTE
#undef QUOTE_

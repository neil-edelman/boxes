#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** Used for `HEAP_TEST`: either, `HEAP_VALUE` (*v)->p, or (v)->p, in which it
 is just a wasted parameter, the return value is what's important. */
#ifdef HEAP_VALUE
typedef PH_(priority) (*PH_(test_fn))(PH_(value) *);
#else
typedef PH_(priority) (*PH_(test_fn))(void);
#endif

/** Draw a graph of `heap` to `fn` in Graphviz format. */
static void PH_(graph)(const struct H_(heap) *const heap,
	const char *const fn) {
	FILE *fp;
	char a[12];
	size_t i;
	assert(heap && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\tgraph [truecolor=true, bgcolor=transparent];\n"
		"\tfontface=modern;\n"
		"\tnode [shape=box, style=filled, fillcolor=\"Gray95\"];\n"
		/* Google search / Wikipedia says we should draw them with the top down.
		"\trankdir = BT;\n" */
		"\tedge [arrowhead = none];\n");
	for(i = 0; i < heap->as_array.size; i++) {
#ifdef HEAP_VALUE
		H_(to_string)(heap->as_array.data[i].value, &a);
#else
		H_(to_string)(heap->as_array.data + i, &a);
#endif
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
	if(!(n0 = heap->as_array.data)) { assert(!heap->as_array.size); return; }
	for(i = 1; i < heap->as_array.size; i++) {
		size_t iparent = (i - 1) >> 1;
		if(PH_(compare)(PH_(get_priority)(n0 + iparent),
			PH_(get_priority)(n0 + i)) <= 0) continue;
		PH_(graph)(heap, "graph/" QUOTE(HEAP_NAME) "-invalid.gv");
		assert(0);
		break;
	}
}

/** Fills `node` whether it has `HEAP_VALUE` or not. */
static void PH_(fill)(PH_(node) *const node) {
#ifdef HEAP_VALUE
	node->priority = H_(filler)(&node->value);
#else
	*node = H_(filler)();
#endif
}

static void PH_(test_basic)(void) {
	struct H_(heap) heap = H_(heap)(), merge = H_(heap)();
	PH_(node) add, *node;
	PH_(value) v, x;
	PH_(priority) last_priority = 0;
	const size_t test_size_1 = 11, test_size_2 = 31, test_size_3 = 4000/*0*/;
	size_t i, cum_size = 0;
	char fn[64];
	int success, ret;

	printf("Test empty.\n");
	PH_(valid)(0);
	errno = 0;
	assert(!heap.as_array.size);
	H_(heap_)(&heap);
	assert(!heap.as_array.size);
	assert(!H_(heap_peek)(&heap));
	/*assert(!H_(heap_pop)(&heap));*/
	PH_(valid)(&heap);
	assert(!errno);

	printf("Test one.\n");
	PH_(fill)(&add);
	v = PH_(get_value)(&add);
	ret = H_(heap_add)(&heap, add), assert(ret), cum_size++;
	printf("Added one, %s.\n", H_(heap_to_string)(&heap));
	assert(heap.as_array.size == cum_size);
	node = H_(heap_peek)(&heap);
	PH_(valid)(&heap);
	x = H_(heap_pop)(&heap), cum_size--;
	assert(v == x && heap.as_array.size == cum_size);
	assert(PH_(get_value)(node) == x);
	PH_(valid)(&heap);

	printf("Test many.\n");
	for(i = 0; i < test_size_1; i++) {
		if(!i || !(i & (i - 1))) {
			sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu.gv",
				(unsigned long)cum_size);
			PH_(graph)(&heap, fn);
		}
		PH_(valid)(&heap);
		PH_(fill)(&add);
		success = H_(heap_add)(&heap, add), cum_size++;
		assert(success);
	}
	sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu-done-1.gv",
		(unsigned long)cum_size);
	PH_(graph)(&heap, fn);
	assert(heap.as_array.size == cum_size);
	printf("Heap: %s.\n", H_(heap_to_string)(&heap));
	printf("Heap buffered add, before size = %lu.\n",
		(unsigned long)heap.as_array.size);
	node = H_(heap_buffer)(&heap, test_size_2);
	assert(node);
	for(i = 0; i < test_size_2; i++) PH_(fill)(node + i);
	H_(heap_append)(&heap, test_size_2), cum_size += test_size_2;
	printf("Now size = %lu.\n", (unsigned long)heap.as_array.size);
	assert(heap.as_array.size == cum_size);
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
		PH_(fill)(&add);
		success = H_(heap_add)(&heap, add), cum_size++;
		assert(success);
	}
	sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu-heap.gv",
		(unsigned long)cum_size);
	PH_(graph)(&heap, fn);
	printf("Setting up merge at %lu.\n", (unsigned long)cum_size);
	for(i = 0; i < test_size_1; i++) {
		PH_(fill)(&add);
		success = H_(heap_add)(&merge, add);
		assert(success);
	}
	sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu-merge.gv",
		(unsigned long)cum_size);
	PH_(graph)(&merge, fn);
	PH_(valid)(&merge);
	success = H_(heap_affix)(&heap, &merge), cum_size += merge.as_array.size;
	sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-%lu-combined.gv",
		(unsigned long)cum_size);
	PH_(graph)(&heap, fn);
	assert(success && heap.as_array.size == cum_size);
	PH_(valid)(&heap);
	printf("Final heap: %s.\n", H_(heap_to_string)(&heap));
	for(i = cum_size; i > 0; i--) {
		char z[12];
		node = H_(heap_peek)(&heap);
		assert(node);
#ifdef HEAP_VALUE
		H_(to_string)(node->value, &z);
#else
		H_(to_string)(node, &z);
#endif
		H_(heap_pop)(&heap);
		if(!i || !(i & (i - 1))) {
			printf("%lu: retreving %s.\n", (unsigned long)i, z);
			sprintf(fn, "graph/" QUOTE(HEAP_NAME) "-remove-%lu.gv",
				(unsigned long)i);
			PH_(graph)(&heap, fn);
		}
		assert(heap.as_array.size == i - 1 && H_(heap_size)(&heap) == i - 1);
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
static void H_(heap_test)(void) {
	printf("<" QUOTE(HEAP_NAME) ">heap"
		" of priority type <" QUOTE(HEAP_TYPE) ">"
		" was created using:"
		" HEAP_COMPARE<" QUOTE(HEAP_COMPARE) ">;"
#ifdef HEAP_VALUE
		" HEAP_VALUE<" QUOTE(HEAP_VALUE) ">;"
#endif
		" HEAP_TEST <" QUOTE(HEAP_TEST) ">;"
		" testing:\n");
	PH_(test_basic)();
	fprintf(stderr, "Done tests of <" QUOTE(HEAP_NAME) ">heap.\n\n");
}

#undef QUOTE
#undef QUOTE_

#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** Used for `HEAP_TEST`: either, `HEAP_VALUE` (*v)->p, or (v)->p, in which it
 is just a wasted parameter, the return value is what's important. */
#ifdef HEAP_VALUE
typedef pT_(priority) (*pT_(test_fn))(pT_(value) *);
#else
typedef pT_(priority) (*pT_(test_fn))(void);
#endif

/** Draw a graph of `heap` to `fn` in Graphviz format. */
static void pT_(graph)(const struct t_(heap) *const heap,
	const char *const fn) {
	FILE *fp;
	char a[12];
	size_t i;
	assert(heap && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\tgraph [truecolor=true, bgcolor=transparent, fontname=modern];\n"
		"\tnode [shape=box, style=filled, fillcolor=\"Gray95\","
		" fontname=modern];\n"
		/* Google search / Wikipedia says we should draw them with the top down.
		"\trankdir = BT;\n" */
		"\tedge [arrowhead = none];\n");
	for(i = 0; i < heap->as_array.size; i++) {
#ifdef HEAP_VALUE
		t_(to_string)(heap->as_array.data[i].priority,
			heap->as_array.data[i].value, &a);
#else
		t_(to_string)(heap->as_array.data + i, &a);
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
static void pT_(valid)(const struct t_(heap) *const heap) {
	size_t i;
	pT_(node) *n0;
	if(!heap) return;
	if(!(n0 = heap->as_array.data)) { assert(!heap->as_array.size); return; }
	for(i = 1; i < heap->as_array.size; i++) {
		size_t iparent = (i - 1) >> 1;
		if(t_(less)(pT_(get_priority)(n0 + iparent),
			pT_(get_priority)(n0 + i)) <= 0) continue;
		pT_(graph)(heap, "graph/" QUOTE(BOX_NAME) "-invalid.gv");
		assert(0);
		break;
	}
}

/** Fills `node` whether it has `HEAP_VALUE` or not. */
static void pT_(fill)(pT_(node) *const node) {
#ifdef HEAP_VALUE
	t_(filler)(&node->priority, &node->value);
#else
	t_(filler)(node);
#endif
}

static void pT_(test_basic)(void) {
	struct t_(heap) heap = t_(heap)(), merge = t_(heap)();
	pT_(node) add, *node;
	pT_(value) v, x;
	pT_(priority) last_priority = 0;
	const size_t test_size_1 = 11, test_size_2 = 31, test_size_3 = 4000/*0*/;
	size_t i, cum_size = 0;
	char fn[64];
	int success, ret;

	printf("Test empty.\n");
	pT_(valid)(0);
	errno = 0;
	assert(!heap.as_array.size);
	t_(heap_)(&heap);
	assert(!heap.as_array.size);
	assert(!T_(peek)(&heap));
	/*assert(!T_(pop)(&heap));*/
	pT_(valid)(&heap);
	assert(!errno);

	printf("Test one.\n");
	pT_(fill)(&add);
	v = pT_(get_value)(&add);
	ret = T_(add)(&heap, add), assert(ret), cum_size++;
	printf("Added one, %s.\n", T_(to_string)(&heap));
	assert(heap.as_array.size == cum_size);
	node = T_(peek)(&heap);
	pT_(valid)(&heap);
	x = T_(pop)(&heap), cum_size--;
	assert(v == x && heap.as_array.size == cum_size);
	assert(pT_(get_value)(node) == x);
	pT_(valid)(&heap);

	printf("Test many.\n");
	for(i = 0; i < test_size_1; i++) {
		if(!i || !(i & (i - 1))) {
			sprintf(fn, "graph/" QUOTE(BOX_NAME) "-%lu.gv",
				(unsigned long)cum_size);
			pT_(graph)(&heap, fn);
		}
		pT_(valid)(&heap);
		pT_(fill)(&add);
		success = T_(add)(&heap, add), cum_size++;
		assert(success);
	}
	sprintf(fn, "graph/" QUOTE(BOX_NAME) "-%lu-done-1.gv",
		(unsigned long)cum_size);
	pT_(graph)(&heap, fn);
	assert(heap.as_array.size == cum_size);
	printf("Heap: %s.\n", T_(to_string)(&heap));
	printf("Heap buffered add, before size = %lu.\n",
		(unsigned long)heap.as_array.size);
	node = T_(buffer)(&heap, test_size_2);
	assert(node);
	for(i = 0; i < test_size_2; i++) pT_(fill)(node + i);
	T_(append)(&heap, test_size_2), cum_size += test_size_2;
	printf("Now size = %lu.\n", (unsigned long)heap.as_array.size);
	assert(heap.as_array.size == cum_size);
	sprintf(fn, "graph/" QUOTE(BOX_NAME) "-%lu-buffer.gv", cum_size);
	pT_(graph)(&heap, fn);
	pT_(valid)(&heap);
	for(i = 0; i < test_size_3; i++) {
		if(!i || !(i & (i - 1))) {
			sprintf(fn, "graph/" QUOTE(BOX_NAME) "-%lu.gv",
				(unsigned long)cum_size);
			pT_(graph)(&heap, fn);
		}
		pT_(valid)(&heap);
		pT_(fill)(&add);
		success = T_(add)(&heap, add), cum_size++;
		assert(success);
	}
	sprintf(fn, "graph/" QUOTE(BOX_NAME) "-%lu-heap.gv",
		(unsigned long)cum_size);
	pT_(graph)(&heap, fn);
	printf("Setting up merge at %lu.\n", (unsigned long)cum_size);
	for(i = 0; i < test_size_1; i++) {
		pT_(fill)(&add);
		success = T_(add)(&merge, add);
		assert(success);
	}
	sprintf(fn, "graph/" QUOTE(BOX_NAME) "-%lu-merge.gv",
		(unsigned long)cum_size);
	pT_(graph)(&merge, fn);
	pT_(valid)(&merge);
	success = T_(affix)(&heap, &merge), cum_size += merge.as_array.size;
	sprintf(fn, "graph/" QUOTE(BOX_NAME) "-%lu-combined.gv",
		(unsigned long)cum_size);
	pT_(graph)(&heap, fn);
	assert(success && heap.as_array.size == cum_size);
	pT_(valid)(&heap);
	printf("Final heap: %s.\n", T_(to_string)(&heap));
	for(i = cum_size; i > 0; i--) {
		char z[12];
		node = T_(peek)(&heap);
		assert(node);
#ifdef HEAP_VALUE
		t_(to_string)(node->priority, node->value, &z);
#else
		t_(to_string)(node, &z);
#endif
		T_(pop)(&heap);
		if(!i || !(i & (i - 1))) {
			printf("%lu: retreving %s.\n", (unsigned long)i, z);
			sprintf(fn, "graph/" QUOTE(BOX_NAME) "-remove-%lu.gv",
				(unsigned long)i);
			pT_(graph)(&heap, fn);
		}
		assert(heap.as_array.size == i - 1 && T_(size)(&heap) == i - 1);
		pT_(valid)(&heap);
		if(i != cum_size)
			assert(t_(less)(last_priority, pT_(get_priority)(node)) <= 0);
		last_priority = pT_(get_priority)(node);
	}
	printf("Destructor:\n");
	t_(heap_)(&merge);
	t_(heap_)(&heap);
	assert(!T_(peek)(&heap));
}

/** Will be tested on stdout. Requires `HEAP_TEST`, `BOX_TO_STRING`, and not
 `NDEBUG` while defining `assert`.
 @param[param] The `void *` parameter in `HEAP_TEST`. Can be null. @allow */
static void T_(test)(void) {
	printf("<" QUOTE(HEAP_NAME) ">heap"
		" of priority type <" QUOTE(HEAP_TYPE) ">"
		" was created using:"
#ifdef HEAP_VALUE
		" HEAP_VALUE<" QUOTE(HEAP_VALUE) ">;"
#endif
		" HEAP_TO_STRING; HEAP_TEST;"
		" testing:\n");
	pT_(test_basic)();
	fprintf(stderr, "Done tests of <" QUOTE(HEAP_NAME) ">heap.\n\n");
}

#undef QUOTE
#undef QUOTE_

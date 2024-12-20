#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

typedef pT_(priority) (*pT_(test_fn))(void);

/** Makes sure the `heap` is in a valid state. */
static void pT_(valid)(const struct t_(heap) *const heap) {
	size_t i;
	pT_(priority) *n0;
	if(!heap) return;
	if(!(n0 = heap->as_array.data)) { assert(!heap->as_array.size); return; }
	for(i = 1; i < heap->as_array.size; i++) {
		size_t iparent = (i - 1) >> 1;
		if(t_(less)(n0[iparent], n0[i]) <= 0) continue;
		pT_(graph)(heap, "graph/heap/" QUOTE(HEAP_NAME) "-invalid.gv");
		assert(0);
		break;
	}
}

static void pT_(test_basic)(void) {
	struct t_(heap) heap = t_(heap)(), merge = t_(heap)();
	pT_(priority) add, *node, x;
#ifndef TEST_HEAP_ZERO
	pT_(priority) last_priority = 0;
#else
	pT_(priority) last_priority = TEST_HEAP_ZERO;
#	undef TEST_HEAP_ZERO
#endif
	const size_t test_size_1 = 11, test_size_2 = 31, test_size_3 = 4000/*0*/;
	size_t i, cum_size = 0;
	char fn[128];
	int success, ret;

	printf("Test empty.\n");
	pT_(valid)(0);
	errno = 0;
	assert(!heap.as_array.size);
	t_(heap_)(&heap);
	assert(!heap.as_array.size);
	assert(!T_(peek)(&heap));
	/*assert(!T_(pop)(&heap)); No. */
	pT_(valid)(&heap);
	assert(!errno);

	printf("Test one.\n");
	t_(filler)(&add);
	ret = T_(add)(&heap, add), assert(ret), cum_size++;
	printf("Added one, %s.\n", T_(to_string)(&heap));
	assert(heap.as_array.size == cum_size);
	node = T_(peek)(&heap);
	pT_(valid)(&heap);
	x = T_(pop)(&heap), cum_size--;
	assert(!t_(less)(*node, add) && !t_(less)(add, *node)
		&& !t_(less)(add, x) && !t_(less)(x, add)
		&& heap.as_array.size == cum_size);
	pT_(valid)(&heap);

	printf("Test many.\n");
	for(i = 0; i < test_size_1; i++) {
		if(!i || !(i & (i - 1))) {
			sprintf(fn, "graph/heap/" QUOTE(HEAP_NAME) "-%lu.gv",
				(unsigned long)cum_size);
			pT_(graph)(&heap, fn);
		}
		pT_(valid)(&heap);
		t_(filler)(&add);
		success = T_(add)(&heap, add), cum_size++;
		assert(success);
	}
	sprintf(fn, "graph/heap/" QUOTE(HEAP_NAME) "-%lu-done-1.gv",
		(unsigned long)cum_size);
	pT_(graph)(&heap, fn);
	assert(heap.as_array.size == cum_size);
	printf("Heap: %s.\n", T_(to_string)(&heap));
	printf("Heap buffered add, before size = %lu.\n",
		(unsigned long)heap.as_array.size);
	node = T_(buffer)(&heap, test_size_2);
	assert(node);
	for(i = 0; i < test_size_2; i++) t_(filler)(node + i);
	T_(append)(&heap, test_size_2), cum_size += test_size_2;
	printf("Now size = %lu.\n", (unsigned long)heap.as_array.size);
	assert(heap.as_array.size == cum_size);
	sprintf(fn, "graph/heap/" QUOTE(HEAP_NAME) "-%lu-buffer.gv", cum_size);
	pT_(graph)(&heap, fn);
	pT_(valid)(&heap);
	for(i = 0; i < test_size_3; i++) {
		if(!i || !(i & (i - 1))) {
			sprintf(fn, "graph/heap/" QUOTE(HEAP_NAME) "-%lu.gv",
				(unsigned long)cum_size);
			pT_(graph)(&heap, fn);
		}
		pT_(valid)(&heap);
		t_(filler)(&add);
		success = T_(add)(&heap, add), cum_size++;
		assert(success);
	}
	sprintf(fn, "graph/heap/" QUOTE(HEAP_NAME) "-%lu-heap.gv",
		(unsigned long)cum_size);
	pT_(graph)(&heap, fn);
	printf("Setting up merge at %lu.\n", (unsigned long)cum_size);
	for(i = 0; i < test_size_1; i++) {
		t_(filler)(&add);
		success = T_(add)(&merge, add);
		assert(success);
	}
	sprintf(fn, "graph/heap/" QUOTE(HEAP_NAME) "-%lu-merge.gv",
		(unsigned long)cum_size);
	pT_(graph)(&merge, fn);
	pT_(valid)(&merge);
	success = T_(affix)(&heap, &merge), cum_size += merge.as_array.size;
	sprintf(fn, "graph/heap/" QUOTE(HEAP_NAME) "-%lu-combined.gv",
		(unsigned long)cum_size);
	pT_(graph)(&heap, fn);
	assert(success && heap.as_array.size == cum_size);
	pT_(valid)(&heap);
	printf("Final heap: %s.\n", T_(to_string)(&heap));
	for(i = cum_size; i > 0; i--) {
		char z[12];
		node = T_(peek)(&heap);
		assert(node);
		t_(to_string)((const void *)node, &z);
		T_(pop)(&heap);
		if(!i || !(i & (i - 1))) {
			printf("%lu: retreving %s.\n", (unsigned long)i, z);
			sprintf(fn, "graph/heap/" QUOTE(HEAP_NAME) "-remove-%lu.gv",
				(unsigned long)i);
			pT_(graph)(&heap, fn);
		}
		assert(heap.as_array.size == i - 1 && T_(size)(&heap) == i - 1);
		pT_(valid)(&heap);
		if(i != cum_size)
			assert(t_(less)(last_priority, *node) <= 0);
		last_priority = *node;
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
		" was created using: HEAP_TO_STRING; HEAP_TEST; testing:\n");
	pT_(test_basic)();
	fprintf(stderr, "Done tests of <" QUOTE(HEAP_NAME) ">heap.\n\n");
}

#undef QUOTE
#undef QUOTE_

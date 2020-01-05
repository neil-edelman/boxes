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





#if 0

/** Draw a graph of `ar` to `fn` in Graphviz format. */
static void PT_(graph)(const struct T_(Array) *const ar, const char *const fn) {
	FILE *fp;
	char a[12];
	assert(ar && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
			"\trankdir = LR;\n"
			"\tnode [shape = record, style = filled];\n"
			"\tArray [label=\"\\<" QUOTE(ARRAY_NAME) "\\>Array: "
			QUOTE(ARRAY_TYPE) "\\l|size: %lu\\lcapacity: %lu\\l"
			"next capacity: %lu\\l\"];\n", (unsigned long)ar->size,
			(unsigned long)ar->capacity, (unsigned long)ar->next_capacity);
	if(ar->data) {
		T *const data = ar->data;
		size_t i;
		fprintf(fp, "\tnode [fillcolor=lightsteelblue];\n"
			"\tArray -> p%p;\n"
			"\tsubgraph cluster_data {\n"
			"\t\tstyle=filled;\n", (void *)data);
		for(i = 0; i < ar->size; i++) {
			PT_(to_string)(data + i, &a);
			fprintf(fp, "\t\tp%p [label=\"%s\"];\n", (void *)(data + i), a);
		}
		fprintf(fp, "\t}\n");
	}
	fprintf(fp, "\tnode [colour=red];\n");
	fprintf(fp, "}\n");
	fclose(fp);
}
#endif

static void PH_(test_basic)(void) {
	struct H_(Heap) heap = HEAP_IDLE;

	printf("Test null.\n");
	errno = 0;
	assert(!H_(HeapSize)(&heap));
	H_(Heap_)(&heap);
	assert(!H_(HeapSize)(&heap));
	H_(Heap)(&heap);
	assert(!H_(HeapSize)(&heap));
	assert(!errno);
	errno = 0;
	T_(Array_)(0);
	T_(Array)(0);
	T_(ArrayClear)(0);
	assert(T_(ArrayGet)(0) == 0);
	assert(T_(ArrayPeek)(0) == 0);
	assert(T_(ArrayPop)(0) == 0);
	assert(T_(ArrayNext)(0, 0) == 0);
	assert(T_(ArrayNew)(0) == 0);
	assert(T_(ArrayUpdateNew)(0, 0) == 0
		&& T_(ArrayUpdateNew)(0, 0) == 0);
	T_(ArrayEach)(0, 0);
	T_(ArrayTrim)(0, 0);
	assert(!strcmp("null", T_(ArrayToString(0))));
	assert(errno == 0);
	PT_(valid_state)(0);

	printf("Test empty.\n");
	T_(Array)(&a);
	t = (T *)1;

	printf("Clear:\n");
	T_(ArrayClear)(&a);
	printf("%s.\n", T_(ArrayToString)(&a));
	assert(T_(ArrayPeek)(&a) == 0);
	
	printf("Destructor:\n");
	T_(Array_)(&a);
	assert(T_(ArrayPeek)(&a) == 0);
	PT_(valid_state)(&a);
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

/** Replace has it's own test. */
static void PT_(test_replace)(void) {
	T ts[5], *t, *t1;
	const size_t ts_size = sizeof ts / sizeof *ts;
	struct T_(Array) a, b;
	T *e;
	int success;

	/* valgrind does not like this. */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	for(t = ts, t1 = t + ts_size; t < t1; t++) PT_(filler)(t);
	printf("Test replace.\n");
	T_(Array)(&a);
	for(t = ts, t1 = t + ts_size; t < t1; t++) {
		e = T_(ArrayNew)(&a), assert(e);
		memcpy(e, t, sizeof *t);
	}
	assert(T_(ArraySize)(&a) == ts_size);	
	T_(Array)(&b);
	/* Passing in null. */
	errno = 0;
	success = T_(ArrayIndexSplice)(0, 0, 0, 0);
	assert(!success && !errno);
	success = T_(ArraySplice)(0, 0, 0, 0);
	assert(!success && !errno);
	/* a == b. */
	success = T_(ArrayIndexSplice)(&a, 0, 0, &a);
	assert(!success && errno == EDOM);
	errno = 0;
	success = T_(ArraySplice)(&a, 0, 0, &a);
	assert(!success && errno == EDOM);
	errno = 0;
	/* Out-of-bounds. */
	success = T_(ArrayIndexSplice)(&a, 0, T_(ArraySize)(&a) + 1, 0);
	assert(!success && errno == EDOM);
	errno = 0;
	/* Large */
	success = T_(ArraySplice)(&a, 0, 65536, 0);
	assert(!success && errno == ERANGE);
	errno = 0;
	/* e0 > e1. */
	success = T_(ArrayIndexSplice)(&a, 1, 0, &b);
	assert(!success && errno == EDOM);
	errno = 0;
	/* No-op. */
	success = T_(ArrayIndexSplice)(&a, 0, 0, 0);
	printf("Array %s.\n", T_(ArrayToString)(&a));
	assert(success && T_(ArraySize)(&a) == ts_size);
	/* Deleting from the front. */
	success = T_(ArrayIndexSplice)(&a, 0, 1, 0);
	printf("Array after deleting from front %s.\n", T_(ArrayToString)(&a));
	assert(success && T_(ArraySize)(&a) == ts_size - 1);
	/* Adding at the back. */
	t = T_(ArrayNew)(&b);
	assert(t);
	memcpy(t, ts + 0, sizeof *t);
	success = T_(ArrayIndexSplice)(&a, T_(ArraySize)(&a), T_(ArraySize)(&a),
		&b);
	printf("Array after adding %s to back %s.\n", T_(ArrayToString)(&b),
		T_(ArrayToString)(&a));
	assert(success && T_(ArraySize)(&a) == ts_size);
	/* Replacing same-size. */
	success = T_(ArrayIndexSplice)(&a, 1, 2, &b);
	printf("Array after replacing [1, 2) %s: %s.\n", T_(ArrayToString)(&b),
		T_(ArrayToString)(&a));
	assert(success && T_(ArraySize)(&a) == ts_size
		&& !memcmp(t, T_(ArrayGet)(&a) + 1, sizeof *t));
	/* Replacing larger size. */
	t = T_(ArrayNew)(&b);
	assert(t);
	memcpy(t, ts + 1, sizeof *t);
	success = T_(ArrayIndexSplice)(&a, 1, 2, &b);
	printf("Array after replacing [1, 2) %s: %s.\n", T_(ArrayToString)(&b),
		T_(ArrayToString)(&a));
	assert(success && T_(ArraySize)(&a) == ts_size + 1
		   && !memcmp(t, T_(ArrayGet)(&a) + 2, sizeof *t));
	/* Replacing a smaller size. */
	success = T_(ArrayIndexSplice)(&a, 1, 4, &b);
	printf("Array after replacing [1, 4) %s: %s.\n", T_(ArrayToString)(&b),
		T_(ArrayToString)(&a));
	assert(success && T_(ArraySize)(&a) == ts_size
		   && !memcmp(t, T_(ArrayGet)(&a) + 2, sizeof *t));
	T_(ArrayClear)(&b);
	t = T_(ArrayBuffer)(&b, 2);
	assert(t);
	memcpy(t, ts + 2, sizeof *t * 2);
	assert(T_(ArraySize)(&b) == 2);
	/* a = [[1],[0],[1],[4],[0]]; b = [[2],[3]] */
	printf("a = %s, b = %s.\n", T_(ArrayToString)(&a), T_(ArrayToString)(&b));
	T_(ArraySplice)(&a, 0, -1, &b);
	printf("a = %s.\n", T_(ArrayToString)(&a));
	/* a = [[1],[0],[1],[4],[0],[2],[3]] */
	T_(ArraySplice)(&a, T_(ArrayGet)(&a) + 1, 2, &b);
	printf("a = %s.\n", T_(ArrayToString)(&a));
	/* a = [[1],[2],[3],[4],[0],[2],[3]] */
	T_(ArraySplice)(&a, T_(ArrayGet)(&a) + 2, -5, &b);
	printf("a = %s.\n", T_(ArrayToString)(&a));
	/* a = [[1],[2],[2],[3],[4],[0],[2],[3]] */
	T_(ArraySplice)(&a, T_(ArrayGet)(&a) + 7, -1, &b);
	printf("a = %s.\n", T_(ArrayToString)(&a));
	/* a = [[1],[2],[2],[3],[4],[0],[2],[2],[3]] */
	/* @fixme This is not enought coverage. */
	assert(T_(ArraySize)(&a) == 9 &&
		!memcmp(ts + 1, T_(ArrayGet)(&a), sizeof *t * 2) &&
		!memcmp(ts + 2, T_(ArrayGet)(&a) + 2, sizeof *t * 3) &&
		!memcmp(ts + 0, T_(ArrayGet)(&a) + 5, sizeof *t) &&
		!memcmp(ts + 2, T_(ArrayGet)(&a) + 6, sizeof *t) &&
		!memcmp(ts + 2, T_(ArrayGet)(&a) + 7, sizeof *t * 2));
	T_(Array_)(&b);
	T_(Array_)(&a);
}

/** @implements <PT>Predicate
 @return A set sequence of ones and zeros, independant of `data`. */
static int PT_(keep_one)(const T *const data) {
	static size_t i;
	static const int things[] = { 1,0,0,0,0,1,0,0,1,1,0,1,0,1,0,1,0 };
	const int predicate = things[i++];
	(void)data;
	i %= sizeof things / sizeof *things;
	return predicate;
}

static void PT_(test_keep)(void) {
	T ts[17], *t, *t1, *e;
	const size_t ts_size = sizeof ts / sizeof *ts;
	struct T_(Array) a = ARRAY_IDLE;
	/* Valgrind. */
	memset(ts, 0, sizeof ts);
	PT_(valid_state)(&a);
	for(t = ts, t1 = t + ts_size; t < t1; t++) {
		PT_(filler)(t);
		e = T_(ArrayNew)(&a), assert(e);
		memcpy(e, t, sizeof *t);
	}
	T_(ArrayKeepIf)(&a, &PT_(keep_one), 0);
	assert(T_(ArraySize)(&a) == 7
		&& !memcmp(ts + 0, T_(ArrayGet)(&a) + 0, sizeof *t * 1)
		&& !memcmp(ts + 5, T_(ArrayGet)(&a) + 1, sizeof *t * 1)
		&& !memcmp(ts + 8, T_(ArrayGet)(&a) + 2, sizeof *t * 2)
		&& !memcmp(ts + 11, T_(ArrayGet)(&a) + 4, sizeof *t * 1)
		&& !memcmp(ts + 13, T_(ArrayGet)(&a) + 5, sizeof *t * 1)
		&& !memcmp(ts + 15, T_(ArrayGet)(&a) + 6, sizeof *t * 1));
	T_(Array_)(&a);
}

static int PT_(num);

/** Increments a global variable, independent of `t`.
 @implements <PT>Action */
static void PT_(increment)(T *const t) {
	(void)t;
	PT_(num)++;
}

/** True, independent of `t`.
 @implements <PT>Predicate */
static int PT_(true)(const T *const t) {
	(void)t;
	return 1;
}

static void PT_(test_each)(void) {
	struct T_(Array) empty = ARRAY_IDLE, one = ARRAY_IDLE;
	T *t;
	t = T_(ArrayNew)(&one);
	assert(t);
	if(!t) return;
	PT_(num) = 0;
	T_(ArrayEach)(&empty, &PT_(increment));
	assert(!PT_(num));
	T_(ArrayEach)(&one, &PT_(increment));
	assert(PT_(num) == 1);
	PT_(num) = 0;
	T_(ArrayIfEach)(&empty, &PT_(true), &PT_(increment));
	assert(!PT_(num));
	T_(ArrayIfEach)(&one, &PT_(true), &PT_(increment));
	assert(PT_(num) == 1);
	PT_(num) = 0;
	t = T_(ArrayAny)(&empty, &PT_(true));
	assert(!t);
	t = T_(ArrayAny)(&one, &PT_(true));
	assert(t == T_(ArrayGet)(&one));
	T_(Array_)(&one);
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

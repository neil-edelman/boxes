/* Intended to be included by `Array.h` on `ARRAY_TEST`. */

/* Define macros. */
#ifdef QUOTE
#undef QUOTE
#endif
#ifdef QUOTE_
#undef QUOTE_
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)



/* ARRAY_TEST must be a function that implements <PT>Action. */
static const PT_(Action) PT_(filler) = (ARRAY_TEST);



/** @return Is `a` in a valid state? */
static void PT_(valid_state)(const struct T_(Array) *const a) {
	const size_t max_size = (size_t)-1 / sizeof *a->data;
	/* Null is a valid state. */
	if(!a) return;
	if(!a->data) { assert(!a->size); return; }
	assert(a->size <= a->capacity && a->capacity <= max_size);
}

/** @implements <PT>Predicate
 @return Is `t` zero-filled? */
static int PT_(zero_filled)(const T *const t) {
	const char *c = (const char *)t,
		*const end = (const char *)(t + 1);
	assert(t);
	while(c < end) if(*c++) return 0;
	return 1;
}



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
		QUOTE(ARRAY_TYPE) "\\l|size: %lu\\lcapacity: %lu\\l\"];\n",
		(unsigned long)ar->size, (unsigned long)ar->capacity);
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

static void PT_(test_basic)(void) {
	struct T_(Array) a;
	T ts[5], *t, *t1, *start;
	const size_t ts_size = sizeof ts / sizeof *ts, big = 1000;
	size_t i;
	int is_zero;

	printf("Test null.\n");
	errno = 0;
	T_(Array_)(0);
	T_(Array)(0);
	assert(T_(ArrayRemove)(0, 0) == 0);
	assert(T_(ArrayLazyRemove)(0, 0) == 0);
	T_(ArrayClear)(0);
	assert(T_(ArrayPeek)(0) == 0);
	assert(T_(ArrayPop)(0) == 0);
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
	assert(T_(ArrayRemove)(&a, 0) == 0 && errno == 0);
	assert(T_(ArrayRemove)(&a, t) == 0 && errno == EDOM), errno = 0;
	assert(T_(ArrayLazyRemove)(&a, 0) == 0 && errno == 0);
	assert(T_(ArrayLazyRemove)(&a, t) == 0 && errno == EDOM), errno = 0;
	assert(a.data == 0);
	assert(T_(ArrayPeek)(0) == 0);
	assert(T_(ArrayPop)(0) == 0);
	T_(ArrayEach)(&a, 0);
	assert(errno == 0);
	PT_(valid_state)(&a);

	/* @fixme valgrind is giving me grief if I don't do this? */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	for(t = ts, t1 = t + ts_size; t < t1; t++) PT_(filler)(t);

	printf("Test one element.\n");
	t = T_(ArrayNew)(&a); /* Add. */
	assert(t && a.data == t && a.size == 1);
	assert(T_(ArrayPeek)(&a) == t);
	t1 = T_(ArrayPop)(&a); /* Remove. */
	assert(t1 == t);
	assert(T_(ArrayPeek)(&a) == 0);
	t = T_(ArrayNew)(&a); /* Add. */
	assert(t && a.size == 1 && a.capacity == 8);
	T_(ArrayShrink)(&a);
	assert(a.size == 1 && a.capacity == 1);
	t = T_(ArrayNew)(&a); /* Add 2. */
	assert(t && a.size == 2 && a.capacity == 8);
	T_(ArrayClear)(&a);
	assert(T_(ArrayPeek)(&a) == 0);
	PT_(valid_state)(&a);

	printf("Testing lazy remove.\n");
	assert(ts_size >= 3);
	for(i = 0; i < 3; i++) {
		t = T_(ArrayNew)(&a);
		assert(t);
		memcpy(t, ts + i, sizeof *t);
	}
	T_(ArrayLazyRemove)(&a, a.data);
	assert(a.size == 2);
	t = a.data;
	assert(!memcmp(t, ts + 2, sizeof *t) && !memcmp(t + 1, ts + 1, sizeof *t));
	T_(ArrayClear)(&a);
	assert(!a.size);

	printf("Testing %lu elements.\n", (unsigned long)ts_size);
	for(i = 0; i < ts_size; i++) {
		t = T_(ArrayNew)(&a);
		assert(t);
		memcpy(t, ts + i, sizeof *t);
	}
	assert(T_(ArrayPeek)(&a));
	printf("Now: %s.\n", T_(ArrayToString)(&a));
	assert(a.size == ts_size);
	if((t = a.data + ts_size - 2)
		&& !T_(ArrayRemove)(&a, t)) {
		perror("Error"), assert(0);
		return;
	}
	printf("Now: %s.\n", T_(ArrayToString)(&a));
	assert(!T_(ArrayRemove)(&a, t + 1) && errno == EDOM);
	printf("(Deliberate) error: %s.\n", strerror(errno)), errno = 0;
	if((t = a.data + ts_size - 3)
		&& !T_(ArrayRemove)(&a, t)) {
		perror("Error"), assert(0);
		return;
	}
	printf("Now: %s.\n", T_(ArrayToString)(&a));
	assert(!T_(ArrayRemove)(&a, t + 1) && errno == EDOM);
	printf("(Deliberate) error: %s.\n", strerror(errno)), errno = 0;

	assert(a.size == ts_size - 2);
	T_(ArrayNew)(&a);
	T_(ArrayNew)(&a);
	memcpy(t + 1, ts + 3, sizeof *t * 2);
	assert(a.size == ts_size);
	PT_(valid_state)(&a);
	printf("Now: %s.\n", T_(ArrayToString)(&a));

	/* Peek/Pop. */
	t = T_(ArrayPeek)(&a);
	assert(t && !memcmp(t, ts + ts_size - 1, sizeof *t));
	t = T_(ArrayPop)(&a);
	assert(t && !memcmp(t, ts + ts_size - 1, sizeof *t));
	t = T_(ArrayPop)(&a);
	assert(t && !memcmp(t, ts + ts_size - 2, sizeof *t));
	T_(ArrayClear)(&a);
	assert(a.size == 0);

	/* Trim 1. */
	t = T_(ArrayNew)(&a);
	assert(t);
	memset(t, 0, sizeof *t);
	T_(ArrayTrim)(&a, &PT_(zero_filled));
	assert(a.size == 0);
	/* Trim 3. */
	t = T_(ArrayNew)(&a);
	assert(t);
	memset(t, 0, sizeof *t);
	t = T_(ArrayNew)(&a);
	assert(t);
	PT_(filler)(t);
	is_zero = PT_(zero_filled)(t);
	t = T_(ArrayNew)(&a);
	assert(t);
	memset(t, 0, sizeof *t);
	T_(ArrayTrim)(&a, &PT_(zero_filled));
	assert(a.size == !is_zero);

	/* Big. */
	start = a.data + a.size;
	for(i = 0; i < big; i++) {
		t = T_(ArrayUpdateNew)(&a, &start);
		assert(t && (size_t)(t - start) == i);
		PT_(filler)(t);
	}
	printf("%s.\n", T_(ArrayToString)(&a));
	PT_(valid_state)(&a);

	printf("Clear:\n");
	T_(ArrayClear)(&a);
	printf("%s.\n", T_(ArrayToString)(&a));
	assert(T_(ArrayPeek)(&a) == 0);
	
	printf("Destructor:\n");
	T_(Array_)(&a);
	assert(T_(ArrayPeek)(&a) == 0);
	PT_(valid_state)(&a);
}

/** Reproducable non-sense `a` `b`. @implements Bipredicate */
static int PT_(equal_byte)(const T *const a, const T *const b)
	{ return *(char *)a == *(char *)b; }

/** Add onto `a` `value` x `repetitions` (non-sense.) */
static int PT_(fill_garbage)(struct T_(Array) *const a,
	const char value, const size_t repetitions) {
	T *t;
	size_t r = 0;
	assert(a && repetitions);
	if(!(t = T_(ArrayBuffer)(a, repetitions))) return 0;
	memset(t, 0, sizeof *t * repetitions);
	while(r < repetitions) *(char *)(t + r++) = value;
	return 1;
}

/** Tests compress, but half-way because we don't know the specifics. */
static void PT_(test_compactify)(void) {
	struct T_(Array) a = ARRAY_IDLE;
	assert(PT_(fill_garbage)(&a, 'a', 3) && PT_(fill_garbage)(&a, 'b', 3)
		&& PT_(fill_garbage)(&a, 'c', 3));
	assert(a.size == 9);
	T_(ArrayCompactify)(&a, &PT_(equal_byte), 0);
	assert(a.size == 3 && *(char *)a.data == 'a'
		&& *(char *)(a.data + 1) == 'b' && *(char *)(a.data + 2) == 'c');
	T_(Array_)(&a);
}

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
			double t = 0.5;
			r = rand() / (RAND_MAX + 1.0);
			if(r < t) {
				data = T_(ArrayPeek)(&a);
				assert(data);
				PT_(to_string)(data, &str);
				printf("popping %s.\n", str);
				assert(data == T_(ArrayPop)(&a));
			} else {
				size_t idx = rand() / (RAND_MAX + 1.0) * size;
				if(!(data = a.data + idx)) continue;
				PT_(to_string)(data, &str);
				printf("removing %s at %lu.\n", str, (unsigned long)idx);
				{
					const int ret = T_(ArrayRemove)(&a, data);
					assert(ret || (perror("Removing"), 0));
				}
			}
			size--;
		}
		PT_(valid_state)(&a);
		if(a.size < 1000000 && !(i & (i - 1))) {
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
	assert(a.size == ts_size);	
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
	success = T_(ArrayIndexSplice)(&a, 0, a.size + 1, 0);
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
	assert(success && a.size == ts_size);
	/* Deleting from the front. */
	success = T_(ArrayIndexSplice)(&a, 0, 1, 0);
	printf("Array after deleting from front %s.\n", T_(ArrayToString)(&a));
	assert(success && a.size == ts_size - 1);
	/* Adding at the back. */
	t = T_(ArrayNew)(&b);
	assert(t);
	memcpy(t, ts + 0, sizeof *t);
	success = T_(ArrayIndexSplice)(&a, a.size, a.size,
		&b);
	printf("Array after adding %s to back %s.\n", T_(ArrayToString)(&b),
		T_(ArrayToString)(&a));
	assert(success && a.size == ts_size);
	/* Replacing same-size. */
	success = T_(ArrayIndexSplice)(&a, 1, 2, &b);
	printf("Array after replacing [1, 2) %s: %s.\n", T_(ArrayToString)(&b),
		T_(ArrayToString)(&a));
	assert(success && a.size == ts_size
		&& !memcmp(t, a.data + 1, sizeof *t));
	/* Replacing larger size. */
	t = T_(ArrayNew)(&b);
	assert(t);
	memcpy(t, ts + 1, sizeof *t);
	success = T_(ArrayIndexSplice)(&a, 1, 2, &b);
	printf("Array after replacing [1, 2) %s: %s.\n", T_(ArrayToString)(&b),
		T_(ArrayToString)(&a));
	assert(success && a.size == ts_size + 1
		   && !memcmp(t, a.data + 2, sizeof *t));
	/* Replacing a smaller size. */
	success = T_(ArrayIndexSplice)(&a, 1, 4, &b);
	printf("Array after replacing [1, 4) %s: %s.\n", T_(ArrayToString)(&b),
		T_(ArrayToString)(&a));
	assert(success && a.size == ts_size
		   && !memcmp(t, a.data + 2, sizeof *t));
	T_(ArrayClear)(&b);
	t = T_(ArrayBuffer)(&b, 2);
	assert(t);
	memcpy(t, ts + 2, sizeof *t * 2);
	assert(b.size == 2);
	/* a = [[1],[0],[1],[4],[0]]; b = [[2],[3]] */
	printf("a = %s, b = %s.\n", T_(ArrayToString)(&a), T_(ArrayToString)(&b));
	T_(ArraySplice)(&a, 0, -1, &b);
	printf("a = %s.\n", T_(ArrayToString)(&a));
	/* a = [[1],[0],[1],[4],[0],[2],[3]] */
	T_(ArraySplice)(&a, a.data + 1, 2, &b);
	printf("a = %s.\n", T_(ArrayToString)(&a));
	/* a = [[1],[2],[3],[4],[0],[2],[3]] */
	T_(ArraySplice)(&a, a.data + 2, -5, &b);
	printf("a = %s.\n", T_(ArrayToString)(&a));
	/* a = [[1],[2],[2],[3],[4],[0],[2],[3]] */
	T_(ArraySplice)(&a, a.data + 7, -1, &b);
	printf("a = %s.\n", T_(ArrayToString)(&a));
	/* a = [[1],[2],[2],[3],[4],[0],[2],[2],[3]] */
	/* @fixme This is not enought coverage. */
	assert(a.size == 9 &&
		!memcmp(ts + 1, a.data, sizeof *t * 2) &&
		!memcmp(ts + 2, a.data + 2, sizeof *t * 3) &&
		!memcmp(ts + 0, a.data + 5, sizeof *t) &&
		!memcmp(ts + 2, a.data + 6, sizeof *t) &&
		!memcmp(ts + 2, a.data + 7, sizeof *t * 2));
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
	assert(a.size == 7
		&& !memcmp(ts + 0, a.data + 0, sizeof *t * 1)
		&& !memcmp(ts + 5, a.data + 1, sizeof *t * 1)
		&& !memcmp(ts + 8, a.data + 2, sizeof *t * 2)
		&& !memcmp(ts + 11, a.data + 4, sizeof *t * 1)
		&& !memcmp(ts + 13, a.data + 5, sizeof *t * 1)
		&& !memcmp(ts + 15, a.data + 6, sizeof *t * 1));
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
	assert(t == one.data);
	T_(Array_)(&one);
}

/** Will be tested on stdout. Requires `ARRAY_TEST`, `ARRAY_TO_STRING`, and not
 `NDEBUG` while defining `assert`. */
static void T_(ArrayTest)(void) {
	printf("<" QUOTE(ARRAY_NAME) ">Array of type <" QUOTE(ARRAY_TYPE)
		"> was created using: "
#ifdef ARRAY_TO_STRING
		"ARRAY_TO_STRING <" QUOTE(ARRAY_TO_STRING) ">; "
#endif
		"ARRAY_TEST <" QUOTE(ARRAY_TEST) ">; "
		"testing:\n");
	PT_(test_basic)();
	PT_(test_compactify)();
	PT_(test_random)();
	PT_(test_replace)();
	PT_(test_keep)();
	PT_(test_each)();
	fprintf(stderr, "Done tests of <" QUOTE(ARRAY_NAME) ">Array.\n\n");
}

/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_

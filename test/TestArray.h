#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

#ifdef ARRAY_TO_STRING /* <!-- to string: Only one, tests all base code. */

/* Copy functions for later includes. */
static void (*PT_(to_string))(const PT_(type) *, char (*)[12])
	= (ARRAY_TO_STRING);
static const char *(*PT_(array_to_string))(const struct T_(array) *)
	= A_(to_string);

/* `ARRAY_TEST` must be a function that implements <typedef:<PT>action>. */
static const PT_(action) PT_(filler) = (ARRAY_TEST);

/** @return Is `a` in a valid state? */
static void PT_(valid_state)(const struct T_(array) *const a) {
	const size_t max_size = (size_t)-1 / sizeof *a->data;
	/* Null is a valid state. */
	if(!a) return;
	if(!a->data) { assert(!a->size); return; }
	assert(a->size <= a->capacity && a->capacity <= max_size);
}

/** @implements <PT>Predicate @return Is `t` zero-filled? */
static int PT_(zero_filled)(const PT_(type) *const t) {
	const char *c = (const char *)t, *const end = (const char *)(t + 1);
	assert(t);
	while(c < end) if(*c++) return 0;
	return 1;
}

/** Draw a graph of `ar` to `fn` in Graphviz format. */
static void PT_(graph)(const struct T_(array) *const ar, const char *const fn) {
	FILE *fp;
	char a[12];
	assert(ar && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\trankdir = LR;\n"
		"\tnode [shape = record, style = filled];\n"
		"\tArray [label=\"\\<" QUOTE(ARRAY_NAME) "\\>array: "
		QUOTE(ARRAY_TYPE) "\\l|size: %lu\\lcapacity: %lu\\l\"];\n",
		(unsigned long)ar->size, (unsigned long)ar->capacity);
	if(ar->data) {
		PT_(type) *const data = ar->data;
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
	struct T_(array) a;
	PT_(type) ts[5], *t, *t1, *start;
	const size_t ts_size = sizeof ts / sizeof *ts, big = 1000;
	size_t i;
	int is_zero;

	assert(errno == 0);
	PT_(valid_state)(0);

	printf("Test empty.\n");
	T_(array)(&a);
	t = (PT_(type) *)1;
	assert(errno == 0);
	PT_(valid_state)(&a);

	/* This is un-necessary, but `valgrind` reports an error if we don't. */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	for(t = ts, t1 = t + ts_size; t < t1; t++) PT_(filler)(t);

	printf("Test one element.\n");
	t = T_(array_new)(&a); /* Add. */
	assert(t && a.data == t && a.size == 1);
	assert(T_(array_peek)(&a) == t);
	t1 = T_(array_pop)(&a); /* Remove. */
	assert(t1 == t);
	assert(T_(array_peek)(&a) == 0);
	t = T_(array_new)(&a); /* Add. */
	assert(t && a.size == 1 && a.capacity == 8);
	T_(array_shrink)(&a);
	assert(a.size == 1 && a.capacity == 1);
	t = T_(array_new)(&a); /* Add 2. */
	assert(t && a.size == 2 && a.capacity == 8);
	T_(array_clear)(&a);
	assert(T_(array_peek)(&a) == 0);
	PT_(valid_state)(&a);

	printf("Testing lazy remove.\n");
	assert(ts_size >= 3);
	for(i = 0; i < 3; i++) {
		t = T_(array_new)(&a);
		assert(t);
		memcpy(t, ts + i, sizeof *t);
	}
	T_(array_lazy_remove)(&a, a.data);
	assert(a.size == 2);
	t = a.data;
	assert(!memcmp(t, ts + 2, sizeof *t) && !memcmp(t + 1, ts + 1, sizeof *t));
	T_(array_clear)(&a);
	assert(!a.size);

	printf("Testing %lu elements.\n", (unsigned long)ts_size);
	for(i = 0; i < ts_size; i++) {
		t = T_(array_new)(&a);
		assert(t);
		memcpy(t, ts + i, sizeof *t);
	}
	assert(T_(array_peek)(&a));
	printf("Now: %s.\n", PT_(array_to_string)(&a));
	assert(a.size == ts_size);
	t = a.data + ts_size - 2;
	T_(array_remove)(&a, t);
	t = a.data + ts_size - 3;
	T_(array_remove)(&a, t);
	printf("Now: %s.\n", PT_(array_to_string)(&a));

	assert(a.size == ts_size - 2);
	T_(array_buffer)(&a, 2);
	memcpy(t + 1, ts + 3, sizeof *t * 2);
	assert(a.size == ts_size);
	PT_(valid_state)(&a);
	printf("Now: %s.\n", PT_(array_to_string)(&a));

	/* Peek/Pop. */
	t = T_(array_peek)(&a);
	assert(t && !memcmp(t, ts + ts_size - 1, sizeof *t));
	t = T_(array_pop)(&a);
	assert(t && !memcmp(t, ts + ts_size - 1, sizeof *t));
	t = T_(array_pop)(&a);
	assert(t && !memcmp(t, ts + ts_size - 2, sizeof *t));
	T_(array_clear)(&a);
	assert(a.size == 0);

	/* Trim 1. */
	t = T_(array_new)(&a);
	assert(t);
	memset(t, 0, sizeof *t);
	T_(array_trim)(&a, &PT_(zero_filled));
	assert(a.size == 0);
	/* Trim 3. */
	t = T_(array_new)(&a);
	assert(t);
	memset(t, 0, sizeof *t);
	t = T_(array_new)(&a);
	assert(t);
	PT_(filler)(t);
	is_zero = PT_(zero_filled)(t);
	t = T_(array_new)(&a);
	assert(t);
	memset(t, 0, sizeof *t);
	T_(array_trim)(&a, &PT_(zero_filled));
	assert(a.size == !is_zero);

	/* Big. */
	start = a.data + a.size;
	for(i = 0; i < big; i++) {
		t = T_(array_update_new)(&a, &start);
		assert(t);
		PT_(filler)(t);
	}
	printf("%s.\n", PT_(array_to_string)(&a));
	PT_(valid_state)(&a);

	printf("Clear:\n");
	T_(array_clear)(&a);
	printf("%s.\n", PT_(array_to_string)(&a));
	assert(T_(array_peek)(&a) == 0 && a.size == 0);

	t = T_(array_buffer_before)(&a, 0, ts_size - 1);
	assert(t && a.size == ts_size - 1);
	memcpy(a.data, ts, sizeof ts - sizeof *ts);
	printf("Buffer t[ : -1]: %s.\n", PT_(array_to_string)(&a));
	t = T_(array_buffer_before)(&a, ts_size - 1, 1);
	memcpy(a.data + ts_size - 1, ts + ts_size - 1, sizeof *ts);
	printf("Buffer t[ : ]: %s.\n", PT_(array_to_string)(&a));
	assert(!memcmp(ts, a.data, sizeof ts));

	printf("Destructor:\n");
	T_(array_)(&a);
	assert(T_(array_peek)(&a) == 0);
	PT_(valid_state)(&a);
}

static void PT_(test_random)(void) {
	struct T_(array) a;
	const size_t mult = 1; /* For long tests. */
	/* This parameter controls how many iterations. */
	size_t i, i_end = 1000 * mult, size = 0;
	/* Random. */
	T_(array)(&a);
	for(i = 0; i < i_end; i++) {
		PT_(type) *data;
		char str[12];
		double r = rand() / (RAND_MAX + 1.0);
		int is_print = !(rand() / (RAND_MAX / 50 + 1));
		if(is_print) printf("%lu: ", (unsigned long)i);
		/* This parameter controls how big the pool wants to be. */
		if(r > size / (100.0 * mult)) {
			if(!(data = T_(array_new)(&a)))
				{ perror("Error"), assert(0); return; }
			size++;
			PT_(filler)(data);
			PT_(to_string)(data, &str);
			if(is_print) printf("created %s.\n", str);
		} else {
			double t = 0.5;
			r = rand() / (RAND_MAX + 1.0);
			if(r < t) {
				data = T_(array_peek)(&a);
				assert(data);
				PT_(to_string)(data, &str);
				if(is_print) printf("popping %s.\n", str);
				assert(data == T_(array_pop)(&a));
			} else {
				size_t idx = rand() / (RAND_MAX + 1.0) * size;
				if(!(data = a.data + idx)) continue;
				PT_(to_string)(data, &str);
				if(is_print)
					printf("removing %s at %lu.\n", str, (unsigned long)idx);
				T_(array_remove)(&a, data);
			}
			size--;
		}
		PT_(valid_state)(&a);
		if(a.size < 1000000 && !(i & (i - 1))) {
			char fn[32];
			printf("%s.\n", PT_(array_to_string)(&a));
			sprintf(fn, "graph/" QUOTE(ARRAY_NAME) "Array%lu.gv",
				(unsigned long)i);
			PT_(graph)(&a, fn);
		}
	}
	T_(array_)(&a);
}

static void PT_(test_replace)(void) {
	PT_(type) ts[5], *t, *t1;
	const size_t ts_size = sizeof ts / sizeof *ts;
	struct T_(array) a, b;
	PT_(type) *e;
	int success;

	/* valgrind does not like this. */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	for(t = ts, t1 = t + ts_size; t < t1; t++) PT_(filler)(t);
	printf("Test replace.\n");
	T_(array)(&a);
	for(t = ts, t1 = t + ts_size; t < t1; t++) {
		e = T_(array_new)(&a), assert(e);
		memcpy(e, t, sizeof *t);
	}
	assert(a.size == ts_size);	
	T_(array)(&b);
	/* No-op. */
	success = T_(array_splice)(&a, 0, 0, 0);
	printf("Array %s.\n", PT_(array_to_string)(&a));
	assert(success && a.size == ts_size);
	/* Deleting from the front. */
	success = T_(array_splice)(&a, 0, 1, 0);
	printf("Array after deleting from front %s.\n", PT_(array_to_string)(&a));
	assert(success && a.size == ts_size - 1);
	/* Adding at the back. */
	t = T_(array_new)(&b);
	assert(t);
	memcpy(t, ts + 0, sizeof *t);
	success = T_(array_splice)(&a, a.size, a.size, &b);
	printf("Array after adding %s to back %s.\n", PT_(array_to_string)(&b),
		PT_(array_to_string)(&a));
	assert(success && a.size == ts_size);
	/* Replacing same-size. */
	success = T_(array_splice)(&a, 1, 2, &b);
	printf("Array after replacing [1, 2) %s: %s.\n", PT_(array_to_string)(&b),
		PT_(array_to_string)(&a));
	assert(success && a.size == ts_size
		&& !memcmp(t, a.data + 1, sizeof *t));
	/* Replacing larger size. */
	t = T_(array_new)(&b);
	assert(t);
	memcpy(t, ts + 1, sizeof *t);
	success = T_(array_splice)(&a, 1, 2, &b);
	printf("Array after replacing [1, 2) %s: %s.\n", PT_(array_to_string)(&b),
		PT_(array_to_string)(&a));
	assert(success && a.size == ts_size + 1
		   && !memcmp(t, a.data + 2, sizeof *t));
	/* Replacing a smaller size. */
	success = T_(array_splice)(&a, 1, 4, &b);
	printf("Array after replacing [1, 4) %s: %s.\n", PT_(array_to_string)(&b),
		PT_(array_to_string)(&a));
	assert(success && a.size == ts_size
		&& !memcmp(t, a.data + 2, sizeof *t));
	T_(array_clear)(&b);
	t = T_(array_buffer)(&b, 2);
	assert(t);
	memcpy(t, ts + 2, sizeof *t * 2);
	assert(b.size == 2);
	/* a = [[1],[0],[1],[4],[0]]; b = [[2],[3]] */
	printf("0: a = %s, b = %s.\n", PT_(array_to_string)(&a),
		PT_(array_to_string)(&b));
	assert(a.size == 5 && b.size == 2);
	T_(array_splice)(&a, a.size, a.size, &b);
	printf("1: a = %s.\n", PT_(array_to_string)(&a));
	/* a = [[1],[0],[1],[4],[0],[2],[3]] */
	assert(a.size == 7);
	T_(array_splice)(&a, 1, 3, &b);
	printf("2: a = %s.\n", PT_(array_to_string)(&a));
	/* a = [[1],[2],[3],[4],[0],[2],[3]] */
	assert(a.size == 7);
	T_(array_splice)(&a, T_(array_clip)(&a, 2), T_(array_clip)(&a, -4), &b);
	printf("3: a = %s.\n", PT_(array_to_string)(&a));
	/* a = [[1],[2],[2],[3],[4],[0],[2],[3]] */
	assert(a.size == 8);
	T_(array_splice)(&a, 7, T_(array_clip)(&a, -1) + 1, &b);
	printf("4: a = %s.\n", PT_(array_to_string)(&a));
	/* a = [[1],[2],[2],[3],[4],[0],[2],[2],[3]] */
	/* @fixme This is not enough coverage. */
	assert(a.size == 9 &&
		!memcmp(ts + 1, a.data, sizeof *t * 2) &&
		!memcmp(ts + 2, a.data + 2, sizeof *t * 3) &&
		!memcmp(ts + 0, a.data + 5, sizeof *t) &&
		!memcmp(ts + 2, a.data + 6, sizeof *t) &&
		!memcmp(ts + 2, a.data + 7, sizeof *t * 2));
	T_(array_)(&b);
	T_(array_)(&a);
}

/** @implements <PT>Predicate
 @return A set sequence of ones and zeros, independant of `data`. */
static int PT_(keep_deterministic)(const PT_(type) *const data) {
	static size_t i;
	static const int things[] = { 1,0,0,0,0,1,0,0,1,1, 0,1,0,1,0,1,0 };
	const int predicate = things[i++];
	(void)data;
	i %= sizeof things / sizeof *things;
	return predicate;
}

static void PT_(test_keep)(void) {
	PT_(type) ts[17], *t, *t1, *e;
	const size_t ts_size = sizeof ts / sizeof *ts;
	struct T_(array) a = ARRAY_IDLE, b = ARRAY_IDLE;
	int ret;
	/* Valgrind. */
	memset(ts, 0, sizeof ts);
	PT_(valid_state)(&a);
	for(t = ts, t1 = t + ts_size; t < t1; t++) {
		PT_(filler)(t);
		e = T_(array_new)(&a), assert(e);
		memcpy(e, t, sizeof *t);
	}
	printf("a = %s.\n", T_(array_to_string)(&a));
	T_(array_keep_if)(&a, &PT_(keep_deterministic), 0);
	printf("a = k(a) = %s.\n", T_(array_to_string)(&a));
	assert(a.size == 7
		&& !memcmp(ts + 0, a.data + 0, sizeof *t * 1)
		&& !memcmp(ts + 5, a.data + 1, sizeof *t * 1)
		&& !memcmp(ts + 8, a.data + 2, sizeof *t * 2)
		&& !memcmp(ts + 11, a.data + 4, sizeof *t * 1)
		&& !memcmp(ts + 13, a.data + 5, sizeof *t * 1)
		&& !memcmp(ts + 15, a.data + 6, sizeof *t * 1));
	PT_(valid_state)(&a);

	ret = T_(array_copy_if)(&b, &PT_(keep_deterministic), 0);
	assert(ret && !b.size);
	ret = T_(array_copy_if)(&b, &PT_(keep_deterministic), &a);
	printf("b = k(a) = %s.\n", T_(array_to_string)(&b));
	assert(ret && b.size == 2
		&& !memcmp(ts + 0, b.data + 0, sizeof *t * 1)
		&& !memcmp(ts + 13, b.data + 1, sizeof *t * 1));
	T_(array_)(&a);
	T_(array_)(&b);
}

static int PT_(num);

/** Increments a global variable, independent of `t`. @implements <PT>action */
static void PT_(increment)(PT_(type) *const t) {
	(void)t;
	PT_(num)++;
}

/** True, independent of `t`.
 @implements <PT>Predicate */
static int PT_(true)(const PT_(type) *const t) {
	(void)t;
	return 1;
}

static void PT_(test_each)(void) {
	struct T_(array) empty = ARRAY_IDLE, one = ARRAY_IDLE;
	PT_(type) *t;
	t = T_(array_new)(&one);
	assert(t);
	if(!t) return;
	PT_(num) = 0;
	T_(array_each)(&empty, &PT_(increment));
	assert(!PT_(num));
	T_(array_each)(&one, &PT_(increment));
	assert(PT_(num) == 1);
	PT_(num) = 0;
	T_(array_if_each)(&empty, &PT_(true), &PT_(increment));
	assert(!PT_(num));
	T_(array_if_each)(&one, &PT_(true), &PT_(increment));
	assert(PT_(num) == 1);
	PT_(num) = 0;
	t = T_(array_any)(&empty, &PT_(true));
	assert(!t);
	t = T_(array_any)(&one, &PT_(true));
	assert(t == one.data);
	T_(array_)(&one);
}

/** Will be tested on stdout. Requires `ARRAY_TEST`, `ARRAY_TO_STRING`, and not
 `NDEBUG` while defining `assert`. @allow */
static void T_(array_test)(void) {
	printf("<" QUOTE(ARRAY_NAME) ">Array of type <" QUOTE(ARRAY_TYPE)
		"> was created using: "
#ifdef ARRAY_TO_STRING
		"ARRAY_TO_STRING <" QUOTE(ARRAY_TO_STRING) ">; "
#endif
		"ARRAY_TEST <" QUOTE(ARRAY_TEST) ">; "
		"testing:\n");
	PT_(test_basic)();
	PT_(test_random)();
	PT_(test_replace)();
	PT_(test_keep)();
	PT_(test_each)();
	fprintf(stderr, "Done tests of <" QUOTE(ARRAY_NAME) ">Array.\n\n");
}

#elif defined(ARRAY_COMPARE) \
	|| defined(ARRAY_EQUAL) /* to string --><!-- comparable */

/** Fills `fill` that is not equal to `neq` if possible. */
static int PTC_(fill_unique)(PT_(type) *const fill,
	const PT_(type) *const neq) {
	size_t i;
	assert(fill);
	for(i = 0; i < 1000; i++) {
		PT_(filler)(fill);
		if(!neq || PTC_(compare)(neq, fill)) return 1;
	}
	assert(0); return 0;
}

static void PTC_(test_compactify)(void) {
	struct T_(array) a = ARRAY_IDLE;
	PT_(type) ts[9], *t, *t1, *t_prev;
	const size_t ts_size = sizeof ts / sizeof *ts;

	/* `valgrind` is giving me grief if I don't do this? */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	assert(ts_size % 3 == 0);
	for(t_prev = 0, t = ts, t1 = t + ts_size; t < t1; t_prev = t, t += 3) {
		if(!PTC_(fill_unique)(t, t_prev)) { assert(0); return; }
		memcpy(t + 1, t, sizeof *t);
		memcpy(t + 2, t, sizeof *t);
	}
	if(!T_(array_buffer)(&a, ts_size)) { assert(0); return; }
	memcpy(a.data, ts, sizeof *t * ts_size);
	printf("Before: %s.\n", PT_(array_to_string)(&a));
	T_C_(array, unique)(&a);
	printf("Compactified: %s.\n", PT_(array_to_string)(&a));
	assert(a.size == ts_size / 3);
#ifdef ARRAY_COMPARE /* <!-- compare */
	T_C_(array, sort)(&a);
	printf("Sorted: %s.\n", PT_(array_to_string)(&a));
	for(t = a.data, t1 = a.data + a.size - 1; t < t1; t++)
		assert(PTC_(compare)(t, t + 1) <= 0);
	T_C_(array, reverse)(&a);
	printf("Reverse: %s.\n", PT_(array_to_string)(&a));
	for(t = a.data, t1 = a.data + a.size - 1; t < t1; t++)
		assert(PTC_(compare)(t, t + 1) >= 0);
#endif /* compare --> */
	T_(array_)(&a);
}

static void PTC_(test_bounds)(void) {
#ifdef ARRAY_COMPARE /* <!-- compare */
	struct T_(array) a = ARRAY_IDLE;
	const size_t size = 10;
	size_t i, low, high;
	PT_(type) elem;
	char z[12];
	int t;
	if(!T_(array_buffer)(&a, size)) { assert(0); return; }
	for(i = 0; i < size; i++) PT_(filler)(a.data + i);
	PT_(filler)(&elem);
	printf("bounds: %s\n", PT_(array_to_string)(&a));
	T_C_(array, sort)(&a);
	printf("sorted: %s.\n", PT_(array_to_string)(&a));
	PT_(to_string)(&elem, &z), printf("elem: %s\n", z);
	low = T_C_(array, lower_bound)(&a, &elem);
	printf(QUOTE(ARRAY_COMPARE) " lower_bound: %lu.\n", (unsigned long)low);
	assert(low <= a.size);
	assert(!low || PTC_(compare)(a.data + low - 1, &elem) < 0);
	assert(low == a.size || PTC_(compare)(&elem, a.data + low) <= 0);
	high = T_C_(array, upper_bound)(&a, &elem);
	printf(QUOTE(ARRAY_COMPARE) " upper_bound: %lu.\n", (unsigned long)high);
	assert(high <= a.size);
	assert(!high || PTC_(compare)(a.data + high - 1, &elem) <= 0);
	assert(high == a.size || PTC_(compare)(&elem, a.data + high) < 0);
	t = T_C_(array, insert)(&a, &elem);
	printf("insert: %s.\n", PT_(array_to_string)(&a));
	assert(t && a.size == size + 1);
	t = memcmp(&elem, a.data + low, sizeof elem);
	assert(!t);
	T_(array_clear)(&a);
	T_(array_buffer)(&a, size);
	for(i = 0; i < size; i++) memcpy(a.data + i, &elem, sizeof elem);
	printf("bounds: %s.\n", PT_(array_to_string)(&a));
	low = T_C_(array, lower_bound)(&a, &elem);
	printf(QUOTE(ARRAY_COMPARE) " lower_bound: %lu.\n", (unsigned long)low);
	assert(low == 0);
	high = T_C_(array, upper_bound)(&a, &elem);
	printf(QUOTE(ARRAY_COMPARE) " upper_bound: %lu.\n", (unsigned long)high);
	assert(high == a.size);
	T_(array_)(&a);
#endif /* compare --> */
}

/** Passed `parent_new` and `parent`, tests sort and meta-sort. */
static void PTC_(test_sort)(void) {
#ifdef ARRAY_COMPARE /* <!-- comp */
	struct T_(array) as[64], *a;
	const size_t as_size = sizeof as / sizeof *as;
	const struct T_(array) *const as_end = as + as_size;
	int cmp;
	/* Random array of Arrays. */
	for(a = as; a < as_end; a++) {
		size_t size = rand() / (RAND_MAX / 5 + 1), i;
		PT_(type) *x, *x_end;
		T_(array)(a);
		x = T_(array_buffer)(a, size);
		x_end = x + size;
		if(!size) continue;
		assert(x);
		for(i = 0; i < size; i++) PT_(filler)(a->data + i);
		T_C_(array, sort)(a);
		for(x = a->data; x < x_end - 1; x++)
			cmp = PTC_(compare)(x, x + 1), assert(cmp <= 0);
	}
	/* Now sort the lists. */
	qsort(as, as_size, sizeof *as,
		(int (*)(const void *, const void *))&T_(array_compare));
	printf("Sorted array of sorted <" QUOTE(LIST_NAME) ">List by "
		   QUOTE(LIST_COMPARE) ":\n");
	for(a = as; a < as_end; a++) {
		printf("List: %s.\n", PT_(array_to_string)(a));
		if(a == as) continue;
		cmp = T_(array_compare)(a - 1, a);
		assert(cmp <= 0);
	}
#else /* comp --><!-- !comp */
	(void)(parent_new), (void)(parent);
#endif /* !comp --> */
}

/** Will be tested on stdout. Requires `ARRAY_TEST`, `ARRAY_TO_STRING`, and not
 `NDEBUG` while defining `assert`. @allow */
static void T_C_(array, comparable_test)(void) {
	printf("<" QUOTE(ARRAY_NAME) ">array testing <"
#ifdef ARRAY_ORDER
		QUOTE(ARRAY_CONTRAST_NAME)
#else
		"anonymous"
#endif
		"> contrast "
#ifdef ARRAY_COMPARE
		"compare <" QUOTE(ARRAY_COMPARE)
#elif defined(ARRAY_IS_EQUAL)
		"is equal <" QUOTE(ARRAY_IS_EQUAL)
#endif
		">:\n");
	PTC_(test_compactify)();
	PTC_(test_bounds)();
	PTC_(test_sort)();
	assert(errno == 0);
	fprintf(stderr, "Done tests of <" QUOTE(ARRAY_NAME) ">array contrast.\n\n");
}

#else /* compare --><!-- */
#error Test unsupported option; testing is out-of-sync?
#endif /* --> */

#undef QUOTE
#undef QUOTE_

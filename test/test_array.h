#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/* `ARRAY_TEST` must be a function that implements <typedef:<PA>action_fn>. */
static void (*PA_(filler))(PA_(type) *) = (ARRAY_TEST);

/** @return Is `a` in a valid state? */
static void PA_(valid_state)(const struct A_(array) *const a) {
	const size_t max_size = (size_t)-1 / sizeof *a->data;
	/* Null is a valid state. */
	if(!a) return;
	if(!a->data) { assert(!a->size); return; }
	assert(a->size <= a->capacity && a->capacity <= max_size);
}

/** Draw a graph of `ar` to `fn` in Graphviz format. */
static void PA_(graph)(const struct A_(array) *const ar, const char *const fn) {
	FILE *fp;
	size_t i;
	char a[12];
	/* This is a messy hack; require that `errno` is not set, if we can't open
	 the file for writing, take it. Drawing graphs is usually not the point. */
	assert(ar && fn && !errno);
	if(!(fp = fopen(fn, "w"))) { perror(fn); errno = 0; return; }
	fprintf(fp, "digraph {\n"
		"\tfontface=modern;"
		"\tnode [shape=box, style=filled, fillcolor=\"Gray95\"];\n"
		"\tarray [label=<\n"
		"<TABLE BORDER=\"0\">\n"
		"\t<TR><TD COLSPAN=\"3\" ALIGN=\"LEFT\">"
		"<FONT COLOR=\"Gray85\">&lt;" QUOTE(ARRAY_NAME)
		"&gt;array: " QUOTE(ARRAY_TYPE) "</FONT></TD></TR>\n"
		"\t<TR>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">size</TD>\n"
		"\t\t<TD BORDER=\"0\" BGCOLOR=\"Gray90\">%lu</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">%lu</TD>\n"
		"\t</TR>\n"
		"</TABLE>>];\n", (unsigned long)ar->size, (unsigned long)ar->capacity);
	if(!ar->data) goto no_data;
	fprintf(fp, "\tarray -> data;\n"
		"\tdata [label=<\n"
		"<TABLE BORDER=\"0\">\n"
		"\t<TR><TD COLSPAN=\"2\" ALIGN=\"LEFT\">"
		"<FONT COLOR=\"Gray85\">%s</FONT></TD></TR>\n", orcify(ar->data));
	for(i = 0; i < ar->size; i++) {
		const char *const bgc = i & 1 ? "" : " BGCOLOR=\"Gray90\"";
		PA_(to_string)(ar->data + i, &a);
		fprintf(fp, "\t<TR>\n"
			"\t\t<TD ALIGN=\"RIGHT\"%s>%lu</TD>\n"
			"\t\t<TD ALIGN=\"LEFT\"%s>%s</TD>\n"
			"\t</TR>\n", bgc, (unsigned long)i, bgc, a);
	}
	fprintf(fp, "</TABLE>>];\n");
no_data:
	fprintf(fp, "\tnode [colour=red];\n"
		"}\n");
	fclose(fp);
}

/** @implements <PA>Predicate @return Is `t` zero-filled? */
static int PA_(zero_filled)(const PA_(type) *const t) {
	const char *c = (const char *)t, *const end = (const char *)(t + 1);
	assert(t);
	while(c < end) if(*c++) return 0;
	return 1;
}

static void PA_(test_basic)(void) {
	struct A_(array) a;
	PA_(type) ts[5], *t, *t1;
	const size_t ts_size = sizeof ts / sizeof *ts, big = 1000;
	size_t i;
	int is_zero;

	assert(errno == 0);
	PA_(valid_state)(0);

	printf("Test empty.\n");
	A_(array)(&a);
	t = (PA_(type) *)1;
	assert(errno == 0);
	PA_(valid_state)(&a);

	/* This is un-necessary, but `valgrind` reports an error if we don't. */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	for(t = ts, t1 = t + ts_size; t < t1; t++) PA_(filler)(t);

	printf("Test one element.\n");
	t = A_(array_new)(&a); /* Add. */
	assert(t && a.data == t && a.size == 1);
	assert(A_(array_peek)(&a) == t);
	t1 = A_(array_pop)(&a); /* Remove. */
	assert(t1 == t);
	assert(A_(array_peek)(&a) == 0);
	t = A_(array_new)(&a); /* Add. */
	assert(t && a.size == 1 && a.capacity >= 1);
	A_(array_shrink)(&a);
	assert(ARRAY_MIN_CAPACITY > 1);
	assert(a.size == 1 && a.capacity == ARRAY_MIN_CAPACITY);
	t = A_(array_new)(&a); /* Add 2. */
	assert(t && a.size == 2 && a.capacity >= 2);
	A_(array_clear)(&a);
	assert(A_(array_peek)(&a) == 0);
	PA_(valid_state)(&a);

	printf("Testing lazy remove.\n");
	assert(ts_size >= 3);
	for(i = 0; i < 3; i++) {
		t = A_(array_new)(&a);
		assert(t);
		memcpy(t, ts + i, sizeof *t);
	}
	A_(array_lazy_remove)(&a, a.data);
	assert(a.size == 2);
	t = a.data;
	assert(!memcmp(t, ts + 2, sizeof *t) && !memcmp(t + 1, ts + 1, sizeof *t));
	A_(array_clear)(&a);
	assert(!a.size);

	printf("Testing %lu elements.\n", (unsigned long)ts_size);
	for(i = 0; i < ts_size; i++) {
		t = A_(array_new)(&a);
		assert(t);
		memcpy(t, ts + i, sizeof *t);
	}
	assert(A_(array_peek)(&a));
	printf("Now: %s.\n", PA_(array_to_string)(&a));
	assert(a.size == ts_size);
	t = a.data + ts_size - 2;
	A_(array_remove)(&a, t);
	t = a.data + ts_size - 3;
	A_(array_remove)(&a, t);
	printf("Now: %s.\n", PA_(array_to_string)(&a));

	assert(a.size == ts_size - 2);
	A_(array_append)(&a, 2);
	memcpy(t + 1, ts + 3, sizeof *t * 2);
	assert(a.size == ts_size);
	PA_(valid_state)(&a);
	printf("Now: %s.\n", PA_(array_to_string)(&a));

	/* Peek/Pop. */
	t = A_(array_peek)(&a);
	assert(t && !memcmp(t, ts + ts_size - 1, sizeof *t));
	t = A_(array_pop)(&a);
	assert(t && !memcmp(t, ts + ts_size - 1, sizeof *t));
	t = A_(array_pop)(&a);
	assert(t && !memcmp(t, ts + ts_size - 2, sizeof *t));
	A_(array_clear)(&a);
	assert(a.size == 0);

	/* Trim 1. */
	t = A_(array_new)(&a);
	assert(t);
	memset(t, 0, sizeof *t);
	A_(array_trim)(&a, &PA_(zero_filled));
	assert(a.size == 0);
	/* Trim 3. */
	t = A_(array_new)(&a);
	assert(t);
	memset(t, 0, sizeof *t);
	t = A_(array_new)(&a);
	assert(t);
	PA_(filler)(t);
	is_zero = PA_(zero_filled)(t);
	t = A_(array_new)(&a);
	assert(t);
	memset(t, 0, sizeof *t);
	A_(array_trim)(&a, &PA_(zero_filled));
	assert(a.size == !is_zero);

	/* Big. */
	for(i = 0; i < big; i++) {
		t = A_(array_new)(&a), assert(t);
		PA_(filler)(t);
	}
	printf("%s.\n", PA_(array_to_string)(&a));
	PA_(valid_state)(&a);

	printf("Clear:\n");
	A_(array_clear)(&a);
	printf("%s.\n", PA_(array_to_string)(&a));
	assert(A_(array_peek)(&a) == 0 && a.size == 0);

	printf("Destructor:\n");
	A_(array_)(&a);
	assert(A_(array_peek)(&a) == 0);
	PA_(valid_state)(&a);
}

static void PA_(test_random)(void) {
	struct A_(array) a;
	const size_t mult = 1; /* For long tests. */
	/* This parameter controls how many iterations. */
	size_t i, i_end = 1000 * mult, size = 0;
	/* Random. */
	A_(array)(&a);
	for(i = 0; i < i_end; i++) {
		PA_(type) *data;
		char str[12];
		unsigned r = (unsigned)rand();
		int is_print = !(rand() / (RAND_MAX / 50 + 1));
		if(is_print) printf("%lu: ", (unsigned long)i);
		/* This parameter controls how big the pool wants to be. */
		if(r > size * (RAND_MAX / (2 * 100 * mult))) {
			if(!(data = A_(array_new)(&a)))
				{ perror("Error"), assert(0); return; }
			size++;
			PA_(filler)(data);
			PA_(to_string)(data, &str);
			if(is_print) printf("created %s.", str);
		} else {
			const unsigned t = RAND_MAX / 2;
			r = (unsigned)rand();
			if(r < t) {
				data = A_(array_peek)(&a);
				assert(data);
				PA_(to_string)(data, &str);
				if(is_print) printf("popping %s.", str);
				assert(data == A_(array_pop)(&a));
			} else {
				size_t idx = (unsigned)rand() / (RAND_MAX / size + 1);
				if(!(data = a.data + idx)) continue;
				PA_(to_string)(data, &str);
				if(is_print)
					printf("removing %s at %lu.", str, (unsigned long)idx);
				A_(array_remove)(&a, data);
			}
			size--;
		}
		if(is_print) printf(" Size %lu.\n", (unsigned long)a.size);
		PA_(valid_state)(&a);
		if(a.size < 1000000 && !(i & (i - 1))) {
			char fn[32];
			printf("%s.\n", PA_(array_to_string)(&a));
			sprintf(fn, "graph/" QUOTE(ARRAY_NAME) "-array-%lu.gv",
				(unsigned long)i);
			PA_(graph)(&a, fn);
		}
	}
	A_(array_)(&a);
}

static void PA_(test_replace)(void) {
	PA_(type) ts[5], *t, *t1;
	const size_t ts_size = sizeof ts / sizeof *ts;
	struct A_(array) a, b;
	PA_(type) *e;
	int success;

	/* valgrind does not like this. */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	for(t = ts, t1 = t + ts_size; t < t1; t++) PA_(filler)(t);
	printf("Test replace.\n");
	A_(array)(&a);
	for(t = ts, t1 = t + ts_size; t < t1; t++) {
		e = A_(array_new)(&a), assert(e);
		memcpy(e, t, sizeof *t);
	}
	assert(a.size == ts_size);	
	A_(array)(&b);
	/* No-op. */
	success = A_(array_splice)(&a, 0, 0, 0);
	printf("Array %s.\n", PA_(array_to_string)(&a));
	assert(success && a.size == ts_size);
	/* Deleting from the front. */
	success = A_(array_splice)(&a, 0, 1, 0);
	printf("Array after deleting from front %s.\n", PA_(array_to_string)(&a));
	assert(success && a.size == ts_size - 1);
	/* Adding at the back. */
	t = A_(array_new)(&b);
	assert(t);
	memcpy(t, ts + 0, sizeof *t);
	success = A_(array_splice)(&a, a.size, a.size, &b);
	printf("Array after adding %s to back %s.\n", PA_(array_to_string)(&b),
		PA_(array_to_string)(&a));
	assert(success && a.size == ts_size);
	/* Replacing same-size. */
	success = A_(array_splice)(&a, 1, 2, &b);
	printf("Array after replacing [1, 2) %s: %s.\n", PA_(array_to_string)(&b),
		PA_(array_to_string)(&a));
	assert(success && a.size == ts_size
		&& !memcmp(t, a.data + 1, sizeof *t));
	/* Replacing larger size. */
	t = A_(array_new)(&b);
	assert(t);
	memcpy(t, ts + 1, sizeof *t);
	success = A_(array_splice)(&a, 1, 2, &b);
	printf("Array after replacing [1, 2) %s: %s.\n", PA_(array_to_string)(&b),
		PA_(array_to_string)(&a));
	assert(success && a.size == ts_size + 1
		&& !memcmp(t, a.data + 2, sizeof *t));
	/* Replacing a smaller size. */
	success = A_(array_splice)(&a, 1, 4, &b);
	printf("Array after replacing [1, 4) %s: %s.\n", PA_(array_to_string)(&b),
		PA_(array_to_string)(&a));
	assert(success && a.size == ts_size
		&& !memcmp(t, a.data + 2, sizeof *t));
	A_(array_clear)(&b);
	t = A_(array_append)(&b, 2), assert(t);
	memcpy(t, ts + 2, sizeof *t * 2);
	assert(b.size == 2);
	/* a = [[1],[0],[1],[4],[0]]; b = [[2],[3]] */
	printf("0: a = %s, b = %s.\n", PA_(array_to_string)(&a),
		PA_(array_to_string)(&b));
	assert(a.size == 5 && b.size == 2);
	A_(array_splice)(&a, a.size, a.size, &b);
	printf("1: a = %s.\n", PA_(array_to_string)(&a));
	/* a = [[1],[0],[1],[4],[0],[2],[3]] */
	assert(a.size == 7);
	A_(array_splice)(&a, 1, 3, &b);
	printf("2: a = %s.\n", PA_(array_to_string)(&a));
	/* a = [[1],[2],[3],[4],[0],[2],[3]] */
	assert(a.size == 7);
	A_(array_splice)(&a, A_(array_clip)(&a, 2), A_(array_clip)(&a, -4), &b);
	printf("3: a = %s.\n", PA_(array_to_string)(&a));
	/* a = [[1],[2],[2],[3],[4],[0],[2],[3]] */
	assert(a.size == 8);
	A_(array_splice)(&a, 7, A_(array_clip)(&a, -1) + 1, &b);
	printf("4: a = %s.\n", PA_(array_to_string)(&a));
	/* a = [[1],[2],[2],[3],[4],[0],[2],[2],[3]] */
	assert(a.size == 9 &&
		!memcmp(ts + 1, a.data, sizeof *t * 2) &&
		!memcmp(ts + 2, a.data + 2, sizeof *t * 3) &&
		!memcmp(ts + 0, a.data + 5, sizeof *t) &&
		!memcmp(ts + 2, a.data + 6, sizeof *t) &&
		!memcmp(ts + 2, a.data + 7, sizeof *t * 2));
	A_(array_)(&b);
	A_(array_)(&a);
}

/** @implements <PA>Predicate
 @return A set sequence of ones and zeros, independant of `data`. */
static int PA_(keep_deterministic)(const PA_(type) *const data) {
	static size_t i;
	static const int things[] = { 1,0,0,0,0,1,0,0,1,1, 0,1,0,1,0,1,0 };
	const int predicate = things[i++];
	(void)data;
	i %= sizeof things / sizeof *things;
	return predicate;
}

static void PA_(test_keep)(void) {
	PA_(type) ts[17], *t, *t1, *e;
	const size_t ts_size = sizeof ts / sizeof *ts;
	struct A_(array) a = ARRAY_IDLE, b = ARRAY_IDLE;
	int ret;
	memset(ts, 0, sizeof ts); /* Valgrind. */
	PA_(valid_state)(&a);
	for(t = ts, t1 = t + ts_size; t < t1; t++) {
		PA_(filler)(t);
		e = A_(array_new)(&a), assert(e);
		memcpy(e, t, sizeof *t);
	}
	printf("a = %s.\n", PA_(array_to_string)(&a));
	A_(array_keep_if)(&a, &PA_(keep_deterministic), 0);
	printf("a = k(a) = %s.\n", PA_(array_to_string)(&a));
	assert(a.size == 7
		&& !memcmp(ts + 0, a.data + 0, sizeof *t * 1)
		&& !memcmp(ts + 5, a.data + 1, sizeof *t * 1)
		&& !memcmp(ts + 8, a.data + 2, sizeof *t * 2)
		&& !memcmp(ts + 11, a.data + 4, sizeof *t * 1)
		&& !memcmp(ts + 13, a.data + 5, sizeof *t * 1)
		&& !memcmp(ts + 15, a.data + 6, sizeof *t * 1));
	PA_(valid_state)(&a);
	ret = A_(array_copy_if)(&b, &PA_(keep_deterministic), 0);
	assert(ret && !b.size);
	ret = A_(array_copy_if)(&b, &PA_(keep_deterministic), &a);
	printf("b = k(a) = %s.\n", PA_(array_to_string)(&b));
	assert(ret && b.size == 2
		&& !memcmp(ts + 0, b.data + 0, sizeof *t * 1)
		&& !memcmp(ts + 13, b.data + 1, sizeof *t * 1));
	A_(array_)(&a);
	A_(array_)(&b);
}

static int PA_(num);

/** Increments a global variable, independent of `t`. @implements <PA>action */
static void PA_(increment)(PA_(type) *const t) {
	(void)t;
	PA_(num)++;
}

/** True, independent of `t`.
 @implements <PA>Predicate */
static int PA_(true)(const PA_(type) *const t) {
	(void)t;
	return 1;
}

static void PA_(test_each)(void) {
	struct A_(array) empty = ARRAY_IDLE, one = ARRAY_IDLE;
	const PA_(type) *t;
	t = A_(array_new)(&one);
	assert(t);
	if(!t) return;
	PA_(num) = 0;
	A_(array_each)(&empty, &PA_(increment));
	assert(!PA_(num));
	A_(array_each)(&one, &PA_(increment));
	assert(PA_(num) == 1);
	PA_(num) = 0;
	A_(array_if_each)(&empty, &PA_(true), &PA_(increment));
	assert(!PA_(num));
	A_(array_if_each)(&one, &PA_(true), &PA_(increment));
	assert(PA_(num) == 1);
	PA_(num) = 0;
	t = A_(array_any)(&empty, &PA_(true));
	assert(!t);
	t = A_(array_any)(&one, &PA_(true));
	assert(t == one.data);
	A_(array_)(&one);
}

/** Will be tested on stdout. Requires `ARRAY_TEST`, `ARRAY_TO_STRING`, and not
 `NDEBUG` while defining `assert`. @allow */
static void A_(array_test)(void) {
	printf("<" QUOTE(ARRAY_NAME) ">array of type <" QUOTE(ARRAY_TYPE)
		"> was created using: ARRAY_TO_STRING <" QUOTE(ARRAY_TO_STRING) ">; "
		"ARRAY_TEST <" QUOTE(ARRAY_TEST) ">; testing:\n");
	assert(PA_(to_string) && PA_(array_to_string));
	PA_(test_basic)();
	PA_(test_random)();
	PA_(test_replace)();
	PA_(test_keep)();
	PA_(test_each)();
	fprintf(stderr, "Done tests of <" QUOTE(ARRAY_NAME) ">Array.\n\n");
}



#if 0 /* <!-- no */

/*defined(ARRAY_COMPARE) || defined(ARRAY_EQUAL) */
/* to string --><!-- comparable */

/** Fills `fill` that is not equal to `neq` if possible. */
static int PTC_(fill_unique)(PA_(type) *const fill,
	const PA_(type) *const neq) {
	size_t i;
	assert(fill);
	for(i = 0; i < 1000; i++) {
		PA_(filler)(fill);
		if(!neq || PTC_(compare)(neq, fill)) return 1;
	}
	assert(0); return 0;
}

static void PTC_(test_compactify)(void) {
	struct A_(array) a = ARRAY_IDLE;
	PA_(type) ts[9], *t, *t1, *t_prev;
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
	if(!A_(array_append)(&a, ts_size)) { assert(0); return; }
	memcpy(a.data, ts, sizeof *t * ts_size);
	printf("Before: %s.\n", PA_(array_to_string)(&a));
	T_C_(array, unique)(&a);
	printf("Compactified: %s.\n", PA_(array_to_string)(&a));
	assert(a.size == ts_size / 3);
#ifdef ARRAY_COMPARE /* <!-- compare */
	T_C_(array, sort)(&a);
	printf("Sorted: %s.\n", PA_(array_to_string)(&a));
	for(t = a.data, t1 = a.data + a.size - 1; t < t1; t++)
		assert(PTC_(compare)(t, t + 1) <= 0);
	T_C_(array, reverse)(&a);
	printf("Reverse: %s.\n", PA_(array_to_string)(&a));
	for(t = a.data, t1 = a.data + a.size - 1; t < t1; t++)
		assert(PTC_(compare)(t, t + 1) >= 0);
#endif /* compare --> */
	A_(array_)(&a);
}

static void PTC_(test_bounds)(void) {
#ifdef ARRAY_COMPARE /* <!-- compare */
	struct A_(array) a = ARRAY_IDLE;
	const size_t size = 10;
	size_t i, low, high;
	PA_(type) elem;
	char z[12];
	int t;
	if(!A_(array_append)(&a, size)) { assert(0); return; }
	for(i = 0; i < size; i++) PA_(filler)(a.data + i);
	PA_(filler)(&elem);
	printf("bounds: %s\n", PA_(array_to_string)(&a));
	T_C_(array, sort)(&a);
	printf("sorted: %s.\n", PA_(array_to_string)(&a));
	PA_(to_string)(&elem, &z), printf("elem: %s\n", z);
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
	printf("insert: %s.\n", PA_(array_to_string)(&a));
	assert(t && a.size == size + 1);
	t = memcmp(&elem, a.data + low, sizeof elem);
	assert(!t);
	A_(array_clear)(&a);
	A_(array_append)(&a, size);
	for(i = 0; i < size; i++) memcpy(a.data + i, &elem, sizeof elem);
	printf("bounds: %s.\n", PA_(array_to_string)(&a));
	low = T_C_(array, lower_bound)(&a, &elem);
	printf(QUOTE(ARRAY_COMPARE) " lower_bound: %lu.\n", (unsigned long)low);
	assert(low == 0);
	high = T_C_(array, upper_bound)(&a, &elem);
	printf(QUOTE(ARRAY_COMPARE) " upper_bound: %lu.\n", (unsigned long)high);
	assert(high == a.size);
	A_(array_)(&a);
#endif /* compare --> */
}

/** Passed `parent_new` and `parent`, tests sort and meta-sort. */
static void PTC_(test_sort)(void) {
#ifdef ARRAY_COMPARE /* <!-- comp */
	struct A_(array) as[64], *a;
	const size_t as_size = sizeof as / sizeof *as;
	const struct A_(array) *const as_end = as + as_size;
	int cmp;
	/* Random array of Arrays. */
	for(a = as; a < as_end; a++) {
		size_t size = (unsigned)rand() / (RAND_MAX / 5 + 1), i;
		PA_(type) *x, *x_end;
		A_(array)(a);
		x = A_(array_append)(a, size);
		x_end = x + size;
		if(!size) continue;
		assert(x);
		for(i = 0; i < size; i++) PA_(filler)(a->data + i);
		T_C_(array, sort)(a);
		for(x = a->data; x < x_end - 1; x++)
			cmp = PTC_(compare)(x, x + 1), assert(cmp <= 0);
	}
	/* Now sort the lists. */
	qsort(as, as_size, sizeof *as,
		(int (*)(const void *, const void *))&A_(array_compare));
	printf("Sorted array of sorted <" QUOTE(ARRAY_NAME) ">array by "
		   QUOTE(ARRAY_COMPARE) ":\n");
	for(a = as; a < as_end; a++) {
		printf("array: %s.\n", PA_(array_to_string)(a));
		if(a == as) continue;
		cmp = A_(array_compare)(a - 1, a);
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
	/*PTC_(test_compactify)();
	PTC_(test_bounds)();
	PTC_(test_sort)();*/
	assert(errno == 0);
	fprintf(stderr, "Done tests of <" QUOTE(ARRAY_NAME) ">array contrast.\n\n");
}

/*#else*/ /* compare --><!-- */
/*#error Test unsupported option; testing is out-of-sync? */
/*#endif*/ /* --> */
#endif /* no --> */

#undef QUOTE
#undef QUOTE_

/* We should *not* undef `ARRAY_TEST`, since it's used to pick the first
 to_string. */

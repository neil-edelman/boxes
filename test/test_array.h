#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** @return Is `a` in a valid state? */
static void PT_(valid_state)(const struct T_(array) *const a) {
	const size_t max_size = (size_t)~0 / sizeof *a->data;
	/* Null is a valid state. */
	if(!a) return;
	if(!a->data) { assert(!a->size); return; }
	assert(a->size <= a->capacity && a->capacity <= max_size);
}

/* fixme: this will be elsewhere… it is also very good at debugging programmes. */
/** Draw a graph of `a` to `fn` in Graphviz format. */
static void PT_(graph)(const struct T_(array) *const a, const char *const fn) {
	FILE *fp;
	size_t i;
	char z[12];
	/* This is a messy hack; require that `errno` is not set, if we can't open
	 the file for writing, take it. Drawing graphs is usually not the point. */
	assert(a && fn && !errno);
	if(!(fp = fopen(fn, "w"))) { perror(fn); errno = 0; return; }
	fprintf(fp, "digraph {\n"
		"\tgraph [rankdir=LR, truecolor=true, bgcolor=transparent,"
		" fontname=modern];\n"
		"\tnode [shape=none, fontname=modern];\n"
		"\tarray [label=<\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td colspan=\"3\" align=\"left\">"
		"<font color=\"Gray75\">&lt;" QUOTE(ARRAY_NAME)
		"&gt;array: " QUOTE(ARRAY_TYPE) "</font></td></tr>\n"
		"\t<hr/>\n"
		"\t<tr>\n"
		"\t\t<td border=\"0\" align=\"right\">size</td>\n"
		"\t\t<td border=\"0\">%lu</td>\n"
		"\t\t<td border=\"0\" align=\"right\">%lu</td>\n"
		"\t</tr>\n"
		"\t<hr/>\n"
		"\t<tr><td></td></tr>\n"
		"</table>>];\n", (unsigned long)a->size, (unsigned long)a->capacity);
	if(!a->data) goto no_data;
	fprintf(fp, "\tarray -> data;\n"
		"\tdata [label=<\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td colspan=\"2\" align=\"left\">"
		"<font color=\"Gray75\">%s</font></td></tr>\n"
		"\t<hr/>\n", orcify(a->data));
	for(i = 0; i < a->size; i++) {
		const char *const bgc = i & 1 ? " bgcolor=\"Gray95\"" : "";
		T_(to_string)((void *)(a->data + i), &z);
		fprintf(fp, "\t<tr>\n"
			"\t\t<td align=\"right\"%s>"
			"<font face=\"Times-Italic\">%lu</font></td>\n"
			"\t\t<td align=\"left\"%s>%s</td>\n"
			"\t</tr>\n", bgc, (unsigned long)i, bgc, z);
	}
	fprintf(fp, "\t<hr/>\n"
		"\t<tr><td></td></tr>\n"
		"</table>>];\n");
no_data:
	fprintf(fp, "\tnode [colour=\"Red\"];\n"
		"}\n");
	fclose(fp);
}

static void PT_(test_basic)(void) {
	struct T_(array) a = T_(array)();
	/*struct T_(array_iterator) it;
	char z[12];*/
	PT_(type) items[5], *item, *item1;
	const size_t items_size = sizeof items / sizeof *items, big = 1000;
	size_t i;

	assert(errno == 0);
	PT_(valid_state)(0);

	printf("Test empty.\n");
	assert(errno == 0);
	PT_(valid_state)(&a);

	/* This is un-necessary, but `valgrind` reports an error if we don't. */
	memset(items, 0, sizeof items);
	/* Get elements. */
	for(item = items, item1 = item + items_size; item < item1; item++)
		T_(filler)(item);

	printf("Test one element.\n");
	item = T_(array_new)(&a); /* Add. */
	assert(item && a.data == item && a.size == 1);
	assert(T_(array_peek)(&a) == item);
	item1 = T_(array_pop)(&a); /* Remove. */
	assert(item1 == item);
	assert(T_(array_peek)(&a) == 0);
	item = T_(array_new)(&a); /* Add. */
	assert(item && a.size == 1 && a.capacity >= 1);
	T_(array_shrink)(&a);
	assert(ARRAY_MIN_CAPACITY > 1);
	assert(a.size == 1 && a.capacity == ARRAY_MIN_CAPACITY);
	item = T_(array_new)(&a); /* Add: 2. */
	assert(item && a.size == 2 && a.capacity >= 2);
	T_(array_clear)(&a);
	assert(T_(array_peek)(&a) == 0);
	PT_(valid_state)(&a);

	assert(items_size >= 3);
	for(i = 0; i < 3; i++) {
		item = T_(array_new)(&a);
		assert(item);
		memcpy(item, items + i, sizeof *item);
	}
#if 0 /* Why would anyone use this? */
	printf("Testing iteration, a = %s.\n", T_(array_to_string)(&a));
	for(it = T_(array_begin)(&a), i = 0; item = T_(array_next)(&it); i++)
		assert(!memcmp(item, items + i, sizeof *item));
	assert(i == 3);
	printf("Backwards:\n");
	for(it = T_(array_end)(&a), i = 0; item = T_(array_previous)(&it); i++)
		T_(to_string)((void *)item, &z), printf("%s\n", z),
		assert(!memcmp(item, items + 2 - i, sizeof *item));
	assert(i == 3);
	it = T_(array_begin)(&a);
	item = T_(array_next)(&it), assert(item), T_(to_string)((void *)item, &z);
	T_(array_next)(&it), item = T_(array_previous)(&it);
	assert(!memcmp(item, items + 0, sizeof *item));
	for(i = 0; i < 3; i++) {
		it = T_(array_at)(&a, i);
		item = T_(array_next)(&it), assert(item);
		T_(to_string)((void *)item, &z);
		printf("a[%lu] = %s\n", (unsigned long)i, z);
		assert(!memcmp(item, items + i, sizeof *item));
	}
	it = T_(array_at)(&a,i/*3*/), item = T_(array_next)(&it);
	assert(!item);
	/* Iteration and back. */
	it = T_(array_begin)(&a);
	item = T_(array_next)(&it), assert(!memcmp(item, items + 0, sizeof *item));
	item = T_(array_previous)(&it), assert(!item);
	/* Two iterations and back. */
	item = T_(array_next)(&it), assert(!memcmp(item, items + 0, sizeof *item));
	item = T_(array_next)(&it), assert(!memcmp(item, items + 1, sizeof *item));
	item = T_(array_previous)(&it),
		assert(!memcmp(item, items + 0, sizeof *item));
	item = T_(array_previous)(&it), assert(!item);
	/* Iteration and back. */
	it = T_(array_begin)(&a);
	item = T_(array_previous)(&it), assert(!item);
	item = T_(array_next)(&it), assert(!memcmp(item, items + 0, sizeof *item));
#endif

	printf("Testing lazy remove.\n");
	T_(array_lazy_remove)(&a, a.data);
	assert(a.size == 2);
	item = a.data;
	assert(!memcmp(item, items + 2, sizeof *item)
		&& !memcmp(item + 1, items + 1, sizeof *item));
	T_(array_clear)(&a);
	assert(!a.size);

	for(i = 0; i < items_size; i++) {
		item = T_(array_new)(&a);
		assert(item);
		memcpy(item, items + i, sizeof *item);
	}
	assert(T_(array_peek)(&a));
	printf("Now %lu elements: %s.\n",
		(unsigned long)items_size, T_(array_to_string)(&a));
	assert(a.size == items_size);
	item = a.data + items_size - 2;
	T_(array_remove)(&a, item);
	item = a.data + items_size - 3;
	T_(array_remove)(&a, item);
	printf("Now: %s.\n", T_(array_to_string)(&a));

	assert(a.size == items_size - 2);
	T_(array_append)(&a, 2);
	memcpy(item + 1, items + 3, sizeof *item * 2);
	assert(a.size == items_size);
	PT_(valid_state)(&a);
	printf("Now: %s.\n", T_(array_to_string)(&a));

	/* Peek/Pop. */
	item = T_(array_peek)(&a);
	assert(item && !memcmp(item, items + items_size - 1, sizeof *item));
	item = T_(array_pop)(&a);
	assert(item && !memcmp(item, items + items_size - 1, sizeof *item));
	item = T_(array_pop)(&a);
	assert(item && !memcmp(item, items + items_size - 2, sizeof *item));
	T_(array_clear)(&a);
	assert(a.size == 0);

	/* Big. */
	for(i = 0; i < big; i++) {
		item = T_(array_new)(&a), assert(item);
		T_(filler)(item);
	}
	printf("%s.\n", T_(array_to_string)(&a));
	PT_(valid_state)(&a);

	printf("Clear:\n");
	T_(array_clear)(&a);
	printf("%s.\n", T_(array_to_string)(&a));
	assert(T_(array_peek)(&a) == 0 && a.size == 0);

	printf("Destructor:\n");
	T_(array_)(&a);
	assert(T_(array_peek)(&a) == 0);
	PT_(valid_state)(&a);
}

static void PT_(test_random)(void) {
	struct T_(array) a = T_(array)();
	const size_t mult = 1; /* For long tests. */
	/* This parameter controls how many iterations. */
	size_t i, i_end = 1000 * mult, size = 0;
	/* Random. */
	for(i = 0; i < i_end; i++) {
		PT_(type) *data;
		char str[12];
		unsigned r = (unsigned)rand();
		int is_print = !(rand() / (RAND_MAX / 50 + 1));
		if(is_print) printf("%lu: ", (unsigned long)i);
		/* This parameter controls how big the pool wants to be. */
		if(r > size * (RAND_MAX / (2 * 100 * mult))) {
			if(!(data = T_(array_new)(&a)))
				{ perror("Error"), assert(0); return; }
			size++;
			T_(filler)(data), T_(to_string)((void *)data, &str);
			if(is_print) printf("created %s.", str);
		} else {
			const unsigned t = RAND_MAX / 2;
			r = (unsigned)rand();
			assert(size);
			if(r < t) {
				data = T_(array_peek)(&a);
				assert(data), T_(to_string)((void *)data, &str);
				if(is_print) printf("popping %s.", str);
				assert(data == T_(array_pop)(&a));
			} else {
				size_t idx = (unsigned)rand() / (RAND_MAX / size + 1);
				if(!(data = a.data + idx)) continue;
				T_(to_string)((void *)data, &str);
				if(is_print)
					printf("removing %s at %lu.", str, (unsigned long)idx);
				T_(array_remove)(&a, data);
			}
			size--;
		}
		if(is_print) printf(" Size %lu.\n", (unsigned long)a.size);
		PT_(valid_state)(&a);
		if(a.size < 1000000 && !(i & (i - 1))) {
			char fn[32];
			printf("%s.\n", T_(array_to_string)(&a));
			sprintf(fn, "graph/" QUOTE(ARRAY_NAME) "-array-%lu.gv",
				(unsigned long)i);
			PT_(graph)(&a, fn);
		}
	}
	T_(array_)(&a);
}

static void PT_(test_replace)(void) {
	PT_(type) ts[5], *t, *t1;
	const size_t ts_size = sizeof ts / sizeof *ts;
	struct T_(array) a = T_(array)(), b = T_(array)();
	PT_(type) *e;
	int success;

	/* valgrind does not like this. */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	for(t = ts, t1 = t + ts_size; t < t1; t++) T_(filler)(t);
	printf("Test replace.\n");
	for(t = ts, t1 = t + ts_size; t < t1; t++) {
		e = T_(array_new)(&a), assert(e);
		memcpy(e, t, sizeof *t);
	}
	assert(a.size == ts_size);	
	/* No-op. */
	success = T_(array_splice)(&a, 0, 0, 0);
	printf("Array %s.\n", T_(array_to_string)(&a));
	assert(success && a.size == ts_size);
	/* Deleting from the front. */
	success = T_(array_splice)(&a, 0, 0, 1);
	printf("Array after deleting from front %s.\n", T_(array_to_string)(&a));
	assert(success && a.size == ts_size - 1);
	/* Adding at the back. */
	t = T_(array_new)(&b);
	assert(t);
	memcpy(t, ts + 0, sizeof *t);
	success = T_(array_splice)(&a, &b, a.size, a.size);
	printf("Array after adding %s to back %s.\n", T_(array_to_string)(&b),
		T_(array_to_string)(&a));
	assert(success && a.size == ts_size);
	/* Replacing same-size. */
	success = T_(array_splice)(&a, &b, 1, 2);
	printf("Array after replacing [1, 2) %s: %s.\n", T_(array_to_string)(&b),
		T_(array_to_string)(&a));
	assert(success && a.size == ts_size
		&& !memcmp(t, a.data + 1, sizeof *t));
	/* Replacing larger size. */
	t = T_(array_new)(&b);
	assert(t);
	memcpy(t, ts + 1, sizeof *t);
	success = T_(array_splice)(&a, &b, 1, 2);
	printf("Array after replacing [1, 2) %s: %s.\n", T_(array_to_string)(&b),
		T_(array_to_string)(&a));
	assert(success && a.size == ts_size + 1
		&& !memcmp(t, a.data + 2, sizeof *t));
	/* Replacing a smaller size. */
	success = T_(array_splice)(&a, &b, 1, 4);
	printf("Array after replacing [1, 4) %s: %s.\n", T_(array_to_string)(&b),
		T_(array_to_string)(&a));
	assert(success && a.size == ts_size
		&& !memcmp(t, a.data + 2, sizeof *t));
	T_(array_clear)(&b);
	t = T_(array_append)(&b, 2), assert(t);
	memcpy(t, ts + 2, sizeof *t * 2);
	assert(b.size == 2);
	/* a = [[1],[0],[1],[4],[0]]; b = [[2],[3]] */
	printf("0: a = %s, b = %s.\n", T_(array_to_string)(&a),
		T_(array_to_string)(&b));
	assert(a.size == 5 && b.size == 2);
	T_(array_splice)(&a, &b, a.size, a.size);
	printf("1: a = %s.\n", T_(array_to_string)(&a));
	/* a = [[1],[0],[1],[4],[0],[2],[3]] */
	assert(a.size == 7);
	T_(array_splice)(&a, &b, 1, 3);
	printf("2: a = %s.\n", T_(array_to_string)(&a));
	/* a = [[1],[2],[3],[4],[0],[2],[3]] */
	assert(a.size == 7);
	T_(array_splice)(&a, &b, 2, 3);
	printf("3: a = %s.\n", T_(array_to_string)(&a));
	/* a = [[1],[2],[2],[3],[4],[0],[2],[3]] */
	assert(a.size == 8);
	T_(array_splice)(&a, &b, 7, 8);
	printf("4: a = %s.\n", T_(array_to_string)(&a));
	/* a = [[1],[2],[2],[3],[4],[0],[2],[2],[3]] */
	assert(a.size == 9 &&
		!memcmp(ts + 1, a.data, sizeof *t * 2) &&
		!memcmp(ts + 2, a.data + 2, sizeof *t * 3) &&
		!memcmp(ts + 0, a.data + 5, sizeof *t) &&
		!memcmp(ts + 2, a.data + 6, sizeof *t) &&
		!memcmp(ts + 2, a.data + 7, sizeof *t * 2));
	T_(array_)(&b);
	T_(array_)(&a);
}

#ifdef HAVE_ITERATE_H /* <!-- iterate */
/** @implements <PA>Predicate
 @return A set sequence of ones and zeros, independant of `data`. */
static int PT_(keep_deterministic)(const PT_(type) *const data) {
	static size_t i;
	static const int things[] = { 1,0,0,0,0,1,0,0,1,1, 0,1,0,1,0,1,0 };
	const int predicate = things[i++];
	(void)data;
	i %= sizeof things / sizeof *things;
	return predicate;
}
static int PT_(num);
/** Increments a global variable, independent of `t`. @implements <PA>action */
static void PT_(increment)(PT_(type) *const t) {
	(void)t;
	PT_(num)++;
}
/** True, independent of `t`.
 @implements <PA>Predicate */
static int PT_(true)(const PT_(type) *const t) {
	(void)t;
	return 1;
}
/** @implements <PA>Predicate @return Is `t` zero-filled? */
static int PT_(zero_filled)(const PT_(type) *const t) {
	const char *c = (const char *)t, *const end = (const char *)(t + 1);
	assert(t);
	while(c < end) if(*c++) return 0;
	return 1;
}
#endif /* iterate --> */

static void PT_(test_keep)(void) {
#ifdef HAVE_ITERATE_H
	PT_(type) ts[17], *t, *t1, *e;
	const size_t ts_size = sizeof ts / sizeof *ts;
	struct T_(array) a = T_(array)(), b = T_(array)();
	int ret;
	memset(ts, 0, sizeof ts); /* Valgrind. */
	PT_(valid_state)(&a);
	for(t = ts, t1 = t + ts_size; t < t1; t++) {
		T_(filler)(t);
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
	ret = T_(array_copy_if)(&b, 0, &PT_(keep_deterministic));
	assert(ret && !b.size);
	ret = T_(array_copy_if)(&b, &a, &PT_(keep_deterministic));
	printf("b = k(a) = %s.\n", T_(array_to_string)(&b));
	assert(ret && b.size == 2
		&& !memcmp(ts + 0, b.data + 0, sizeof *t * 1)
		&& !memcmp(ts + 13, b.data + 1, sizeof *t * 1));
	T_(array_)(&a);
	T_(array_)(&b);
#endif
}

static void PT_(test_each)(void) {
#ifdef HAVE_ITERATE_H
	struct T_(array) empty = T_(array)(), one = T_(array)();
	PT_(type) *t;
	t = T_(array_new)(&one);
	assert(t);
	T_(filler)(t);
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
#endif
}

static void PT_(test_trim)(void) {
#ifdef HAVE_ITERATE_H
	struct T_(array) a = T_(array)();
	PT_(type) *item;
	int is_zero;
	/* Trim 1. */
	item = T_(array_new)(&a);
	assert(item);
	memset(item, 0, sizeof *item);
	T_(array_trim)(&a, &PT_(zero_filled));
	assert(a.size == 0);
	/* Trim 3. */
	item = T_(array_new)(&a);
	assert(item);
	memset(item, 0, sizeof *item);
	item = T_(array_new)(&a);
	assert(item);
	T_(filler)(item);
	is_zero = PT_(zero_filled)(item);
	item = T_(array_new)(&a);
	assert(item);
	memset(item, 0, sizeof *item);
	T_(array_trim)(&a, &PT_(zero_filled));
	assert(a.size == !is_zero);
	T_(array_)(&a);
#endif
}

static void PT_(test_insert)(void) {
	struct T_(array) a = T_(array)();
	PT_(type) original[17], solitary, *t, *t1, *e;
	const size_t original_size = sizeof original / sizeof *original;
	size_t i;
	printf("Test insert:\n");
	memset(original, 0, sizeof original); /* Valgrind. */
	PT_(valid_state)(&a);
	for(t = original, t1 = t + original_size; t < t1; t++) T_(filler)(t);
	T_(filler)(&solitary);
	for(i = 0; i <= original_size; i++) {
		e = T_(array_append)(&a, original_size), assert(e);
		memcpy(e, original, sizeof original);
		if(!i) printf("a = %s.\n", T_(array_to_string)(&a));
		PT_(valid_state)(&a);
		e = T_(array_insert)(&a, 1, i), assert(e);
		memcpy(e, &solitary, sizeof solitary);
		printf("After insert(%lu) a = %s.\n",
			(unsigned long)i, T_(array_to_string)(&a));
		T_(array_clear)(&a);
	}
	T_(array_)(&a);
}

/** `ARRAY_TEST`, `ARRAY_TO_STRING`, !`NDEBUG`: will be tested on stdout.
 @allow */
static void T_(array_test)(void) {
	printf("array<" QUOTE(ARRAY_NAME) "> of type <" QUOTE(ARRAY_TYPE)
		"> was created using: ARRAY_TO_STRING <" QUOTE(ARRAY_TO_STRING) ">; "
		"ARRAY_TEST <" QUOTE(ARRAY_TEST) ">; testing:\n");
	assert(T_(to_string) && T_(array_to_string));
	PT_(test_basic)();
	PT_(test_random)();
	PT_(test_replace)();
	PT_(test_keep)();
	PT_(test_each)();
	PT_(test_trim)();
	PT_(test_insert)();
	fprintf(stderr, "Done tests of array<" QUOTE(ARRAY_NAME) ">.\n\n");
}

#undef QUOTE
#undef QUOTE_

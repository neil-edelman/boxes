#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** Fills `fill` that is not equal to `neq` if possible. */
static int PCMP_(fill_unique)(PA_(type) *const fill,
	const PA_(type) *const neq) {
	size_t i;
	assert(fill);
	for(i = 0; i < 1000; i++) {
		A_(filler)(fill);
		if(!neq || !CMPEXTERN_(is_equal)(neq, fill)) return 1;
	}
	assert(0); return 0;
}

#if 0 /* <!-- 0: I don't think we use this anymore? */
#ifdef ARRAY_COMPARE /* <!-- comp */
static int PCMP_(unique_array)(PA_(type) *const fill, const size_t size) {
	const size_t no_try = 5000;
	size_t attempt, i = 0;
	for(attempt = 0; attempt < no_try; attempt++) {
		size_t back;
		char z[12];
		A_(filler)(fill + i);
		A_(to_string)(fill + i, &z), printf("unique_array: %s?\n", z);
		for(back = i; back && !CMPEXTERN_(is_equal)(fill + back - 1, fill + i);
			back--);
		if(back)
			{ printf("unique_array: %s is duplicated.\n", z); continue; }
		if(++i >= size) break;
	}
	if(attempt == no_try) {
		return printf("unique_array: couldn't get unique entries from "
			QUOTE(LIST_NAME) " in %lu tries; giving up.\n",
			(unsigned long)no_try), 0;
	}
	return printf("unique_array: got duplicates in %lu tries.\n",
		(unsigned long)attempt), 1;
}
#endif /* comp --> */
#endif /* 0 */

static void PCMP_(test_compactify)(void) {
	struct A_(array) a = A_(array)();
	PA_(type) ts[9], *t, *t1, *t_prev;
	const size_t ts_size = sizeof ts / sizeof *ts;
	/* `valgrind` is giving me grief if I don't do this? */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	assert(ts_size % 3 == 0);
	for(t_prev = 0, t = ts, t1 = t + ts_size; t < t1; t_prev = t, t += 3) {
		if(!PCMP_(fill_unique)(t, t_prev)) { assert(0); return; }
		memcpy(t + 1, t, sizeof *t);
		memcpy(t + 2, t, sizeof *t);
	}
	if(!A_(array_append)(&a, ts_size)) { assert(0); return; }
	memcpy(a.data, ts, sizeof *t * ts_size);
	printf("\ntest compactify: %s.\n", A_(array_to_string)(&a));
	CMP_(unique)(&a);
	printf("Compactified: %s.\n", A_(array_to_string)(&a));
	assert(a.size == ts_size / 3);
#ifdef ARRAY_COMPARE /* <!-- compare */
	CMP_(reverse)(&a);
	printf("Reverse: %s.\n", A_(array_to_string)(&a));
	for(t = a.data, t1 = a.data + a.size - 1; t < t1; t++)
		assert(CMPEXTERN_(compare)(t, t + 1) >= 0);
	CMP_(sort)(&a);
	printf("Sorted: %s.\n", A_(array_to_string)(&a));
	for(t = a.data, t1 = a.data + a.size - 1; t < t1; t++)
		assert(CMPEXTERN_(compare)(t, t + 1) <= 0);
#endif /* compare --> */
	A_(array_)(&a);
}

static void PCMP_(test_compare)(void) {
	struct A_(array) a = A_(array)(), b = A_(array)();
	struct A_(array_iterator) it;
	PA_(type) ts[9], *t, *t1;
	const size_t ts_size = sizeof ts / sizeof *ts;
	size_t i;
	int cmp;
	/* `valgrind` is giving me grief if I don't do this? */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	for(t = ts, t1 = t + ts_size; t < t1; t++) A_(filler)(t);
	if(!A_(array_append)(&a, ts_size)) { assert(0); return; }
	memcpy(a.data, ts, sizeof *t * ts_size);
	printf("\ntest compare: %s.\n", A_(array_to_string)(&a));
	assert(ts_size == a.size);
	t = 0, i = 0;
	it = A_(array_iterator)(&a);
	while(t = A_(array_next)(&it)) {
		char z[12];
		A_(to_string)(t, &z);
		printf("next %s; ", z);
		assert(t = a.data + i++);
	}
	assert(i == ts_size);
	while(t = A_(array_previous)(&it)) {
		char z[12];
		A_(to_string)(t, &z);
		printf("previous %s; ", z);
		assert(t = a.data + --i);
	}
	printf("done.\n");
	assert(!i);
	cmp = CMP_(is_equal)(0, 0), assert(cmp);
	printf("a: %s.\n"
		"b: %s.\n", A_(array_to_string)(&a), A_(array_to_string)(&b));
	cmp = CMP_(is_equal)(&a, &b), assert(!cmp);
	cmp = CMP_(is_equal)(&a, 0), assert(!cmp);
	cmp = CMP_(is_equal)(0, &b), assert(cmp); /* Null == size 0. */
	if(!A_(array_append)(&b, ts_size)) { assert(0); return; }
	memcpy(b.data, ts, sizeof *t * ts_size);
	printf("now b: %s.\n", A_(array_to_string)(&b));
	cmp = CMP_(is_equal)(&a, &b), assert(cmp);
	A_(array_)(&a);
	A_(array_)(&b);
}

#ifdef ARRAY_COMPARE /* <!-- comp */
static int PCMP_(cmp_void)(const void *const a, const void *const b)
	{ return CMP_(compare)(a, b); }
#endif /* comp --> */

static void PCMP_(test_sort)(void) {
#ifdef ARRAY_COMPARE /* <!-- comp */
	struct A_(array) as[64], *a;
	const size_t as_size = sizeof as / sizeof *as;
	const struct A_(array) *const as_end = as + as_size;
	int cmp;
	printf("\ntest sort:\n");
	/* Random array of Arrays. */
	for(a = as; a < as_end; a++) {
		size_t size = (unsigned)rand() / (RAND_MAX / 5 + 1), i;
		PA_(type) *x, *x_end;
		*a = A_(array)();
		x = A_(array_append)(a, size);
		x_end = x + size;
		if(!size) continue;
		assert(x);
		for(i = 0; i < size; i++) A_(filler)(a->data + i); /* Emplace. */
		CMP_(sort)(a);
		for(x = a->data; x < x_end - 1; x++)
			cmp = CMPEXTERN_(compare)(x, x + 1), assert(cmp <= 0);
	}
	/* Now sort the lists. */
	qsort(as, as_size, sizeof *as, &PCMP_(cmp_void));
	printf("Sorted array of sorted <" QUOTE(ARRAY_NAME) ">array by "
		   QUOTE(ARRAY_COMPARE) ":\n");
	for(a = as; a < as_end; a++) {
		printf("array: %s.\n", A_(array_to_string)(a));
		if(a == as) continue;
		cmp = CMP_(compare)(a - 1, a);
		assert(cmp <= 0);
	}
	for(a = as; a < as_end; a++) A_(array_)(a);
#endif /* comp --> */
}

static void PCMP_(test_bounds)(void) {
#ifdef ARRAY_COMPARE /* <!-- compare */
	struct A_(array) a = A_(array)();
	const size_t size = 10;
	size_t i, low, high;
	PA_(type) elem, *cont;
	int ret;
	char z[12];
	printf("\ntest bounds:\n");
	if(!A_(array_append)(&a, size)) { assert(0); return; }
	for(i = 0; i < size; i++) A_(filler)(a.data + i);
	memset(&elem, 0, sizeof elem), A_(filler)(&elem);
	CMP_(sort)(&a);
	printf("sorted: %s.\n", A_(array_to_string)(&a));
	A_(to_string)(&elem, &z);
	printf("element to compare: %s\n", z);
	low = CMP_(lower_bound)(&a, &elem);
	printf(QUOTE(ARRAY_COMPARE) " lower_bound: %lu.\n", (unsigned long)low);
	assert(low <= a.size);
	assert(!low || CMPEXTERN_(compare)(a.data + low - 1, &elem) < 0);
	assert(low == a.size || CMPEXTERN_(compare)(&elem, a.data + low) <= 0);
	high = CMP_(upper_bound)(&a, &elem);
	printf(QUOTE(ARRAY_COMPARE) " upper_bound: %lu.\n", (unsigned long)high);
	assert(high <= a.size);
	assert(!high || CMPEXTERN_(compare)(a.data + high - 1, &elem) <= 0);
	assert(high == a.size || CMPEXTERN_(compare)(&elem, a.data + high) < 0);
	A_(to_string)(&elem, &z);
	printf("insert: %s into %s of size %lu.\n",
		z, A_(array_to_string)(&a), (unsigned long)size);
	ret = CMP_(insert_after)(&a, &elem);
	assert(ret && a.size == size + 1);
	ret = memcmp(&elem, a.data + low, sizeof elem), assert(!ret);
	A_(array_clear)(&a);
	cont = A_(array_append)(&a, size), assert(cont);
	for(i = 0; i < size; i++) memcpy(cont + i, &elem, sizeof elem);
	printf("bounds: now %s.\n", A_(array_to_string)(&a));
	low = CMP_(lower_bound)(&a, &elem);
	printf(QUOTE(ARRAY_COMPARE) " lower_bound: %lu.\n", (unsigned long)low);
	assert(low == 0);
	high = CMP_(upper_bound)(&a, &elem);
	printf(QUOTE(ARRAY_COMPARE) " upper_bound: %lu.\n", (unsigned long)high);
	assert(high == a.size);
	A_(array_)(&a);
#endif /* compare --> */
}

/** `ARRAY_TEST`, `ARRAY_COMPARE` -> `ARRAY_TO_STRING`, !`NDEBUG`: will be
 tested on stdout. @allow */
static void CMP_(compare_test)(void) {
	printf("<" QUOTE(ARRAY_NAME) ">array testing <"
#ifdef ARRAY_COMPARE_NAME
		QUOTE(ARRAY_COMPARE_NAME)
#else
		"anonymous"
#endif
		"> "
#ifdef ARRAY_COMPARE
		"compare <" QUOTE(ARRAY_COMPARE)
#elif defined(ARRAY_IS_EQUAL)
		"is equal <" QUOTE(ARRAY_IS_EQUAL)
#endif
		">:\n");
	errno = 0;
	PCMP_(test_sort)();
	PCMP_(test_bounds)();
	PCMP_(test_compactify)();
	PCMP_(test_compare)();
	assert(errno == 0);
	fprintf(stderr, "Done tests of <" QUOTE(ARRAY_NAME) ">array compare.\n\n");
}

#undef QUOTE
#undef QUOTE_

/* fixme: This isn't mutation tested; I would have to go into it to see what I
 did. */

#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** Fills `fill` that is not equal to `neq` if possible. */
static int pTN_(fill_unique)(pT_(type) *const fill,
	const pT_(type) *const neq) {
	size_t i;
	assert(fill);
	for(i = 0; i < 1000; i++) {
		t_(filler)(fill);
		if(!neq || !t_(is_equal)(neq, fill)) return 1;
	}
	assert(0); return 0;
}

#if 0 /* <!-- 0: I don't think we use this anymore? */
#ifdef BOX_COMPARE /* <!-- comp */
static int PCMP_(unique_array)(PA_(type) *const fill, const size_t size) {
	const size_t no_try = 5000;
	size_t attempt, i = 0;
	for(attempt = 0; attempt < no_try; attempt++) {
		size_t back;
		char z[12];
		A_(filler)(fill + i);
		A_(to_string)(fill + i, &z), printf("unique_array: %s?\n", z);
		for(back = i; back && !CMPCALL_(is_equal)(fill + back - 1, fill + i);
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

/* fixme: This is not general. */
static void pTN_(test_compactify)(void) {
	struct t_(array) a = t_(array)();
	pT_(type) ts[9], *t, *t1, *t_prev;
	const size_t ts_size = sizeof ts / sizeof *ts;
	/* `valgrind` is giving me grief if I don't do this? */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	assert(ts_size % 3 == 0);
	for(t_prev = 0, t = ts, t1 = t + ts_size; t < t1; t_prev = t, t += 3) {
		if(!pTN_(fill_unique)(t, t_prev)) { assert(0); return; }
		memcpy(t + 1, t, sizeof *t);
		memcpy(t + 2, t, sizeof *t);
	}
	if(!T_(append)(&a, ts_size)) { assert(0); return; }
	memcpy(a.data, ts, sizeof *t * ts_size);
	printf("\ntest compactify: %s.\n", T_(to_string)(&a));
	TN_(unique)(&a);
	printf("Compactified: %s.\n", T_(to_string)(&a));
	assert(a.size == ts_size / 3);
#ifdef BOX_COMPARE /* <!-- compare */
	TN_(reverse)(&a);
	printf("Reverse: %s.\n", T_(to_string)(&a));
	/* "Discards qualifiers in nested pointer types" sometimes. Cast. */
	for(t = a.data, t1 = a.data + a.size - 1; t < t1; t++)
		assert(tn_(compare)((const void *)t, (const void *)(t + 1)) >= 0);
	TN_(sort)(&a);
	printf("Sorted: %s.\n", T_(to_string)(&a));
	/* "Discards qualifiers in nested pointer types" sometimes. Cast. */
	for(t = a.data, t1 = a.data + a.size - 1; t < t1; t++)
		assert(tn_(compare)((const void *)t, (const void *)(t + 1)) <= 0);
#endif /* compare --> */
	t_(array_)(&a);
}

static void pTN_(test_compare)(void) {
	struct t_(array) a = t_(array)(), b = t_(array)();
	/*struct A_(array_iterator) it;*/
	pT_(type) ts[9], *t, *t1;
	const size_t ts_size = sizeof ts / sizeof *ts;
	/*size_t i;*/
	int cmp;
	/* `valgrind` is giving me grief if I don't do this? */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	for(t = ts, t1 = t + ts_size; t < t1; t++) t_(filler)(t);
	if(!T_(append)(&a, ts_size)) { assert(0); return; }
	memcpy(a.data, ts, sizeof *t * ts_size);
	printf("\ntest compare: %s.\n", T_(to_string)(&a));
	assert(ts_size == a.size);
#if 0 /* I don't use iterators. */
	t = 0, i = 0;
	it = A_(array_begin)(&a);
	while(t = A_(array_next)(&it)) {
		char z[12];
		A_(to_string)((void *)t, &z);
		printf("next %s; ", z);
		assert(t = a.data + i++);
	}
	assert(i == ts_size);
	while(t = A_(array_previous)(&it)) {
		char z[12];
		A_(to_string)((void *)t, &z);
		printf("previous %s; ", z);
		assert(t = a.data + --i);
	}
	printf("done.\n");
	assert(!i);
#endif
	cmp = TN_(is_equal)(0, 0), assert(cmp);
	printf("a: %s.\n"
		"b: %s.\n", T_(to_string)(&a), T_(to_string)(&b));
	cmp = TN_(is_equal)(&a, &b), assert(!cmp);
	cmp = TN_(is_equal)(&a, 0), assert(!cmp);
	cmp = TN_(is_equal)(0, &b), /*assert(cmp)*/assert(!cmp); /* Null == size 0. <- nah */
	if(!T_(append)(&b, ts_size)) { assert(0); return; }
	memcpy(b.data, ts, sizeof *t * ts_size);
	printf("now b: %s.\n", T_(to_string)(&b));
	cmp = TN_(is_equal)(&a, &b), assert(cmp);
	t_(array_)(&a);
	t_(array_)(&b);
}

#ifdef BOX_COMPARE /* <!-- comp */
static int pTN_(cmp_void)(const void *const a, const void *const b)
	{ return TN_(compare)(a, b); }
#endif /* comp --> */

static void pTN_(test_sort)(void) {
#ifdef BOX_COMPARE /* <!-- comp */
	struct t_(array) as[64], *a;
	const size_t as_size = sizeof as / sizeof *as;
	const struct t_(array) *const as_end = as + as_size;
	int cmp;
	printf("\ntest sort:\n");
	/* Random array of Arrays. */
	for(a = as; a < as_end; a++) {
		size_t size = (unsigned)rand() / (RAND_MAX / 5 + 1), i;
		pT_(type) *x, *x_end;
		*a = t_(array)();
		x = T_(append)(a, size);
		x_end = x + size;
		if(!size) continue;
		assert(x);
		for(i = 0; i < size; i++) t_(filler)(a->data + i); /* Emplace. */
		TN_(sort)(a);
		for(x = a->data; x < x_end - 1; x++)
			cmp = tn_(compare)((void *)x, (void *)(x + 1)),
			assert(cmp <= 0);
		/* fixme: Why the void casts again? */
	}
	/* Now sort the lists. */
	qsort(as, as_size, sizeof *as, &pTN_(cmp_void));
	printf("Sorted array of sorted <" QUOTE(BOX_NAME) ">array by "
		   QUOTE(BOX_COMPARE) ":\n");
	for(a = as; a < as_end; a++) {
		printf("array: %s.\n", T_(to_string)(a));
		if(a == as) continue;
		cmp = TN_(compare)(a - 1, a);
		assert(cmp <= 0);
	}
	for(a = as; a < as_end; a++) t_(array_)(a);
#endif /* comp --> */
}

static void pTN_(test_bounds)(void) {
#ifdef COMPARE /* <!-- compare */
	/* fixme */
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
	A_(to_string)((void *)&elem, &z);
	printf("element to compare: %s\n", z);
	low = CMP_(lower_bound)(&a, &elem);
	printf(QUOTE(BOX_COMPARE) " lower_bound: %lu.\n", (unsigned long)low);
	assert(low <= a.size);
	assert(!low || CMPCALL_(compare)((void *)(a.data + low - 1),
		(void *)&elem) < 0);
	assert(low == a.size || CMPCALL_(compare)((void *)&elem,
		(void *)(a.data + low)) <= 0);
	high = CMP_(upper_bound)(&a, &elem);
	printf(QUOTE(BOX_COMPARE) " upper_bound: %lu.\n", (unsigned long)high);
	assert(high <= a.size);
	assert(!high || CMPCALL_(compare)((void *)(a.data + high - 1),
		(void *)&elem) <= 0);
	assert(high == a.size || CMPCALL_(compare)((void *)&elem,
		(void *)(a.data + high)) < 0);
	A_(to_string)((void *)&elem, &z);
	printf("insert: %s into %s of size %lu.\n",
		z, A_(array_to_string)(&a), (unsigned long)size);
	ret = CMP_(insert_after)(&a, &elem);
	assert(ret && a.size == size + 1);
	/*printf("insert: %s into %s of size %lu.\n",
		z, A_(array_to_string)(&a), (unsigned long)a.size); oops? */
	ret = memcmp(&elem, a.data + /*low*/high, sizeof elem), assert(!ret);
	A_(array_clear)(&a);
	cont = A_(array_append)(&a, size), assert(cont);
	for(i = 0; i < size; i++) memcpy(cont + i, &elem, sizeof elem);
	printf("bounds: now %s.\n", A_(array_to_string)(&a));
	low = CMP_(lower_bound)(&a, &elem);
	printf(QUOTE(BOX_COMPARE) " lower_bound: %lu.\n", (unsigned long)low);
	assert(low == 0);
	high = CMP_(upper_bound)(&a, &elem);
	printf(QUOTE(BOX_COMPARE) " upper_bound: %lu.\n", (unsigned long)high);
	assert(high == a.size);
	A_(array_)(&a);
#endif /* compare --> */
}

/** `BOX_TEST`, `BOX_COMPARE` -> `BOX_TO_STRING`, !`NDEBUG`: will be
 tested on stdout. @allow */
static void TN_(compare_test)(void) {
	printf("<" QUOTE(BOX_NAME) ","
#ifdef BOX_TRAIT
		QUOTE(BOX_TRAIT)
#else
		"anonymous"
#endif
		">array testing compare:\n");
	errno = 0;
	pTN_(test_sort)();
	pTN_(test_bounds)();
	pTN_(test_compactify)();
	pTN_(test_compare)();
	assert(errno == 0);
	fprintf(stderr, "Done tests of <" QUOTE(BOX_NAME) ">array compare.\n\n");
}

#undef QUOTE
#undef QUOTE_

/* Intended to be included by {Array.h} on {ARRAY_TYPE_FILLER}. */

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



static void PT_(valid_state)(const struct T_(Array) *const a) {
	const size_t max_size = (size_t)-1 / sizeof *a->data;
	/* Null is a valid state. */
	if(!a) return;
	if(!a->data) { assert(!a->size); return; }
	assert(a->size <= a->capacity);
	assert(a->capacity < a->next_capacity
		|| (a->capacity == a->next_capacity) == max_size);
}

static int PT_(zero_filled)(const T *const t) {
	const char *c = (const char *)t,
		*const end = (const char *)(t + 1);
	assert(t);
	while(c < end) if(*c++) return 0;
	return 1;
}



static void PT_(test_basic)(void) {
	struct T_(Array) a;
	T ts[5], *t, *t1;
	const size_t ts_size = sizeof ts / sizeof *ts, big = 1000;
	size_t i;
	int is_zero;

	printf("Test null.\n");
	errno = 0;
	T_(Array_)(0);
	T_(Array)(0);
#ifndef ARRAY_STACK /* <-- !stack */
	assert(T_(ArrayRemove)(0, 0) == 0);
#endif /* !stack --> */
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
	assert(T_(ArraySize)(&a) == 0);
#ifndef ARRAY_STACK /* <-- !stack */
	assert(T_(ArrayRemove)(&a, 0) == 0 && errno == 0);
	assert(T_(ArrayRemove)(&a, t) == 0 && errno == EDOM), errno = 0;
#endif /* !stack --> */
	assert(T_(ArrayGet)(&a) == 0);
	assert(T_(ArrayPeek)(0) == 0);
	assert(T_(ArrayPop)(0) == 0);
	assert(T_(ArrayNext)(0, 0) == 0 && T_(ArrayNext)(0, t) == 0);
	T_(ArrayEach)(&a, 0);
	assert(errno == 0);
	PT_(valid_state)(&a);

	/* @fixme valgrind is giving me grief if I don't do this? */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	for(t = ts, t1 = t + ts_size; t < t1; t++) PT_(filler)(t);

	printf("Test one element.\n");
	t = T_(ArrayNew)(&a); /* Add. */
	assert(t);
	t1 = T_(ArrayNext)(&a, 0);
	assert(t == t1);
	t1 = T_(ArrayNext)(&a, t1);
	assert(t1 == 0);
	t1 = T_(ArrayBack)(&a, 0);
	assert(t == t1);
	t1 = T_(ArrayBack)(&a, t1);
	assert(t1 == 0);
	assert(T_(ArrayIndex)(&a, t) == 0);
	assert(T_(ArrayPeek)(&a) == t);
	assert(T_(ArrayGet(&a)) == t);
	t1 = T_(ArrayPop)(&a); /* Remove. */
	assert(t1 == t);
	assert(T_(ArrayPeek)(&a) == 0);
	t = T_(ArrayNew)(&a); /* Add. */
	assert(t);
	T_(ArrayClear)(&a);
	assert(T_(ArrayPeek)(&a) == 0);
	PT_(valid_state)(&a);

	printf("Testing %lu elements.\n", (unsigned long)ts_size);
	for(i = 0; i < ts_size; i++) {
		t = T_(ArrayNew)(&a);
		assert(t);
		memcpy(t, ts + i, sizeof *t);
	}
	assert(T_(ArrayPeek)(&a));
	printf("Now: %s.\n", T_(ArrayToString)(&a));
	assert(T_(ArraySize)(&a) == ts_size);
#ifdef ARRAY_STACK /* <-- stack */
#else /* stack --><-- !stack */
	if((t = T_(ArrayGet)(&a) + ts_size - 2)
		&& !T_(ArrayRemove)(&a, t)) {
		perror("Error"), assert(0);
		return;
	}
	printf("Now: %s.\n", T_(ArrayToString)(&a));
	assert(!T_(ArrayRemove)(&a, t + 1) && errno == EDOM);
	printf("(Deliberate) error: %s.\n", strerror(errno)), errno = 0;
	if((t = T_(ArrayGet)(&a) + ts_size - 3)
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
#endif /* !stack --> */

	/* Peek/Pop. */
	t = T_(ArrayPeek)(&a);
	assert(t && !memcmp(t, ts + ts_size - 1, sizeof *t));
	t = T_(ArrayPop)(&a);
	assert(t && !memcmp(t, ts + ts_size - 1, sizeof *t));
	t = T_(ArrayPop)(&a);
	assert(t && !memcmp(t, ts + ts_size - 2, sizeof *t));
	T_(ArrayClear)(&a);
	assert(T_(ArraySize)(&a) == 0);

	/* Trim 1. */
	t = T_(ArrayNew)(&a);
	assert(t);
	memset(t, 0, sizeof *t);
	T_(ArrayTrim)(&a, &PT_(zero_filled));
	assert(T_(ArraySize)(&a) == 0);
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
	assert(T_(ArraySize)(&a) == !is_zero);

	/* Big. */
	for(i = 0; i < big; i++) {
		t = T_(ArrayNew)(&a);
		assert(t);
		PT_(filler)(t);
	}
	printf("%s.\n", T_(ArrayToString)(&a));
	PT_(valid_state)(&a);
	for(i = 0, t = 0; (t = T_(ArrayNext)(&a, t)); i++);
	assert(a.size == i);
	for(i = 0, t = 0; (t = T_(ArrayBack)(&a, t)); i++);
	assert(a.size == i);
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

static void PT_(test_random)(void) {
	struct T_(Array) a;
	size_t i, size = 0;
	const size_t mult = 1; /* For long tests. */
	/* Random. */
	T_(Array)(&a);
	/* This parameter controls how many iterations. */
	i = 1000 * mult;
	while(i--) {
		T *data;
		char str[12];
		double r = rand() / (RAND_MAX + 1.0);
		/* This parameter controls how big the pool wants to be. */
		if(r > size / (100.0 * mult)) {
			if(!(data = T_(ArrayNew)(&a))) {
				perror("Error"), assert(0);
				return;
			}
			size++;
			PT_(filler)(data);
			PT_(to_string)(data, &str);
			printf("Created %s.\n", str);
		} else {
#ifdef ARRAY_STACK /* <-- stack */
			double t = 1.0;
#else /* stack --><-- !stack */
			double t = 0.5;
#endif /* !stack --> */
			r = rand() / (RAND_MAX + 1.0);
			if(r < t) {
				data = T_(ArrayPeek)(&a);
				assert(data);
				PT_(to_string)(data, &str);
				printf("Popping %s.\n", str);
				assert(data == T_(ArrayPop)(&a));
			} else {
#ifndef ARRAY_STACK /* <-- !stack */
				size_t idx = rand() / (RAND_MAX + 1.0) * size;
				if(!(data = T_(ArrayGet)(&a) + idx)) continue;
				PT_(to_string)(data, &str);
				printf("Removing %s at %lu.\n", str, (unsigned long)idx);
				{
					const int ret = T_(ArrayRemove)(&a, data);
					assert(ret || (perror("Removing"), 0));
				}
#endif /* !stack --> */
			}
			size--;
		}
		printf("%s.\n", T_(ArrayToString)(&a));
		PT_(valid_state)(&a);
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
	success = T_(ArrayIndexReplace)(0, 0, 0, 0);
	assert(!success && !errno);
	success = T_(ArrayReplace)(0, 0, 0, 0);
	assert(!success && !errno);
	/* a == b. */
	success = T_(ArrayIndexReplace)(&a, 0, 0, &a);
	assert(!success && errno == EDOM);
	errno = 0;
	success = T_(ArrayReplace)(&a, 0, 0, &a);
	assert(!success && errno == EDOM);
	errno = 0;
	/* Out-of-bounds. */
	success = T_(ArrayIndexReplace)(&a, 0, T_(ArraySize)(&a) + 1, 0);
	assert(!success && errno == EDOM);
	errno = 0;
	/* Large */
	success = T_(ArrayReplace)(&a, 0, 65536, 0);
	assert(!success && errno == ERANGE);
	errno = 0;
	/* e0 > e1. */
	success = T_(ArrayIndexReplace)(&a, 1, 0, &b);
	assert(!success && errno == EDOM);
	errno = 0;
	/* No-op. */
	success = T_(ArrayIndexReplace)(&a, 0, 0, 0);
	printf("Array %s.\n", T_(ArrayToString)(&a));
	assert(success && T_(ArraySize)(&a) == ts_size);
	/* Deleting from the front. */
	success = T_(ArrayIndexReplace)(&a, 0, 1, 0);
	printf("Array after deleting from front %s.\n", T_(ArrayToString)(&a));
	assert(success && T_(ArraySize)(&a) == ts_size - 1);
	/* Adding at the back. */
	t = T_(ArrayNew)(&b);
	assert(t);
	memcpy(t, ts + 0, sizeof *t);
	success = T_(ArrayIndexReplace)(&a, T_(ArraySize)(&a), T_(ArraySize)(&a),
		&b);
	printf("Array after adding %s to back %s.\n", T_(ArrayToString)(&b),
		T_(ArrayToString)(&a));
	assert(success && T_(ArraySize)(&a) == ts_size);
	/* Replacing same-size. */
	success = T_(ArrayIndexReplace)(&a, 1, 2, &b);
	printf("Array after replacing [1, 2) %s: %s.\n", T_(ArrayToString)(&b),
		T_(ArrayToString)(&a));
	assert(success && T_(ArraySize)(&a) == ts_size
		&& !memcmp(t, T_(ArrayGet)(&a) + 1, sizeof *t));
	/* Replacing larger size. */
	t = T_(ArrayNew)(&b);
	assert(t);
	memcpy(t, ts + 1, sizeof *t);
	success = T_(ArrayIndexReplace)(&a, 1, 2, &b);
	printf("Array after replacing [1, 2) %s: %s.\n", T_(ArrayToString)(&b),
		T_(ArrayToString)(&a));
	assert(success && T_(ArraySize)(&a) == ts_size + 1
		   && !memcmp(t, T_(ArrayGet)(&a) + 2, sizeof *t));
	/* Replacing a smaller size. */
	success = T_(ArrayIndexReplace)(&a, 1, 4, &b);
	printf("Array after replacing [1, 4) %s: %s.\n", T_(ArrayToString)(&b),
		T_(ArrayToString)(&a));
	assert(success && T_(ArraySize)(&a) == ts_size
		   && !memcmp(t, T_(ArrayGet)(&a) + 2, sizeof *t));
	T_(ArrayClear)(&b);
	t = T_(ArrayBuffer)(&b, 2);
	assert(t);
	memcpy(t, ts + 2, sizeof *t * 2);
	T_(ArrayExpand)(&b, 2);
	/* a = [[1],[0],[1],[4],[0]]; b = [[2],[3]] */
	printf("a = %s, b = %s.\n", T_(ArrayToString)(&a), T_(ArrayToString)(&b));
	T_(ArrayReplace)(&a, 0, -1, &b);
	printf("a = %s.\n", T_(ArrayToString)(&a));
	/* a = [[1],[0],[1],[4],[0],[2],[3]] */
	T_(ArrayReplace)(&a, T_(ArrayGet)(&a) + 1, 2, &b);
	printf("a = %s.\n", T_(ArrayToString)(&a));
	/* a = [[1],[2],[3],[4],[0],[2],[3]] */
	T_(ArrayReplace)(&a, T_(ArrayGet)(&a) + 2, -5, &b);
	printf("a = %s.\n", T_(ArrayToString)(&a));
	/* a = [[1],[2],[2],[3],[4],[0],[2],[3]] */
	T_(ArrayReplace)(&a, T_(ArrayGet)(&a) + 7, -1, &b);
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
	struct T_(Array) a = ARRAY_ZERO;
	PT_(valid_state)(&a);
	for(t = ts, t1 = t + ts_size; t < t1; t++) {
		PT_(filler)(t);
		e = T_(ArrayNew)(&a), assert(e);
		memcpy(e, t, sizeof *t);
	}
	T_(ArrayKeepIf)(&a, &PT_(keep_one));
	assert(T_(ArraySize)(&a) == 7
		&& !memcmp(ts + 0, T_(ArrayGet)(&a) + 0, sizeof *t * 1)
		&& !memcmp(ts + 5, T_(ArrayGet)(&a) + 1, sizeof *t * 1)
		&& !memcmp(ts + 8, T_(ArrayGet)(&a) + 2, sizeof *t * 2)
		&& !memcmp(ts + 11, T_(ArrayGet)(&a) + 4, sizeof *t * 1)
		&& !memcmp(ts + 13, T_(ArrayGet)(&a) + 5, sizeof *t * 1)
		&& !memcmp(ts + 15, T_(ArrayGet)(&a) + 6, sizeof *t * 1));
	T_(Array_)(&a);
}

/** The list will be tested on stdout. */
static void T_(ArrayTest)(void) {
	printf("Array<" QUOTE(ARRAY_NAME) ">: of type <" QUOTE(ARRAY_TYPE)
		"> was created using: "
#ifdef ARRAY_STACK
		"ARRAY_STACK; "
#endif
#ifdef ARRAY_TAIL_DELETE
		"ARRAY_TAIL_DELETE; "
#endif
#ifdef ARRAY_MIGRATE_EACH
		"ARRAY_MIGRATE_EACH<" QUOTE(ARRAY_MIGRATE_EACH) ">; "
#endif
#ifdef ARRAY_MIGRATE_UPDATE
		"ARRAY_MIGRATE_UPDATE<" QUOTE(ARRAY_MIGRATE_UPDATE) ">; "
#endif		   
#ifdef ARRAY_TO_STRING
		"ARRAY_TO_STRING<" QUOTE(ARRAY_TO_STRING) ">; "
#endif
#ifdef ARRAY_TEST
		"ARRAY_TEST<" QUOTE(ARRAY_TEST) ">; "
#endif
		"testing:\n");
	PT_(test_basic)();
	PT_(test_random)();
	PT_(test_replace)();
	PT_(test_keep)();
	fprintf(stderr, "Done tests of Array<" QUOTE(ARRAY_NAME) ">.\n\n");
}

/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_

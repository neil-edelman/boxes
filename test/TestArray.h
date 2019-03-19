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
	/* Null is a valid state. */
	if(!a) return;
	assert(a->size <= a->capacity[0]);
	assert(a->capacity[0] < a->capacity[1] || (a->capacity[0] == a->capacity[1]
		&& (a->capacity[1] == pool_max / sizeof(struct PT_(Node))
		|| a->capacity[0] == 0)));
#ifdef ARRAY_FREE_LIST /* <-- !stack */
	{
		struct PT_(Node) *node;
		size_t i, remove_start = 0, remove_end =0,remove_both=0,remove_data = 0;
		size_t prev, next;
		enum { SDATA, SNULL, SVOID } prev_class, next_class;
		assert((a->removed.next == pool_null) == (a->removed.prev ==pool_null));
		for(i = 0; i < a->size; i++) {
			node = a->nodes + i;
			prev = node->x.prev;
			next = node->x.next;
			/* five states: info:info, null:info, info:null, null:null, and not
			 part of the removed queue; check for invalid states */
			if(prev == pool_null) prev_class = SNULL;
			else if(prev == pool_void) prev_class = SVOID;
			else prev_class = SDATA;
			if(next == pool_null) next_class = SNULL;
			else if(next == pool_void) next_class = SVOID;
			else next_class = SDATA;
			/* count */
			if(prev_class == SVOID && next_class == SVOID);
			else if(prev_class == SNULL && next_class == SNULL) remove_both++;
			else if(prev_class == SNULL && next_class == SDATA) remove_start++;
			else if(prev_class == SDATA && next_class == SDATA) remove_data++;
			else if(prev_class == SDATA && next_class == SNULL) remove_end++;
			else assert((printf("Invalid state.\n"), 0));
		}
		assert((remove_both == 0 && remove_start == 0 && remove_end == 0)
			|| (a->size >= 2 && remove_both == 1 && remove_start == 0
			&& remove_end == 0)
			|| (a->size > 2 && remove_both == 0 && remove_start == 1
			&& remove_end == 1));
		assert(a->size == 0 || a->nodes[a->size - 1].x.prev == pool_void);
	}
#endif /* !stack --> */
}

#ifdef ARRAY_MIGRATE_ALL /* <-- all */
static S dummy_parent;
/** @implements Migrate */
static void PT_(migrate)(S *const parent,
	const struct Migrate *const migrate) {
	assert(parent && parent == &dummy_parent && migrate);
	printf("#%p migrate #%p-%p -> %p\n", (void *)parent, migrate->begin,
		migrate->end, (void *)migrate->delta);
	/* @fixme Check. */
}
#endif /* all --> */



/*
 static void T_(Array_)(struct T_(Array) *const pool);
 #ifdef ARRAY_MIGRATE_ALL * <-- all *
 static void T_(Array)(struct T_(Array) *const pool,
 const PT_(MigrateAll) migrate_all, A *const all);
 #else * all --><-- !all *
 static void T_(Array)(struct T_(Array) *const pool);
 #endif * all --> *
 #ifndef ARRAY_FREE_LIST * <-- stack *
 static size_t T_(ArraySize)(const struct T_(Array) *const pool) {
 #else * stack --><-- !stack *
 static int T_(ArrayRemove)(struct T_(Array) *const pool, T *const data);
 #endif * !stack --> *
 static void T_(ArrayClear)(struct T_(Array) *const pool);
 static T *T_(ArrayGet)(struct T_(Array) *const pool, const size_t idx);
 static size_t T_(ArrayIndex)(struct T_(Array) *const pool, T *const data);
 static T *T_(ArrayPeek)(const struct T_(Array) *const pool);
 static T *T_(ArrayPop)(struct T_(Array) *const pool);
 static T *T_(ArrayNext)(struct T_(Array) *const pool, T *const prev);
 static T *T_(ArrayNew)(struct T_(Array) *const pool);
 static T *T_(ArrayUpdateNew)(struct T_(Array) *const pool, S **const update_ptr);
 static void T_(ArrayForEach)(struct T_(Array) *const pool,
 const PT_(Action) action);
 static void T_(ArrayMigrateEach)(struct T_(Array) *const pool,
 const PT_(Migrate) handler, const struct Migrate *const migrate);
 static void T_(ArrayMigratePointer)(T **const data_ptr,
 const struct Migrate *const migrate);
 static const char *T_(ArrayToString)(const struct T_(Array) *const pool);
*/



static void PT_(test_basic)(void) {
	struct T_(Array) a;
	S *supertype;
	T ts[5], *t, *t1;
	const size_t ts_size = sizeof ts / sizeof *ts, big = 1000;
	size_t i;

	printf("Test null.\n");
	errno = 0;
	T_(Array_)(0);
#ifdef ARRAY_MIGRATE_ALL /* <-- all */
	T_(Array)(0, 0, 0);
#else /* all --><-- !all */
	T_(Array)(0);
#endif /* all --> */
#ifndef ARRAY_FREE_LIST /* <-- stack */
	assert(T_(ArraySize(0)) == 0);
#else /* stack --><-- !stack */
	assert(T_(ArrayRemove)(0, 0) == 0);
#endif /* !stack --> */
	T_(ArrayClear)(0);
	assert(T_(ArrayGet)(0, 0) == 0 && T_(ArrayGet)(0, 1) == 0);
	assert(T_(ArrayPeek)(0) == 0);
	assert(T_(ArrayPop)(0) == 0);
	assert(T_(ArrayNext)(0, 0) == 0);
	assert(T_(ArrayNew)(0) == 0);
	assert(T_(ArrayUpdateNew)(0, 0) == 0
		&& T_(ArrayUpdateNew)(0, &supertype) == 0);
	T_(ArrayForEach)(0, 0);
	T_(ArrayMigrateEach)(0, 0, 0);
	T_(ArrayMigratePointer)(0, 0);
	assert(!strcmp("null", T_(ArrayToString(0))));
	assert(errno == 0);
	PT_(valid_state)(0);

	printf("Test empty.\n");
#ifdef ARRAY_MIGRATE_ALL /* <-- all */
	T_(Array)(&a, &PT_(migrate), &dummy_parent);
#else /* all --><-- !all */
	T_(Array)(&a);
#endif /* all --> */
	t = (T *)1;
#ifndef ARRAY_FREE_LIST /* <-- stack */
	assert(T_(ArraySize)(&a) == 0);
#else /* stack --><-- !stack */
	assert(T_(ArrayRemove)(&a, 0) == 0 && errno == 0);
	assert(T_(ArrayRemove)(&a, t) == 0 && errno == EDOM), errno = 0;
#endif /* !stack --> */
	assert(T_(ArrayGet)(&a, 0) == 0);
	assert(T_(ArrayPeek)(0) == 0);
	assert(T_(ArrayPop)(0) == 0);
	assert(T_(ArrayNext)(0, 0) == 0 && T_(ArrayNext)(0, t) == 0);
	T_(ArrayForEach)(&a, 0);
	T_(ArrayMigrateEach)(&a, 0, 0);
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
	t1 = T_(ArrayNext)(&a, t);
	assert(t1 == 0);
	assert(T_(ArrayIndex)(&a, t) == 0);
	assert(T_(ArrayPeek)(&a) == t);
	assert(T_(ArrayGet(&a, 0)) == t);
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
#ifndef ARRAY_FREE_LIST /* <-- stack */
	assert(T_(ArraySize)(&a) == ts_size);
#else /* stack --><-- !stack */
	if((t = T_(ArrayGet)(&a, ts_size - 2))
		&& !T_(ArrayRemove)(&a, t)) {
		perror("Error"), assert(0);
		return;
	}
	printf("Now: %s.\n", T_(ArrayToString)(&a));
	assert(!T_(ArrayRemove)(&a, t) && errno == EDOM);
	printf("(Deliberate) error: %s.\n", strerror(errno)), errno = 0;
	if((t = T_(ArrayGet)(&a, ts_size - 3))
		&& !T_(ArrayRemove)(&a, t)) {
		perror("Error"), assert(0);
		return;
	}
	printf("Now: %s.\n", T_(ArrayToString)(&a));
	assert(!T_(ArrayRemove)(&a, t) && errno == EDOM);
	printf("(Deliberate) error: %s.\n", strerror(errno)), errno = 0;
	if((t = T_(ArrayGet)(&a, ts_size - 1)) && !T_(ArrayRemove)(&a, t)) {
		perror("Error"), assert(0);
		return;
	}
	printf("Now: %s.\n", T_(ArrayToString)(&a));
	assert(!T_(ArrayRemove)(&a, t) && errno == EDOM);
	printf("(Deliberate) error: %s.\n", strerror(errno)), errno = 0;
	assert(a.size == ts_size - 3);
	T_(ArrayNew)(&a); /* Cheating. */
	T_(ArrayNew)(&a);
	T_(ArrayNew)(&a);
	assert(a.size == ts_size);
#endif /* !stack --> */
	PT_(valid_state)(&a);

	/* Peek/Pop. */
	t = T_(ArrayPeek)(&a);
	assert(t && !memcmp(t, ts + ts_size - 1, sizeof *t));
	t = T_(ArrayPop)(&a);
	assert(t && !memcmp(t, ts + ts_size - 1, sizeof *t));
	t = T_(ArrayPop)(&a);
	assert(t && !memcmp(t, ts + ts_size - 2, sizeof *t));
	T_(ArrayClear)(&a);

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

static void PT_(test_migrate)(void) {

#if 0 /* <-- 0 */

	struct T_(Array) a, *a1, *a2;
	T ts[5000], *t, *t1;
	const size_t ts_size = sizeof ts / sizeof *ts;
	T ss[5000], *s;
	const size_t ss_size = sizeof ss / sizeof *ss;
	size_t i;
	S *supertype;

	/* Get elements. */
	assert(ts_size == ss_size);
	for(t = ts, t1 = t + ts_size; t < t1; t++) {
		PT_(filler)(t);
		memcpy(&ss[t - ts], t, sizeof *t);
	}
	T_(Array)(&a);

#ifdef ARRAY_MIGRATE_ALL /* <-- all */
	T_(Array)(&a, &PT_(migrate), &dummy_parent);
#else /* all --><-- !all */
	T_(Array)(&a);
#endif /* all --> */

#endif /* 0 --> */

}

static void PT_(test_random)(void) {
	struct T_(Array) a;
	size_t i, size = 0;
	const size_t mult = 1; /* For long tests. */
	/* Random. */
#ifdef ARRAY_MIGRATE_ALL /* <-- all */
	T_(Array)(&a, &PT_(migrate), &dummy_parent);
#else /* all --><-- !all */
	T_(Array)(&a);
#endif /* !all --> */
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
#ifndef ARRAY_FREE_LIST /* <-- stack */
			data = T_(ArrayPeek)(&a);
			assert(data);
			PT_(to_string)(data, &str);
			printf("Popping %s.\n", str);
			assert(data == T_(ArrayPop)(&a));
#else /* stack --><-- !stack */
			size_t idx = rand() / (RAND_MAX + 1.0) * size;
			if(!(data = T_(ArrayGet)(&a, idx))) continue;
			PT_(to_string)(data, &str);
			printf("Removing %s at %lu.\n", str, (unsigned long)idx);
			{
				const int ret = T_(ArrayRemove)(&a, data);
				assert(ret || (perror("Removing"), 0));
			}
#endif /* !stack --> */
			size--;
		}
		printf("%s.\n", T_(ArrayToString)(&a));
		PT_(valid_state)(&a);
	}
	T_(Array_)(&a);
}

/** The list will be tested on stdout. */
static void T_(ArrayTest)(void) {
	printf("Array<" QUOTE(ARRAY_NAME) ">: of type <" QUOTE(ARRAY_TYPE)
		"> was created using: "
#ifndef ARRAY_FREE_LIST
		"ARRAY_FREE_LIST; "
#endif
#ifdef ARRAY_MIGRATE_EACH
		"ARRAY_MIGRATE_EACH<" QUOTE(ARRAY_MIGRATE_EACH) ">; "
#endif
#ifdef ARRAY_MIGRATE_ALL
		"ARRAY_MIGRATE_ALL<" QUOTE(ARRAY_MIGRATE_ALL) ">; "
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
	PT_(test_migrate)();
	PT_(test_random)();
	fprintf(stderr, "Done tests of Array<" T_NAME ">.\n\n");
}

/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_

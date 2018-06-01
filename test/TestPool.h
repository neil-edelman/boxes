/* Intended to be included by {Pool.h} on {POOL_TYPE_FILLER}. */

/* Define macros. */
#ifdef QUOTE
#undef QUOTE
#endif
#ifdef QUOTE_
#undef QUOTE_
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)



/* POOL_TEST must be a function that implements <PT>Action. */
static const PT_(Action) PT_(filler) = (POOL_TEST);



static void PT_(valid_state)(const struct T_(Pool) *const a) {
	/* Null is a valid state. */
	if(!a) return;
	assert(a->size <= a->capacity[0]);
	assert(a->capacity[0] < a->capacity[1] || (a->capacity[0] == a->capacity[1]
		&& (a->capacity[1] == pool_max / sizeof(struct PT_(Node))
		|| a->capacity[0] == 0)));
#ifndef POOL_STACK /* <-- !stack */
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

#ifdef POOL_MIGRATE_ALL /* <-- all */
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
 static void T_(Pool_)(struct T_(Pool) *const pool);
 #ifdef POOL_MIGRATE_ALL * <-- all *
 static void T_(Pool)(struct T_(Pool) *const pool,
 const PT_(MigrateAll) migrate_all, A *const all);
 #else * all --><-- !all *
 static void T_(Pool)(struct T_(Pool) *const pool);
 #endif * all --> *
 #ifdef POOL_STACK * <-- stack *
 static size_t T_(PoolSize)(const struct T_(Pool) *const pool) {
 #else * stack --><-- !stack *
 static int T_(PoolRemove)(struct T_(Pool) *const pool, T *const data);
 #endif * !stack --> *
 static void T_(PoolClear)(struct T_(Pool) *const pool);
 static T *T_(PoolGet)(struct T_(Pool) *const pool, const size_t idx);
 static size_t T_(PoolIndex)(struct T_(Pool) *const pool, T *const data);
 static T *T_(PoolPeek)(const struct T_(Pool) *const pool);
 static T *T_(PoolPop)(struct T_(Pool) *const pool);
 static T *T_(PoolNext)(struct T_(Pool) *const pool, T *const prev);
 static T *T_(PoolNew)(struct T_(Pool) *const pool);
 static T *T_(PoolUpdateNew)(struct T_(Pool) *const pool, S **const update_ptr);
 static void T_(PoolForEach)(struct T_(Pool) *const pool,
 const PT_(Action) action);
 static void T_(PoolMigrateEach)(struct T_(Pool) *const pool,
 const PT_(Migrate) handler, const struct Migrate *const migrate);
 static void T_(PoolMigratePointer)(T **const data_ptr,
 const struct Migrate *const migrate);
 static const char *T_(PoolToString)(const struct T_(Pool) *const pool);
*/



static void PT_(test_basic)(void) {
	struct T_(Pool) a;
	S *supertype;
	T ts[5], *t, *t1;
	const size_t ts_size = sizeof ts / sizeof *ts, big = 1000;
	size_t i;

	printf("Test null.\n");
	errno = 0;
	T_(Pool_)(0);
#ifdef POOL_MIGRATE_ALL /* <-- all */
	T_(Pool)(0, 0, 0);
#else /* all --><-- !all */
	T_(Pool)(0);
#endif /* all --> */
#ifdef POOL_STACK /* <-- stack */
	assert(T_(PoolSize(0)) == 0);
#else /* stack --><-- !stack */
	assert(T_(PoolRemove)(0, 0) == 0);
#endif /* !stack --> */
	T_(PoolClear)(0);
	assert(T_(PoolGet)(0, 0) == 0 && T_(PoolGet)(0, 1) == 0);
	assert(T_(PoolPeek)(0) == 0);
	assert(T_(PoolPop)(0) == 0);
	assert(T_(PoolNext)(0, 0) == 0);
	assert(T_(PoolNew)(0) == 0);
	assert(T_(PoolUpdateNew)(0, 0) == 0
		&& T_(PoolUpdateNew)(0, &supertype) == 0);
	T_(PoolForEach)(0, 0);
	T_(PoolMigrateEach)(0, 0, 0);
	T_(PoolMigratePointer)(0, 0);
	assert(!strcmp("null", T_(PoolToString(0))));
	assert(errno == 0);
	PT_(valid_state)(0);

	printf("Test empty.\n");
#ifdef POOL_MIGRATE_ALL /* <-- all */
	T_(Pool)(&a, &PT_(migrate), &dummy_parent);
#else /* all --><-- !all */
	T_(Pool)(&a);
#endif /* all --> */
	t = (T *)1;
#ifdef POOL_STACK /* <-- stack */
	assert(T_(PoolSize)(&a) == 0);
#else /* stack --><-- !stack */
	assert(T_(PoolRemove)(&a, 0) == 0 && errno == 0);
	assert(T_(PoolRemove)(&a, t) == 0 && errno == EDOM), errno = 0;
#endif /* !stack --> */
	assert(T_(PoolGet)(&a, 0) == 0);
	assert(T_(PoolPeek)(0) == 0);
	assert(T_(PoolPop)(0) == 0);
	assert(T_(PoolNext)(0, 0) == 0 && T_(PoolNext)(0, t) == 0);
	T_(PoolForEach)(&a, 0);
	T_(PoolMigrateEach)(&a, 0, 0);
	assert(errno == 0);
	PT_(valid_state)(&a);

	/* @fixme valgrind is giving me grief if I don't do this? */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	for(t = ts, t1 = t + ts_size; t < t1; t++) PT_(filler)(t);

	printf("Test one element.\n");
	t = T_(PoolNew)(&a); /* Add. */
	assert(t);
	t1 = T_(PoolNext)(&a, 0);
	assert(t == t1);
	t1 = T_(PoolNext)(&a, t);
	assert(t1 == 0);
	assert(T_(PoolIndex)(&a, t) == 0);
	assert(T_(PoolPeek)(&a) == t);
	assert(T_(PoolGet(&a, 0)) == t);
	t1 = T_(PoolPop)(&a); /* Remove. */
	assert(t1 == t);
	assert(T_(PoolPeek)(&a) == 0);
	t = T_(PoolNew)(&a); /* Add. */
	assert(t);
	T_(PoolClear)(&a);
	assert(T_(PoolPeek)(&a) == 0);
	PT_(valid_state)(&a);

	printf("Testing %lu elements.\n", (unsigned long)ts_size);
	for(i = 0; i < ts_size; i++) {
		t = T_(PoolNew)(&a);
		assert(t);
		memcpy(t, ts + i, sizeof *t);
	}
	assert(T_(PoolPeek)(&a));
	printf("Now: %s.\n", T_(PoolToString)(&a));
#ifdef POOL_STACK /* <-- stack */
	assert(T_(PoolSize)(&a) == ts_size);
#else /* stack --><-- !stack */
	if((t = T_(PoolGet)(&a, ts_size - 2))
		&& !T_(PoolRemove)(&a, t)) {
		perror("Error"), assert(0);
		return;
	}
	printf("Now: %s.\n", T_(PoolToString)(&a));
	assert(!T_(PoolRemove)(&a, t) && errno == EDOM);
	printf("(Deliberate) error: %s.\n", strerror(errno)), errno = 0;
	if((t = T_(PoolGet)(&a, ts_size - 3))
		&& !T_(PoolRemove)(&a, t)) {
		perror("Error"), assert(0);
		return;
	}
	printf("Now: %s.\n", T_(PoolToString)(&a));
	assert(!T_(PoolRemove)(&a, t) && errno == EDOM);
	printf("(Deliberate) error: %s.\n", strerror(errno)), errno = 0;
	if((t = T_(PoolGet)(&a, ts_size - 1)) && !T_(PoolRemove)(&a, t)) {
		perror("Error"), assert(0);
		return;
	}
	printf("Now: %s.\n", T_(PoolToString)(&a));
	assert(!T_(PoolRemove)(&a, t) && errno == EDOM);
	printf("(Deliberate) error: %s.\n", strerror(errno)), errno = 0;
	assert(a.size == ts_size - 3);
	T_(PoolNew)(&a); /* Cheating. */
	T_(PoolNew)(&a);
	T_(PoolNew)(&a);
	assert(a.size == ts_size);
#endif /* !stack --> */
	PT_(valid_state)(&a);

	/* Peek/Pop. */
	t = T_(PoolPeek)(&a);
	assert(t && !memcmp(t, ts + ts_size - 1, sizeof *t));
	t = T_(PoolPop)(&a);
	assert(t && !memcmp(t, ts + ts_size - 1, sizeof *t));
	t = T_(PoolPop)(&a);
	assert(t && !memcmp(t, ts + ts_size - 2, sizeof *t));
	T_(PoolClear)(&a);

	/* Big. */
	for(i = 0; i < big; i++) {
		t = T_(PoolNew)(&a);
		assert(t);
		PT_(filler)(t);
	}
	printf("%s.\n", T_(PoolToString)(&a));
	PT_(valid_state)(&a);
	for(i = 0, t = 0; (t = T_(PoolNext)(&a, t)); i++);
	assert(a.size == i);
	PT_(valid_state)(&a);

	printf("Clear:\n");
	T_(PoolClear)(&a);
	printf("%s.\n", T_(PoolToString)(&a));
	assert(T_(PoolPeek)(&a) == 0);
	printf("Destructor:\n");
	T_(Pool_)(&a);
	assert(T_(PoolPeek)(&a) == 0);
	PT_(valid_state)(&a);
}

static void PT_(test_migrate)(void) {

#if 0 /* <-- 0 */

	struct T_(Pool) a, *a1, *a2;
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
	T_(Pool)(&a);

#ifdef POOL_MIGRATE_ALL /* <-- all */
	T_(Pool)(&a, &PT_(migrate), &dummy_parent);
#else /* all --><-- !all */
	T_(Pool)(&a);
#endif /* all --> */

#endif /* 0 --> */

}

static void PT_(test_random)(void) {
	struct T_(Pool) a;
	size_t i, size = 0;
	const size_t mult = 1; /* For long tests. */
	/* Random. */
#ifdef POOL_MIGRATE_ALL /* <-- all */
	T_(Pool)(&a, &PT_(migrate), &dummy_parent);
#else /* all --><-- !all */
	T_(Pool)(&a);
#endif /* !all --> */
	/* This parameter controls how many iterations. */
	i = 1000 * mult;
	while(i--) {
		T *data;
		char str[12];
		double r = rand() / (RAND_MAX + 1.0);
		/* This parameter controls how big the pool wants to be. */
		if(r > size / (100.0 * mult)) {
			if(!(data = T_(PoolNew)(&a))) {
				perror("Error"), assert(0);
				return;
			}
			size++;
			PT_(filler)(data);
			PT_(to_string)(data, &str);
			printf("Created %s.\n", str);
		} else {
#ifdef POOL_STACK /* <-- stack */
			data = T_(PoolPeek)(&a);
			assert(data);
			PT_(to_string)(data, &str);
			printf("Popping %s.\n", str);
			assert(data == T_(PoolPop)(&a));
#else /* stack --><-- !stack */
			size_t idx = rand() / (RAND_MAX + 1.0) * size;
			if(!(data = T_(PoolGet)(&a, idx))) continue;
			PT_(to_string)(data, &str);
			printf("Removing %s at %lu.\n", str, (unsigned long)idx);
			{
				const int ret = T_(PoolRemove)(&a, data);
				assert(ret || (perror("Removing"), 0));
			}
#endif /* !stack --> */
			size--;
		}
		printf("%s.\n", T_(PoolToString)(&a));
		PT_(valid_state)(&a);
	}
	T_(Pool_)(&a);
}

/** The list will be tested on stdout. */
static void T_(PoolTest)(void) {
	printf("Pool<" QUOTE(POOL_NAME) ">: of type <" QUOTE(POOL_TYPE)
		"> was created using: "
#ifdef POOL_STACK
		"POOL_STACK; "
#endif
#ifdef POOL_MIGRATE_EACH
		"POOL_MIGRATE_EACH<" QUOTE(POOL_MIGRATE_EACH) ">; "
#endif
#ifdef POOL_MIGRATE_ALL
		"POOL_MIGRATE_ALL<" QUOTE(POOL_MIGRATE_ALL) ">; "
#endif
#ifdef POOL_MIGRATE_UPDATE
		"POOL_MIGRATE_UPDATE<" QUOTE(POOL_MIGRATE_UPDATE) ">; "
#endif		   
#ifdef POOL_TO_STRING
		"POOL_TO_STRING<" QUOTE(POOL_TO_STRING) ">; "
#endif
#ifdef POOL_TEST
		"POOL_TEST<" QUOTE(POOL_TEST) ">; "
#endif
		"testing:\n");
	PT_(test_basic)();
	PT_(test_migrate)();
	PT_(test_random)();
	fprintf(stderr, "Done tests of Pool<" T_NAME ">.\n\n");
}

/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_

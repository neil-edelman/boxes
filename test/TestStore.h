/* intended to be included by Store.h on STORE_TYPE_FILLER */

/* prototype */
static void T_(StoreTest)(void);
static void PRIVATE_T_(test_basic)(void);

/* STORE_TEST must be a function that implements <T>Action. */
static const T_(Action) PRIVATE_T_(filler) = (STORE_TEST);



static void PRIVATE_T_(valid_state)(const struct T_(Store) *const a) {
	struct PRIVATE_T_(Element) *elem;
	size_t i, remove_start = 0, remove_end = 0, remove_both = 0,remove_data = 0;
	size_t r0, r1;
	enum { SDATA, SNULL, SNOT } r0_class, r1_class;
	if(!a) return; /* null is valid */
	assert(a->size <= a->capacity[0]);
	assert(a->capacity[0] < a->capacity[1] || (a->capacity[0] == a->capacity[1]
		&& a->capacity[1]
		== (store_null - 1) / sizeof(struct PRIVATE_T_(Element))));
	assert((a->head == store_null) == (a->tail == store_null));
	for(i = 0; i < a->size; i++) {
		elem = a->array + i;
		r0 = elem->prev;
		r1 = elem->next;
		/* five states: info:info, null:info, info:null, null:null, and not
		 part of the removed queue; check for invalid states */
		if(r0 == store_null) r0_class = SNULL;
		else if(r0 == store_not_part) r0_class = SNOT;
		else r0_class = SDATA;
		if(r1 == store_null) r1_class = SNULL;
		else if(r1 == store_not_part) r1_class = SNOT;
		else r1_class = SDATA;
		/* count */
		if(r0_class == SNOT && r1_class == SNOT);
		else if(r0_class == SNULL && r1_class == SNULL) remove_both++;
		else if(r0_class == SNULL && r1_class == SDATA) remove_start++;
		else if(r0_class == SDATA && r1_class == SDATA) remove_data++;
		else if(r0_class == SDATA && r1_class == SNULL) remove_end++;
		else assert((printf("Invalid state.\n"), 0));
	}
	assert((remove_both == 0 && remove_start == 0 && remove_end == 0)
		|| (a->size >= 2 && remove_both == 1 && remove_start == 0
		&& remove_end == 0)
		|| (a->size > 2 && remove_both == 0 && remove_start == 1
		&& remove_end == 1));
	assert(a->size == 0 || a->array[a->size - 1].prev == store_not_part);
}

/** @implements Migrate */
static void PRIVATE_T_(migrate)(const void *parent,
	const struct Migrate *const info) {
	assert(parent && info);
	printf("#%p migrate #%p-%p -> %p\n", parent, info->begin, info->end,
		(void *)info->delta);
	/* fixme: check */
}

static void PRIVATE_T_(test_basic)(void) {
	struct T_(Store) *a = 0;
	T test[5], *testp;
	const size_t test_size = sizeof test / sizeof *test;
	size_t i;
	const char *err;
	enum { CREATE, DESTROY };

	for(i = 0; i < test_size; i++) PRIVATE_T_(filler)(test + i);
	printf("Constructor:\n");
	assert(T_(StoreIsEmpty)(a));
	a = T_(Store)(&PRIVATE_T_(migrate), (void *)1 /* stub */);
	err = T_(StoreGetError)(a);
	printf("%s: %s.\n", T_(StoreToString)(a), err);
	assert(a);
	assert(!strcmp("no error", err));
	assert(T_(StoreIsEmpty)(a));

	printf("Adding %lu elements:\n", (unsigned long)test_size);
	for(i = 0; i < test_size; i++) {
		testp = T_(StoreNew)(a);
		assert(testp);
		memcpy(testp, test + i, sizeof *test);
	}
	assert(!T_(StoreIsEmpty)(a));
	printf("Now: %s.\n", T_(StoreToString)(a));

	printf("Remove last:\n");
	assert(((testp = T_(StoreGetElement)(a, test_size - 2))
		&& T_(StoreRemove)(a, testp))
		|| (printf("Error: %s.\n", T_(StoreGetError(a))), 0));
	printf("Now: %s.\n", T_(StoreToString)(a));
	assert(!T_(StoreRemove)(a, testp));
	printf("(Deliberate) error: %s.\n", T_(StoreGetError)(a));
	assert(((testp = T_(StoreGetElement)(a, test_size - 3))
		&& T_(StoreRemove)(a, testp))
		|| (printf("Error: %s.\n", T_(StoreGetError(a))), 0));
	printf("Now: %s.\n", T_(StoreToString)(a));
	assert(!T_(StoreRemove)(a, testp));
	printf("(Deliberate) error: %s.\n", T_(StoreGetError)(a));
	assert(((testp = T_(StoreGetElement)(a, test_size - 1))
		&& T_(StoreRemove)(a, testp))
		|| (printf("Error: %s.\n", T_(StoreGetError(a))), 0));
	printf("Now: %s.\n", T_(StoreToString)(a));
	assert(!T_(StoreRemove)(a, testp));
	printf("(Deliberate) error: %s.\n", T_(StoreGetError)(a));
	printf("Store reserve.\n");
	T_(StoreReserve)(a, (size_t)100);
	assert(a->capacity[0] >= 100);
	for(i = 0; i < 100; i++) {
		testp = T_(StoreNew)(a);
		assert(testp);
		PRIVATE_T_(filler)(testp);
	}
	printf("%s.\n", T_(StoreToString)(a));
	printf("Clear:\n");
	T_(StoreClear)(a);
	printf("%s.\n", T_(StoreToString)(a));
	assert(a->size == 0);

	printf("Destructor:\n");
	T_(Store_)(&a);
	assert(!a);
}

static void PRIVATE_T_(test_random)(void) {
	struct T_(Store) *a;
	size_t i;
	/* random */
	a = T_(Store)(&PRIVATE_T_(migrate), a);
	/* this parameter controls how many iterations */
	i = 1000;
	while(i--) {
		T *node;
		char str[12];
		double r = rand() / (RAND_MAX + 1.0);
		size_t size = a->size;
		/* this parameter controls how big the store wants to be */
		if(r > size / 10.0) {
			assert((node = T_(StoreNew)(a))
				   || (printf("Error: %s.\n", T_(StoreGetError)(a)), 0));
			PRIVATE_T_(filler)(node);
			PRIVATE_T_(to_string)(node, &str);
			printf("Created %s.\n", str);
		} else {
			size_t idx = rand() / (RAND_MAX + 1.0) * size;
			if(!T_(StoreIsElement)(a, idx)) continue;
			assert((node = T_(StoreGetElement)(a, idx))
				   || (printf("Error getting: %s.\n", T_(StoreGetError)(a)), 0));
			PRIVATE_T_(to_string)(node, &str);
			printf("Removing %s at %lu.\n", str, (unsigned long)idx);
			assert(T_(StoreRemove)(a, node)
				   || (printf("Error removing: %s.\n", T_(StoreGetError)(a)), 0));
		}
		printf("%s.\n", T_(StoreToString)(a));
		PRIVATE_T_(valid_state)(a);
	}
}

/** The list will be tested on stdout. */
static void T_(StoreTest)(void) {
	printf("Store<" T_NAME ">: of type <" QUOTE(STORE_TYPE)
		"> was created using: "
#ifdef STORE_TO_STRING
		"TYPE_TO_STRING<" QUOTE(STORE_TO_STRING) ">; "
#endif
#ifdef STORE_TEST
		"STORE_TEST<" QUOTE(STORE_TEST) ">; "
#endif
#ifdef STORE_DEBUG
		"DEBUG; "
#endif
		"testing:\n");
	PRIVATE_T_(test_basic)();
	PRIVATE_T_(test_random)();
	fprintf(stderr, "Done tests of Store<" T_NAME ">.\n\n");
}

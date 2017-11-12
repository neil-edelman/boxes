/* intended to be included by Stack.h on STACK_TYPE_FILLER */

/* prototype */
static void T_(StackTest)(void);
static void PRIVATE_T_(test_basic)(void);

/* STACK_TEST must be a function that implements <T>Action. */
static const T_(Action) PRIVATE_T_(filler) = (STACK_TEST);



static void PRIVATE_T_(valid_state)(const struct T_(Stack) *const a) {
	if(!a) return; /* null is valid */
	assert(a->size <= a->capacity[0]);
	assert(a->capacity[0] < a->capacity[1] || (a->capacity[0] == a->capacity[1]
		&& a->capacity[1] == (size_t)(-1) / sizeof(T)));
}

static void PRIVATE_T_(test_basic)(void) {
	struct T_(Stack) *a = 0;
	T test[5], *testp;
	const size_t test_size = sizeof test / sizeof *test;
	size_t i;
	const char *err;
	enum { CREATE, DESTROY };

	for(i = 0; i < test_size; i++) PRIVATE_T_(filler)(test + i);
	printf("Constructor:\n");
	assert(!T_(StackPop)(a));
	assert(!T_(StackPeek)(a));
	assert(!T_(StackGetElement)(a, 0));
	a = T_(Stack)();
	err = T_(StackGetError)(a);
	printf("%s: %s.\n", T_(StackToString)(a), err);
	assert(a);
	assert(!strcmp("no error", err));
	assert(!T_(StackPop)(a));
	assert(!T_(StackPeek)(a));
	assert(!T_(StackGetElement)(a, 0));
	printf("(Deliberate) error: %s.\n", T_(StackGetError)(a));

	printf("Adding %lu elements:\n", (unsigned long)test_size);
	for(i = 0; i < test_size; i++) {
		testp = T_(StackNew)(a);
		assert(testp);
		memcpy(testp, test + i, sizeof *test);
	}

	printf("Remove last:\n");
	if(!(testp = T_(StackPop)(a))) {
		printf("Error: %s.\n", T_(StackGetError(a))), assert(0);
		return;
	}
	printf("Now: %s.\n", T_(StackToString)(a));
	if(!(testp = T_(StackPop)(a))) {
		printf("Error: %s.\n", T_(StackGetError(a))), assert(0);
		return;
	}
	printf("Now: %s.\n", T_(StackToString)(a));
	printf("Stack reserve.\n");
	T_(StackReserve)(a, (size_t)100);
	assert(a->capacity[0] >= 100);
	for(i = 0; i < 100; i++) {
		testp = T_(StackNew)(a);
		assert(testp);
		PRIVATE_T_(filler)(testp);
	}
	printf("%s.\n", T_(StackToString)(a));
	printf("Clear:\n");
	T_(StackClear)(a);
	printf("%s.\n", T_(StackToString)(a));
	assert(a->size == 0);

	printf("Destructor:\n");
	T_(Stack_)(&a);
	assert(!a);
}

static void PRIVATE_T_(test_random)(void) {
	struct T_(Stack) *a;
	size_t i;
	/* random */
	a = T_(Stack)();
	/* this parameter controls how many iterations */
	i = 1000;
	while(i--) {
		T *node;
		char str[12];
		double r = rand() / (RAND_MAX + 1.0);
		size_t size = a->size;
		/* this parameter controls how big the pool wants to be */
		if(r > size / 100.0) {
			if(!(node = T_(StackNew)(a))) {
				printf("Error: %s.\n", T_(StackGetError)(a)), assert(0);
				return;
			}
			PRIVATE_T_(filler)(node);
			PRIVATE_T_(to_string)(node, &str);
			printf("Created %s.\n", str);
		} else {
			if(!T_(StackPop)(a)) continue;
			PRIVATE_T_(to_string)(node, &str);
			printf("Removing %s.\n", str);
		}
		printf("%s.\n", T_(StackToString)(a));
		PRIVATE_T_(valid_state)(a);
	}
}

/** The list will be tested on stdout. */
static void T_(StackTest)(void) {
	printf("Stack<" T_NAME ">: of type <" QUOTE(STACK_TYPE)
		"> was created using: "
#ifdef STACK_TO_STRING
		"TYPE_TO_STRING<" QUOTE(STACK_TO_STRING) ">; "
#endif
#ifdef STACK_TEST
		"STACK_TEST<" QUOTE(STACK_TEST) ">; "
#endif
#ifdef STACK_DEBUG
		"DEBUG; "
#endif
		"testing:\n");
	PRIVATE_T_(test_basic)();
	PRIVATE_T_(test_random)();
	fprintf(stderr, "Done tests of Stack<" T_NAME ">.\n\n");
}

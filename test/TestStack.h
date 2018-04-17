/* intended to be included by Stack.h on STACK_TYPE_FILLER */

/* Define macros. */
#ifdef QUOTE
#undef QUOTE
#endif
#ifdef QUOTE_
#undef QUOTE_
#endif
#ifdef T_NAME
#undef T_NAME
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)
#define T_NAME QUOTE(STACK_NAME)



/* prototype */
static void T_(StackTest)(void);
static void PT_(test_basic)(void);

/* STACK_TEST must be a function that implements <T>Action. */
static const PT_(Action) PT_(filler) = (STACK_TEST);



static void PT_(valid_state)(const struct T_(Stack) *const a) {
	if(!a) return; /* null is valid */
	assert(a->size <= a->capacity[0]);
	assert(a->capacity[0] < a->capacity[1] || (a->capacity[0] == a->capacity[1]
		&& a->capacity[1] == (size_t)(-1) / sizeof(T)));
}

static void PT_(test_basic)(void) {
	struct T_(Stack) a;
	T test[5], *testp;
	const size_t test_size = sizeof test / sizeof *test;
	size_t i;
	enum { CREATE, DESTROY };

	errno = 0;
	for(i = 0; i < test_size; i++) PT_(filler)(test + i);
	printf("Constructor:\n");
	T_(Stack)(&a);
	assert(!T_(StackPop)(&a));
	assert(!T_(StackPeek)(&a));
	assert(!T_(StackGet)(&a, 0));
	printf("(Deliberate) error: %s.\n", strerror(errno));
	assert(errno == EDOM);
	errno = 0;
	printf("Stack: %s.\n", T_(StackToString)(&a));
	/*assert(a);*/
	assert(!errno);
	assert(!T_(StackPop)(&a));
	assert(!T_(StackPeek)(&a));
	assert(!T_(StackGet)(&a, 0));
	printf("(Deliberate) error: %s.\n", strerror(errno));
	assert(errno == EDOM);
	errno = 0;

	printf("Adding %lu elements:\n", (unsigned long)test_size);
	for(i = 0; i < test_size; i++) {
		testp = T_(StackNew)(&a);
		assert(testp);
		memcpy(testp, test + i, sizeof *test);
	}

	printf("Iterating.\n");
	testp = 0, i = 0;
	while((testp = T_(StackNext)(&a, testp))) i++;
	assert(i == T_(StackSize)(&a) && i == test_size);

	printf("Remove last:\n");
	if(!(testp = T_(StackPop)(&a))) {
		perror("Error"), assert(0);
		return;
	}
	printf("Now: %s.\n", T_(StackToString)(&a));
	if(!(testp = T_(StackPop)(&a))) {
		perror("Error"), assert(0);
		return;
	}
	printf("Now: %s.\n", T_(StackToString)(&a));
	printf("Stack reserve.\n");
	T_(StackReserve)(&a, (size_t)100);
	assert(a.capacity[0] >= 100);
	for(i = 0; i < 100; i++) {
		testp = T_(StackNew)(&a);
		assert(testp);
		PT_(filler)(testp);
	}
	printf("%s.\n", T_(StackToString)(&a));
	printf("Clear:\n");
	T_(StackClear)(&a);
	printf("%s.\n", T_(StackToString)(&a));
	assert(a.size == 0);

	printf("Destructor:\n");
	T_(Stack_)(&a);
	assert(a.size == 0);
}

static void PT_(test_random)(void) {
	struct T_(Stack) a;
	size_t i;
	/* random */
	T_(Stack)(&a);
	/* this parameter controls how many iterations */
	i = 1000;
	while(i--) {
		T *node;
		char str[12];
		double r = rand() / (RAND_MAX + 1.0);
		size_t size = a.size;
		/* this parameter controls how big the pool wants to be */
		if(r > size / 100.0) {
			if(!(node = T_(StackNew)(&a))) {
				perror("Error"), assert(0);
				return;
			}
			PT_(filler)(node);
			PT_(to_string)(node, &str);
			printf("Created %s.\n", str);
		} else {
			if(!T_(StackPop)(&a)) continue;
			PT_(to_string)(node, &str);
			printf("Removing %s.\n", str);
		}
		printf("%s.\n", T_(StackToString)(&a));
		PT_(valid_state)(&a);
	}
}

/** The list will be tested on stdout. */
static void T_(StackTest)(void) {
	printf("Stack<" T_NAME ">: of type <" QUOTE(STACK_TYPE)
		"> was created using: "
#ifdef POOL_MIGRATE
		"POOL_MIGRATE<" QUOTE(POOL_MIGRATE) ">; "
#endif
#ifdef STACK_TO_STRING
		"STACK_TO_STRING<" QUOTE(STACK_TO_STRING) ">; "
#endif
#ifdef STACK_TEST
		"STACK_TEST<" QUOTE(STACK_TEST) ">; "
#endif
		"testing:\n");
	PT_(test_basic)();
	PT_(test_random)();
	fprintf(stderr, "Done tests of Stack<" T_NAME ">.\n\n");
}

/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_
#undef T_NAME

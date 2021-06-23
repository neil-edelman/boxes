/* Intended to be included by `Pool.h` on `POOL_TEST`. */

#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

#ifdef POOL_TO_STRING /* <!-- to string: Only one, tests all base code. */

/** Operates by side-effects. */
typedef void (*PX_(action_fn))(PX_(type) *);

/* Copy functions for later includes. */
static void (*PX_(to_string))(const PX_(type) *, char (*)[12])
	= (POOL_TO_STRING);
static const char *(*PX_(pool_to_string))(const struct X_(pool) *)
	= Z_(to_string);

/* POOL_TEST must be a function that implements <typedef:<PX>Action>. */
static const PX_(action_fn) PX_(filler) = (POOL_TEST);

/** Tries to graphs `p` in `fn`. */
static void PX_(graph)(const struct X_(pool) *const pool,
	const char *const fn) {
	/* "lightgray" "firebrick" "lightsteelblue"
	 "darkseagreen" "darkseagreen4" */
	FILE *fp;
	char str[12];
	assert(pool && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	printf("*** %s\n", fn);
	fprintf(fp, "digraph {\n"
		"\trankdir=LR;\n"
		"\tnode [shape = record, style = filled, fillcolor = lightsteelblue];\n"
		"\tpool [label=\"\\<" QUOTE(POOL_NAME) "\\>pool\\l|"
		"%s slots %lu/%lu\\l"
		"%s free[0] %lu/%lu\\l",
		pool->slots.data ? "active" : "idle",
		(unsigned long)pool->slots.size,
		(unsigned long)pool->slots.capacity,
		pool->free0.a.data ? "active" : "idle",
		(unsigned long)pool->free0.a.size,
		(unsigned long)pool->free0.a.capacity);
	if(!pool->slots.size)
		fprintf(fp, "idle slots.data[0].capacity: %lu\\l",
		(unsigned long)pool->capacity0);
	fprintf(fp, "\", fillcolor=lightgray];\n");
	if(pool->slots.data) {
		size_t i, j;
		struct pool_chunk *chunk;
		PX_(type) *data;
		/* Slots are in one array. */
		if(!pool->slots.size) {
			fprintf(fp, "\tslot0 [label = \"no slots\", shape = record]\n");
		} else {
			fprintf(fp, "\tsubgraph cluster_slots {\n"
				"\t\tstyle=filled;\n"
				"\t\tlabel=\"slots %lu/%lu\";\n",
				pool->slots.size, pool->slots.capacity);
			for(i = 0; i < pool->slots.size; i++)
				fprintf(fp, "\t\tslot%lu [label=\"[%lu] #%p\"];\n",
				(unsigned long)i, (unsigned long)i, (void *)pool->slots.data[i]);
			fprintf(fp, "\t}\n");
		}
		fprintf(fp, "\tpool -> slot0;\n");
		/* For each slot, there is a chunk array. */
		for(i = 0; i < pool->slots.size; i++) {
			chunk = pool->slots.data[i];
			data = PX_(data)(chunk);
			fprintf(fp, "\tsubgraph cluster_chunk%lu {\n"
				"\t\tstyle=filled;\n"
				"\t\tlabel=\"chunk size %lu",
				(unsigned long)i, (unsigned long)chunk->size);
			if(!i) fprintf(fp, "/%lu", (unsigned long)pool->capacity0);
			fprintf(fp, "\";\n");
			if(i || !chunk->size) {
				fprintf(fp, "\t\tdata%lu_0 [style=invis]\n", i);
			} else {
				for(j = 0; j < chunk->size; j++) {
					size_t *f, *f_end;
					for(f = pool->free0.a.data, f_end = f + pool->free0.a.size;
						f < f_end && *f != j; f++);
					if(f == f_end) {
						PX_(to_string)(data + j, &str);
						fprintf(fp, "\t\tdata%lu_%lu [label=\"[%lu] %s\"];\n",
							(unsigned long)i, (unsigned long)j, (unsigned long)j,
							str);
					} else {
						fprintf(fp, "\t\tdata%lu_%lu [label=\"removed\","
							" fillcolor=red, style=invis];\n",
							(unsigned long)i, (unsigned long)j);
					}
				}
			}
			fprintf(fp, "\t}\n"
				"\tslot%lu -> data%lu_0;\n",
				(unsigned long)i, (unsigned long)i);
		}
	}
	fprintf(fp, "\tnode [fillcolour=red];\n"
		"}\n");
	fclose(fp);
}

/** Crashes if `pool` is not in a valid state. */
static void PX_(valid_state)(const struct X_(pool) *const pool) {
	size_t i;
	if(!pool) return;
	/* If there's no capacity, there's no slots. */
	if(!pool->capacity0) assert(!pool->slots.size);
	/* Every slot up to size is active. */
	for(i = 0; i < pool->slots.size; i++) assert(pool->slots.data[i]);
	if(!pool->slots.size) {
		/* There are no free0 without slots. */
		assert(!pool->free0.a.size);
	} else {
		/* size[0] <= capacity0 */
		assert(pool->slots.data[0]->size <= pool->capacity0);
		/* The free-heap indices are strictly less than the size. */
		for(i = 0; i < pool->free0.a.size; i++)
			assert(pool->free0.a.data[i] < pool->slots.data[0]->size);
	}
	/*...?*/
}

static void PX_(test_states)(void) {
	struct X_(pool) pool = POOL_IDLE;
	PX_(type) /*ts[5],*/ *t/*, *t1*/;
	/*const size_t ts_size = sizeof ts / sizeof *ts, big = 1000;*/
	size_t i;
	int r;

	printf("Test null.\n");
	errno = 0;
	PX_(valid_state)(0);

	printf("Empty.\n");
	PX_(valid_state)(&pool);
	X_(pool)(&pool);
	PX_(valid_state)(&pool);
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-01-idle.gv");

	printf("One element.\n");
	t = X_(pool_new)(&pool), assert(t), PX_(filler)(t), PX_(valid_state)(&pool);
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-02-one.gv");

	printf("Remove.\n");
	r = X_(pool_remove)(&pool, PX_(data)(pool.slots.data[0])), assert(r),
		PX_(valid_state)(&pool);
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-03-remove.gv");

	for(i = 0; i < 8; i++) t = X_(pool_new)(&pool), assert(t), PX_(filler)(t),
		PX_(valid_state)(&pool);
	assert(pool.slots.size == 1);
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-04-one-chunk.gv");
	for(i = 0; i < 13; i++) t = X_(pool_new)(&pool), assert(t), PX_(filler)(t),
		PX_(valid_state)(&pool);
	assert(pool.slots.size == 2);
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-05-two-chunks.gv");
	t = X_(pool_new)(&pool), assert(t), PX_(filler)(t), PX_(valid_state)(&pool);
	assert(pool.slots.size == 3);
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-06-three-chunks.gv");
	if(pool.slots.data[1]->size == 8) i = 0;
	else assert(pool.slots.data[1]->size == 13);
	if(pool.slots.data[2]->size == 8) i = 1;
	else assert(pool.slots.data[2]->size == 13);
	X_(pool_remove)(&pool, PX_(data)(pool.slots.data[!i + 1]));
	X_(pool_remove)(&pool, PX_(data)(pool.slots.data[i + 1]));
	printf("remove a zero-slot:\n");
	X_(pool_remove)(&pool, PX_(data)(pool.slots.data[0]));
	printf("end remove\n");
	X_(pool_remove)(&pool, PX_(data)(pool.slots.data[i + 1]));
	X_(pool_remove)(&pool, PX_(data)(pool.slots.data[i + 1]));
	X_(pool_remove)(&pool, PX_(data)(pool.slots.data[i + 1]));
	X_(pool_remove)(&pool, PX_(data)(pool.slots.data[i + 1]));
	X_(pool_remove)(&pool, PX_(data)(pool.slots.data[i + 1]));
	X_(pool_remove)(&pool, PX_(data)(pool.slots.data[i + 1]));
	X_(pool_remove)(&pool, PX_(data)(pool.slots.data[i + 1]));
	PX_(valid_state)(&pool);
	assert(pool.slots.size == 2);
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-07-remove-chunk.gv");
	X_(pool_clear)(&pool);
	assert(pool.slots.size == 1);
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-08-clear.gv");

	for(i = 0; i < 20; i++) t = X_(pool_new)(&pool), assert(t), PX_(filler)(t),
		PX_(valid_state)(&pool);
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-09-remove.gv");
	assert(pool.slots.size == 1 && pool.slots.data[0]->size == 20
		&& pool.capacity0 == 20);
#if 0
	if(!X_(pool_remove)(&a, t)) { perror("Error"), assert(0); return; }
	PX_(valid_state)(&a);
	t = X_(pool_new)(&a), PX_(filler)(t); /* Add. */
	assert(t);
	PX_(valid_state)(&a);
	X_(pool_clear)(&a);
	PX_(valid_state)(&a);
	PX_(graph)(&a, "graph/" QUOTE(POOL_NAME) "-zero.gv");
	X_(pool_)(&a);

	/* @fixme valgrind is giving me grief if I don't do this? */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	for(t = ts, t1 = t + ts_size; t < t1; t++) PX_(filler)(t);

	printf("Testing %lu elements.\n", (unsigned long)ts_size);
	for(t1 = 0, i = 0; i < ts_size; i++) {
		t = X_(pool_new)(&a);
		if(!t1) t1 = t;
		assert(t);
		memcpy(t, ts + i, sizeof *t);
	}
	printf("Now: %s.\n", PX_(pool_to_string)(&a));
	if(!X_(pool_remove)(&a, t1)) { perror("Error"), assert(0); return; }
	printf("Now: %s.\n", PX_(pool_to_string)(&a));
	assert(!X_(pool_remove)(&a, t1) && errno == EDOM);
	printf("(Deliberate) error: %s.\n", strerror(errno)), errno = 0;
	PX_(valid_state)(&a);
	PX_(graph)(&a, "graph/" QUOTE(POOL_NAME) "-small.gv");
	t = X_(pool_new)(&a), PX_(filler)(t); /* Cheating. */
	t = X_(pool_new)(&a), PX_(filler)(t);
	t = X_(pool_new)(&a), PX_(filler)(t);
	t = X_(pool_new)(&a), PX_(filler)(t);
	t = X_(pool_new)(&a), PX_(filler)(t);
	assert(X_(pool_remove)(&a, t));
	X_(pool_for_each)(&a, &PX_(print));
	PX_(valid_state)(&a);
	assert(!X_(pool_reserve)(&a, 1000) && errno == EDOM), errno = 0;
	PX_(graph)(&a, "graph/" QUOTE(POOL_NAME) "-small-1000.gv");
	PX_(valid_state)(&a);
	X_(pool_clear)(&a);
	PX_(valid_state)(&a);

	/* Big. */
	for(i = 0; i < big; i++) {
		t = X_(pool_new)(&a);
		assert(t);
		PX_(filler)(t);
	}
	printf("%s.\n", PX_(pool_to_string)(&a));
	PX_(valid_state)(&a);

	printf("Clear:\n");
	X_(pool_clear)(&a);
	printf("Now: %s.\n", PX_(pool_to_string)(&a));
#endif
	printf("Destructor:\n");
	X_(pool_)(&pool);
	PX_(valid_state)(&pool);
	printf("Done basic tests.\n\n");
}

#if 0
static void PX_(test_random)(void) {
	struct X_(pool) a = POOL_IDLE;
	size_t i, size = 0;
	const size_t length = 120000; /* Controls how many iterations. */
	char graph_fn[64];
	const size_t graph_max = 100000;
	for(i = 0; i < length; i++) {
		char str[12];
		double r = rand() / (RAND_MAX + 1.0);
		int is_print = !(i & (i - 1));
		/* This parameter controls how big the pool wants to be. */
		if(r > size / 5000.0) {
			PX_(type) *data = X_(pool_new)(&a);
			if(!data) { perror("Error"), assert(0); return;}
			size++;
			PX_(filler)(data);
			PX_(to_string)(data, &str);
			if(is_print) printf("%lu: Created %s.\n", (unsigned long)i, str);
		} else {
			struct PX_(block) *block;
			struct PX_(node) *node = 0, *end;
			size_t idx = rand() / (RAND_MAX + 1.0) * size;
			assert(a.largest);
			/* Pick random. */
			for(block = a.largest; block; block = block->smaller) {
				for(node = PX_(block_nodes)(block), end = node
					+ (block == a.largest ? block->size : block->capacity);
					node < end; node++) {
					if(node->x.prev) continue;
					if(!idx) break;
					idx--;
				}
				if(node < end) break;
			}
			assert(block);
			PX_(to_string)(&node->data, &str);
			if(is_print) printf("%lu: Removing %s in block %p.\n",
				(unsigned long)i, str, (const void *)block);
			{
				const int ret = X_(pool_remove)(&a, &node->data);
				assert(ret || (perror("Removing"),
					PX_(graph)(&a, "graph/" QUOTE(POOL_NAME) "-rem-err.gv"),0));
			}
			size--;
		}
		if(is_print && i < graph_max) {
			sprintf(graph_fn, "graph/" QUOTE(POOL_NAME) "-%u.gv", (unsigned)i);
			PX_(graph)(&a, graph_fn);
			printf("%s.\n", PX_(pool_to_string)(&a));
		}
		PX_(valid_state)(&a);
	}
	if(i < graph_max) {
		sprintf(graph_fn, "graph/" QUOTE(POOL_NAME) "-%u-end.gv", (unsigned)i);
		PX_(graph)(&a, graph_fn);
	}
	X_(pool_)(&a);
}
#endif

/** The list will be tested on stdout; requires `POOL_TEST` and not `NDEBUG`.
 @allow */
static void X_(pool_test)(void) {
	printf("<" QUOTE(POOL_NAME) ">pool: of type <" QUOTE(POOL_TYPE)
		"> was created using: "
#ifdef POOL_TO_STRING
		"POOL_TO_STRING<" QUOTE(POOL_TO_STRING) ">; "
#endif
#ifdef POOL_TEST
		"POOL_TEST<" QUOTE(POOL_TEST) ">; "
#endif
		"testing:\n");
	PX_(test_states)();
	/*PX_(test_random)();*/
	fprintf(stderr, "Done tests of <" QUOTE(POOL_NAME) ">pool.\n\n");
}

#else /* to string --><!-- */
#error Test unsupported option; testing is out-of-sync?
#endif /* --> */

#undef QUOTE
#undef QUOTE_

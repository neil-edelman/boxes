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
	FILE *fp;
	char str[12];
	assert(pool && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\trankdir=LR;\n"
		"\tnode [shape = record, style = filled, fillcolor = lightgray];\n"
		/*"\tedge [color=royalblue];\n"*/
		"\tpool [label=\"\\<" QUOTE(POOL_NAME)
		"\\>pool\\l|capacity0: %lu\\lfree0.size %lu\\l"
		"free0.capacity %lu\\l\"];\n",
		(unsigned long)pool->capacity0,
		(unsigned long)pool->free0.a.size,
		(unsigned long)pool->free0.a.capacity);
	if(pool->slots.data) {
		size_t i, j;
		struct pool_chunk *chunk;
		PX_(type) *data;
		/* Slots are in one array. */
		if(!pool->slots.size) {
			fprintf(fp, "\tslots [label = \"no slots\", shape = record]\n");
		} else {
			fprintf(fp, "\tnode [fillcolor=lightsteelblue];\n"
				"\tsubgraph cluster_slots {\n"
				"\t\tstyle=filled;\n"
				"\t\tlabel=\"size %lu\\lcapacity %lu\";\n",
				pool->slots.size, pool->slots.capacity);
			for(i = 0; i < pool->slots.size; i++)
				fprintf(fp, "\t\tslot%lu;\n", (unsigned long)i);
			fprintf(fp, "\t}\n");
		}
		fprintf(fp, "\tpool -> slot0;\n");
		/* For each slot, there is a chunk array; `chunk[0]` is special. */
		if(pool->slots.size) {
			chunk = pool->slots.data[0];
			data = PX_(datum)(chunk);
			fprintf(fp, "\tsubgraph cluster_chunk%lu {\n"
				"\t\tstyle=filled;\n"
				"\t\tlabel=\"size %lu\\lcapacity %lu\";\n",
				(unsigned long)0, (unsigned long)chunk->size,
				(unsigned long)pool->capacity0);
			for(j = 0; j < chunk->size; j++) {
				size_t *f, *f_end;
				for(f = pool->free0.a.data, f_end = f + pool->free0.a.size;
					f < f_end && *f != j; f++);
				if(f == f_end) {
					PX_(to_string)(data + j, &str);
					fprintf(fp, "\t\tdata%lu_%lu [label=\"%s\"];\n",
						(unsigned long)0, (unsigned long)j, str);
				} else {
					fprintf(fp, "\t\tdata%lu_%lu [color=red];\n",
						(unsigned long)0, (unsigned long)j);
				}
			}
			fprintf(fp, "\t}\n"
				"\tslot%lu -> data%lu_0;\n",
				(unsigned long)0, (unsigned long)0);
		}
		for(i = 1; i < pool->slots.size; i++) {
			assert(0);
		}
	}

#if 0
	for(slot = pool->slots.data, slot_end = slot + pool->slots.size;
		slot < slot_end; slot++) {
		sprintf(b_strs[b = !b], "block%p", (const void *)block);
		/* This is cleaver but I don't know what I did. Hack. */
		if(block == p->largest) {
			fprintf(fp, "\tnode%p",
			(const void *)PX_(x_const_upcast)(&p->removed));
		} else {
			fprintf(fp, "\tdud_%s", b_strs[!b]);
		}
		fprintf(fp,
			" -> dud_%s [ltail=cluster_%s, lhead=cluster_%s];\n",
			b_strs[b], b_strs[!b], b_strs[b]);
		fprintf(fp, "\tsubgraph cluster_%s {\n"
			"\t\tstyle=filled;\n"
			"\t\tfillcolor=lightgray;\n"
			"\t\tlabel=\"capacity=%lu\\lsize=%lu\\l\";\n"
			"\t\tdud_%s [shape=point, style=invis];\n", b_strs[b],
			(unsigned long)block->capacity, (unsigned long)block->size,
			b_strs[b]);
		for(node = PX_(block_nodes)(block), end = node + PX_(range)(p, block);
			node < end; node++) {
			PX_(to_string)(&node->data, &str);
			fprintf(fp, "\t\tnode%p [label=\"%s\", fillcolor=%s];\n",
				(const void *)node, str,
				node->x.prev ? "firebrick" : "lightsteelblue");
			/*if(node == beg) continue;
			fprintf(fp, "\t\tnode%p -> node%p [style=invis];\n",
				(const void *)(node - 1), (const void *)node);*/
		}
		fprintf(fp, "\t}\n");
	}
	if(p->removed.prev) {
		const struct PX_(x) *x0 = &p->removed, *x1 = x0->next, *turtle = x0;
		int is_turtle = 0;
		fprintf(fp, "\tedge [color=darkseagreen, constraint=false];\n");
		do {
			fprintf(fp, "\tnode%p -> node%p;\n",
				(const void *)PX_(x_const_upcast)(x0),
				(const void *)PX_(x_const_upcast)(x1));
			if(is_turtle) turtle = turtle->next, is_turtle=0; else is_turtle=1;
		} while(x0 = x1, x1 = x1->next, x0 != &p->removed && x0 != turtle);
		x0 = &p->removed, x1 = x0->prev, turtle = x0, is_turtle = 0;
		fprintf(fp, "\tedge [color=darkseagreen4];\n");
		do {
			fprintf(fp, "\tnode%p -> node%p;\n",
				(const void *)PX_(x_const_upcast)(x0),
				(const void *)PX_(x_const_upcast)(x1));
			if(is_turtle) turtle = turtle->prev, is_turtle=0; else is_turtle=1;
		} while(x0 = x1, x1 = x1->prev, x0 != &p->removed && x0 != turtle);
	}
#endif
	fprintf(fp, "\tnode [colour=red];\n"
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

static void PX_(test_basic)(void) {
	struct X_(pool) a = POOL_IDLE;
	PX_(type) ts[5], *t, *t1;
	const size_t ts_size = sizeof ts / sizeof *ts, big = 1000;
	size_t i;

	printf("Test null.\n");
	errno = 0;
	PX_(valid_state)(0);

	printf("Test empty.\n");
	PX_(valid_state)(&a);
	X_(pool)(&a);
	PX_(valid_state)(&a);
	/*assert(X_(pool_remove)(&a, &node.data) == 0 && errno == EDOM), errno = 0;
	assert(errno == 0);
	PX_(valid_state)(&a);*/

	printf("Test one element.\n");
	X_(pool_)(&a);
	for(i = 0; i < 5; i++) t = X_(pool_new)(&a), assert(t), PX_(filler)(t),
		PX_(valid_state)(&a); /* Add. */
	X_(pool_remove)(&a, PX_(datum)(a.slots.data[0]) + 1);

	PX_(graph)(&a, "graph/" QUOTE(POOL_NAME) "-one.gv");
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
	printf("Destructor:\n");
	X_(pool_)(&a);
	PX_(valid_state)(&a);
#endif
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
	PX_(test_basic)();
	/*PX_(test_random)();*/
	fprintf(stderr, "Done tests of <" QUOTE(POOL_NAME) ">pool.\n\n");
}

#else /* to string --><!-- */
#error Test unsupported option; testing is out-of-sync?
#endif /* --> */

#undef QUOTE
#undef QUOTE_

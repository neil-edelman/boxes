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

/** Private: {container_of}. */
static const struct PT_(Node) *
	PT_(x_const_upcast)(const struct PT_(X) *const x) {
	return (const struct PT_(Node) *)(const void *)
	((const char *)x - offsetof(const struct PT_(Node), x));
}

static void PT_(graph)(const struct T_(Pool) *const p, const char *const fn) {
	FILE *fp;
	struct PT_(Block) *block;
	const struct PT_(Node) *node, /* *beg, 10000 nodes bogs down dot */ *end;
	char str[12], b_strs[2][128] = { "pool", "???" };
	unsigned b = 0;
	assert(p && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\tgraph [compound=true, nslimit=3, nslimit1=3];\n"
		"\trankdir=LR;\n" /* *beg take this out. */
		"\tnode [shape=box, style=filled];\n"
		"\tsubgraph cluster_%s {\n"
		"\t\tdud_%s [label=\"Pool\\nnext_capacity=%lu\\l\"];\n",
		b_strs[b], b_strs[b], (unsigned long)p->next_capacity);
	if(p->removed.prev) fprintf(fp, "\t\tnode%p [label=\"removed\"];\n",
		(const void *)PT_(x_const_upcast)(&p->removed));
	fprintf(fp, "\t}\n"
		"\tedge [color=royalblue];\n");
	for(block = p->largest; block; block = block->smaller) {
		sprintf(b_strs[b = !b], "block%p", (const void *)block);
		fprintf(fp,
			"\tdud_%s -> dud_%s [ltail=cluster_%s, lhead=cluster_%s];\n",
			b_strs[!b], b_strs[b], b_strs[!b], b_strs[b]);
		fprintf(fp, "\tsubgraph cluster_%s {\n"
			"\t\tstyle=filled;\n"
			"\t\tcolor=lightgray;\n"
			"\t\tlabel=\"capacity=%lu\\lsize=%lu\\l\";\n"
			"\t\tdud_%s [shape=point, style=invis];\n", b_strs[b],
			(unsigned long)block->capacity, (unsigned long)block->size,
			b_strs[b]);
		for(node = PT_(block_nodes)(block), end = node + PT_(range)(p, block);
			node < end; node++) {
			PT_(to_string)(&node->data, &str);
			fprintf(fp, "\t\tnode%p [label=\"%s\", color=%s];\n",
				(const void *)node, str, node->x.prev ? "firebrick" : "white");
			/*if(node == beg) continue;
			fprintf(fp, "\t\tnode%p -> node%p [style=invis];\n",
				(const void *)(node - 1), (const void *)node);*/
		}
		fprintf(fp, "\t}\n");
	}
	if(p->removed.prev) {
		const struct PT_(X) *x0 = &p->removed, *x1 = x0->next, *turtle = x0;
		int is_turtle = 0;
		fprintf(fp, "\tedge [color=darkseagreen];\n");
		do {
			fprintf(fp, "\tnode%p -> node%p;\n",
				(const void *)PT_(x_const_upcast)(x0),
				(const void *)PT_(x_const_upcast)(x1));
			if(is_turtle) turtle = turtle->next, is_turtle=0; else is_turtle=1;
		} while(x0 = x1, x1 = x1->next, x0 != &p->removed && x0 != turtle);
		x0 = &p->removed, x1 = x0->prev, turtle = x0, is_turtle = 0;
		fprintf(fp, "\tedge [color=darkseagreen4];\n");
		do {
			fprintf(fp, "\tnode%p -> node%p;\n",
				(const void *)PT_(x_const_upcast)(x0),
				(const void *)PT_(x_const_upcast)(x1));
			if(is_turtle) turtle = turtle->prev, is_turtle=0; else is_turtle=1;
		} while(x0 = x1, x1 = x1->prev, x0 != &p->removed && x0 != turtle);
	}
	fprintf(fp, "}\n");
	fclose(fp);
}

static void PT_(valid_block)(const struct PT_(Block) *const b) {
	assert(b && b->capacity);
}

static void PT_(valid_state)(const struct T_(Pool) *const a) {
	struct PT_(Block) *block;
	const struct PT_(Node) *node;
	const size_t max_size = ((size_t)-1 - sizeof *block) / sizeof *node;
	/* Null is a valid state. */
	if(!a) return;
	assert(a && !a->removed.prev == !a->removed.next);
	assert(!a->largest
		   || (a->largest->capacity < a->next_capacity
			   && a->next_capacity <= max_size)
		   || (a->largest->capacity == a->next_capacity) == max_size);
	for(block = a->largest; block; block = block->smaller)
		PT_(valid_block)(block), assert(block == a->largest || block->size > 0);
	if(!a->largest) {
		assert(!a->removed.prev && !a->removed.next);
	} else if(a->removed.prev) {
		size_t forward = 0, back = 0;
		const struct PT_(X) *head = &a->removed, *x, *turtle = head;
		int is_turtle = 0;
		const struct PT_(Node) *const first = PT_(block_nodes)(a->largest),
			*const last = first + a->largest->size - 1;
		assert(head->next && head->prev && a->largest->size > 1);
		x = head;
		do {
			forward++, x = x->next;
			if(x == head) break;
			node = PT_(x_const_upcast)(x);
			if(is_turtle) turtle = turtle->next, is_turtle=0; else is_turtle=1;
				assert(x && node >= first && node < last);
				assert(&node->x != turtle);
		} while(1);
		turtle = head, is_turtle = 0, x = head;
		do {
			back++,    x = x->prev;
			if(x == head) break;
			node = PT_(x_const_upcast)(x);
			if(is_turtle) turtle = turtle->prev, is_turtle=0; else is_turtle=1;
			assert(x && node >= first && node < last && &node->x != turtle);
		} while(1);
		/* The removed counts for a thing. */
		assert(forward == back && a->largest->size >= forward);
	}
}



static void PT_(test_basic)(void) {
	struct T_(Pool) a;
	struct PT_(Node) node;
	T ts[5], *t, *t1;
	const size_t ts_size = sizeof ts / sizeof *ts, big = 1000;
	size_t i;

	printf("Test null.\n");
	errno = 0;
	T_(Pool_)(0);
	T_(Pool)(0);
	assert(T_(PoolRemove)(0, 0) == 0);
	T_(PoolClear)(0);
	assert(T_(PoolNew)(0) == 0);
	T_(PoolForEach)(0, 0);
	assert(!strcmp("null", T_(PoolToString(0))));
	assert(errno == 0);
	PT_(valid_state)(0);

	printf("Test empty.\n");
	T_(Pool)(&a);
	assert(T_(PoolRemove)(&a, 0)          == 0 && errno == 0);
	assert(T_(PoolRemove)(&a, &node.data) == 0 && errno == EDOM), errno = 0;
	T_(PoolForEach)(&a, 0);
	assert(errno == 0);
	PT_(valid_state)(&a);

	printf("Test one element.\n");
	t = T_(PoolNew)(&a), PT_(filler)(t); /* Add. */
	assert(t);
	PT_(valid_state)(&a);
	if(!T_(PoolRemove)(&a, t)) { perror("Error"), assert(0); return; }
	PT_(valid_state)(&a);
	t = T_(PoolNew)(&a), PT_(filler)(t); /* Add. */
	assert(t);
	PT_(valid_state)(&a);
	T_(PoolClear)(&a);
	PT_(valid_state)(&a);
	PT_(graph)(&a, QUOTE(POOL_NAME) "-zero.gv");

	/* @fixme valgrind is giving me grief if I don't do this? */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	for(t = ts, t1 = t + ts_size; t < t1; t++) PT_(filler)(t);

	printf("Testing %lu elements.\n", (unsigned long)ts_size);
	for(t1 = 0, i = 0; i < ts_size; i++) {
		t = T_(PoolNew)(&a);
		if(!t1) t1 = t;
		assert(t);
		memcpy(t, ts + i, sizeof *t);
	}
	printf("Now: %s.\n", T_(PoolToString)(&a));
	if(!T_(PoolRemove)(&a, t1)) { perror("Error"), assert(0); return; }
	printf("Now: %s.\n", T_(PoolToString)(&a));
	assert(!T_(PoolRemove)(&a, t1) && errno == EDOM);
	printf("(Deliberate) error: %s.\n", strerror(errno)), errno = 0;
	PT_(valid_state)(&a);
	PT_(graph)(&a, QUOTE(POOL_NAME) "-small.gv");
	t = T_(PoolNew)(&a), PT_(filler)(t); /* Cheating. */
	t = T_(PoolNew)(&a), PT_(filler)(t);
	t = T_(PoolNew)(&a), PT_(filler)(t);
	PT_(valid_state)(&a);
	T_(PoolReserve)(&a, 1000);
	PT_(graph)(&a, QUOTE(POOL_NAME) "-small-1000.gv");
	PT_(valid_state)(&a);
	T_(PoolClear)(&a);
	PT_(valid_state)(&a);

	/* Big. */
	for(i = 0; i < big; i++) {
		t = T_(PoolNew)(&a);
		assert(t);
		PT_(filler)(t);
	}
	printf("%s.\n", T_(PoolToString)(&a));
	PT_(valid_state)(&a);

	printf("Clear:\n");
	T_(PoolClear)(&a);
	printf("Now: %s.\n", T_(PoolToString)(&a));
	printf("Destructor:\n");
	T_(Pool_)(&a);
	PT_(valid_state)(&a);
	printf("Done basic tests.\n\n");
}

static void PT_(test_random)(void) {
	struct T_(Pool) a;
	size_t i, size = 0;
	const size_t length = 100000; /* Controls how many iterations. */
	T_(Pool)(&a);
	for(i = 0; i < length; i++) {
		char str[12];
		double r = rand() / (RAND_MAX + 1.0);
		/* This parameter controls how big the pool wants to be. */
		if(r > size / 5000.0) {
			T *data = T_(PoolNew)(&a);
			if(!data) { perror("Error"), assert(0); return;}
			size++;
			PT_(filler)(data);
			PT_(to_string)(data, &str);
			printf("%lu: Created %s.\n", (unsigned long)i, str);
		} else {
			struct PT_(Block) *block;
			struct PT_(Node) *node, *end;
			size_t idx = rand() / (RAND_MAX + 1.0) * size;
			assert(a.largest);
			/* Pick random. */
			for(block = a.largest; block; block = block->smaller) {
				for(node = PT_(block_nodes)(block), end = node
					+ (block == a.largest ? block->size : block->capacity);
					node < end; node++) {
					if(node->x.prev) continue;
					if(!idx) break;
					idx--;
				}
				if(node < end) break;
			}
			assert(block);
			PT_(to_string)(&node->data, &str);
			printf("%lu: Removing %s in block %p.\n", (unsigned long)i, str,
				(const void *)block);
			{
				const int ret = T_(PoolRemove)(&a, &node->data);
				assert(ret || (perror("Removing"),
					PT_(graph)(&a, QUOTE(POOL_NAME) "-rem-err.gv"), 0));
			}
			size--;
		}
		/* The file size is huge and dot balks. */
		if(i < 10000 && i % 5000 == 2500) {
			char fn[64];
			sprintf(fn, QUOTE(POOL_NAME) "-%u.gv", (unsigned)i);
			PT_(graph)(&a, fn);
			printf("%s.\n", T_(PoolToString)(&a));
		}
		PT_(valid_state)(&a);
	}
	PT_(graph)(&a, QUOTE(POOL_NAME) "-finish.gv");
	T_(Pool_)(&a);
}

/** The list will be tested on stdout. */
static void T_(PoolTest)(void) {
	printf("Pool<" QUOTE(POOL_NAME) ">: of type <" QUOTE(POOL_TYPE)
		"> was created using: "
#ifdef POOL_TO_STRING
		"POOL_TO_STRING<" QUOTE(POOL_TO_STRING) ">; "
#endif
#ifdef POOL_TEST
		"POOL_TEST<" QUOTE(POOL_TEST) ">; "
#endif
		"testing:\n");
	PT_(test_basic)();
	PT_(test_random)();
	fprintf(stderr, "Done tests of Pool<" QUOTE(POOL_NAME) ">.\n\n");
}

/* Un-define all macros. */
#undef QUOTE
#undef QUOTE_

/* Intended to be included by `Pool.h` on `POOL_TEST`. */

#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

#ifdef POOL_TO_STRING /* <!-- to string: Only one, tests all base code. */

/* Copy functions for later includes. */
static void (*PP_(to_string))(const PP_(type) *, char (*)[12])
	= (POOL_TO_STRING);
static const char *(*PP_(pool_to_string))(const struct P_(pool) *)
	= S_(to_string);

/* POOL_TEST must be a function that implements <typedef:<PP>Action>. */
static const PP_(action_fn) PP_(filler) = (POOL_TEST);

/** Private: `container_of` `x`. */
static const struct PP_(node) *
	PP_(x_const_upcast)(const struct PP_(x) *const x) {
	return (const struct PP_(node) *)(const void *)
	((const char *)x - offsetof(const struct PP_(node), x));
}

/** Tries to graphs `p` in `fn`. */
static void PP_(graph)(const struct P_(pool) *const p, const char *const fn) {
	FILE *fp;
	struct PP_(block) *block;
	const struct PP_(node) *node, /* *beg, 10000 nodes bogs down dot */ *end;
	char str[12], b_strs[2][128] = { "pool", "???" };
	unsigned b = 0;
	assert(p && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	fprintf(fp, "digraph {\n"
		"\tgraph [compound=true, nslimit=3, nslimit1=3];\n"
		"\trankdir=LR;\n"
		"\tedge [color=royalblue];\n"
		"\tnode [shape=record, style=filled, fillcolor=lightgray];\n"
		"\tnode%p [label=\"\\<" QUOTE(POOL_NAME)
		"\\>Pool\\l|next capacity %lu\\l|removed list\\l\"];\n",
		(const void *)PP_(x_const_upcast)(&p->removed), /*b_strs[b],*/
		(unsigned long)p->next_capacity);
	for(block = p->largest; block; block = block->smaller) {
		sprintf(b_strs[b = !b], "block%p", (const void *)block);
		/* This is cleaver but I don't know what I did. Hack. */
		if(block == p->largest) {
			fprintf(fp, "\tnode%p",
			(const void *)PP_(x_const_upcast)(&p->removed));
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
		for(node = PP_(block_nodes)(block), end = node + PP_(range)(p, block);
			node < end; node++) {
			PP_(to_string)(&node->data, &str);
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
		const struct PP_(x) *x0 = &p->removed, *x1 = x0->next, *turtle = x0;
		int is_turtle = 0;
		fprintf(fp, "\tedge [color=darkseagreen, constraint=false];\n");
		do {
			fprintf(fp, "\tnode%p -> node%p;\n",
				(const void *)PP_(x_const_upcast)(x0),
				(const void *)PP_(x_const_upcast)(x1));
			if(is_turtle) turtle = turtle->next, is_turtle=0; else is_turtle=1;
		} while(x0 = x1, x1 = x1->next, x0 != &p->removed && x0 != turtle);
		x0 = &p->removed, x1 = x0->prev, turtle = x0, is_turtle = 0;
		fprintf(fp, "\tedge [color=darkseagreen4];\n");
		do {
			fprintf(fp, "\tnode%p -> node%p;\n",
				(const void *)PP_(x_const_upcast)(x0),
				(const void *)PP_(x_const_upcast)(x1));
			if(is_turtle) turtle = turtle->prev, is_turtle=0; else is_turtle=1;
		} while(x0 = x1, x1 = x1->prev, x0 != &p->removed && x0 != turtle);
	}
	fprintf(fp, "}\n");
	fclose(fp);
}

/** Crashes if `b` is not a valid block.
 @implements <PP>Action */
static void PP_(valid_block)(const struct PP_(block) *const b) {
	assert(b && b->capacity);
}

/** Crashes if `a` is not in a valid state. */
static void PP_(valid_state)(const struct P_(pool) *const a) {
	struct PP_(block) *block;
	const struct PP_(node) *node;
	const size_t max_size = ((size_t)-1 - sizeof *block) / sizeof *node;
	/* Null is a valid state. */
	if(!a) return;
	assert(a && !a->removed.prev == !a->removed.next);
	assert(!a->largest
		   || (a->largest->capacity < a->next_capacity
			   && a->next_capacity <= max_size)
		   || (a->largest->capacity == a->next_capacity) == max_size);
	for(block = a->largest; block; block = block->smaller)
		PP_(valid_block)(block), assert(block == a->largest || block->size > 0);
	if(!a->largest) {
		assert(!a->removed.prev && !a->removed.next);
	} else if(a->removed.prev) {
		size_t forward = 0, back = 0;
		const struct PP_(x) *head = &a->removed, *x, *turtle = head;
		int is_turtle = 0;
		const struct PP_(node) *const first = PP_(block_nodes)(a->largest),
			*const last = first + a->largest->size - 1;
		assert(head->next && head->prev && a->largest->size > 1);
		x = head;
		do {
			forward++, x = x->next;
			if(x == head) break;
			node = PP_(x_const_upcast)(x);
			if(is_turtle) turtle = turtle->next, is_turtle=0; else is_turtle=1;
				assert(x && node >= first && node < last);
				assert(&node->x != turtle);
		} while(1);
		turtle = head, is_turtle = 0, x = head;
		do {
			back++,    x = x->prev;
			if(x == head) break;
			node = PP_(x_const_upcast)(x);
			if(is_turtle) turtle = turtle->prev, is_turtle=0; else is_turtle=1;
			assert(x && node >= first && node < last && &node->x != turtle);
		} while(1);
		/* The removed counts for a thing. */
		assert(forward == back && a->largest->size >= forward);
	}
}


/** Prints `data`. */
static void PP_(print)(PP_(type) *const data) {
	char a[12];
	assert(data);
	PP_(to_string)(data, &a);
	printf("> %s!\n", a);
}

static void PP_(test_basic)(void) {
	struct P_(pool) a = POOL_IDLE;
	struct PP_(node) node;
	PP_(type) ts[5], *t, *t1;
	const size_t ts_size = sizeof ts / sizeof *ts, big = 1000;
	size_t i;

	printf("Test null.\n");
	errno = 0;
	PP_(valid_state)(0);

	printf("Test empty.\n");
	PP_(valid_state)(&a);
	P_(pool)(&a);
	assert(P_(pool_remove)(&a, &node.data) == 0 && errno == EDOM), errno = 0;
	P_(pool_for_each)(&a, 0);
	assert(errno == 0);
	PP_(valid_state)(&a);

	printf("Test one element.\n");
	P_(pool_)(&a);
	assert(P_(pool_reserve)(&a, 1000) && a.next_capacity == 2584);
	t = P_(pool_new)(&a), PP_(filler)(t); /* Add. */
	assert(t);
	PP_(valid_state)(&a);
	if(!P_(pool_remove)(&a, t)) { perror("Error"), assert(0); return; }
	PP_(valid_state)(&a);
	t = P_(pool_new)(&a), PP_(filler)(t); /* Add. */
	assert(t);
	PP_(valid_state)(&a);
	P_(pool_clear)(&a);
	PP_(valid_state)(&a);
	PP_(graph)(&a, "graph/" QUOTE(POOL_NAME) "-zero.gv");
	P_(pool_)(&a);

	/* @fixme valgrind is giving me grief if I don't do this? */
	memset(ts, 0, sizeof ts);
	/* Get elements. */
	for(t = ts, t1 = t + ts_size; t < t1; t++) PP_(filler)(t);

	printf("Testing %lu elements.\n", (unsigned long)ts_size);
	for(t1 = 0, i = 0; i < ts_size; i++) {
		t = P_(pool_new)(&a);
		if(!t1) t1 = t;
		assert(t);
		memcpy(t, ts + i, sizeof *t);
	}
	printf("Now: %s.\n", PP_(pool_to_string)(&a));
	if(!P_(pool_remove)(&a, t1)) { perror("Error"), assert(0); return; }
	printf("Now: %s.\n", PP_(pool_to_string)(&a));
	assert(!P_(pool_remove)(&a, t1) && errno == EDOM);
	printf("(Deliberate) error: %s.\n", strerror(errno)), errno = 0;
	PP_(valid_state)(&a);
	PP_(graph)(&a, "graph/" QUOTE(POOL_NAME) "-small.gv");
	t = P_(pool_new)(&a), PP_(filler)(t); /* Cheating. */
	t = P_(pool_new)(&a), PP_(filler)(t);
	t = P_(pool_new)(&a), PP_(filler)(t);
	t = P_(pool_new)(&a), PP_(filler)(t);
	t = P_(pool_new)(&a), PP_(filler)(t);
	assert(P_(pool_remove)(&a, t));
	P_(pool_for_each)(&a, &PP_(print));
	PP_(valid_state)(&a);
	assert(!P_(pool_reserve)(&a, 1000) && errno == EDOM), errno = 0;
	PP_(graph)(&a, "graph/" QUOTE(POOL_NAME) "-small-1000.gv");
	PP_(valid_state)(&a);
	P_(pool_clear)(&a);
	PP_(valid_state)(&a);

	/* Big. */
	for(i = 0; i < big; i++) {
		t = P_(pool_new)(&a);
		assert(t);
		PP_(filler)(t);
	}
	printf("%s.\n", PP_(pool_to_string)(&a));
	PP_(valid_state)(&a);

	printf("Clear:\n");
	P_(pool_clear)(&a);
	printf("Now: %s.\n", PP_(pool_to_string)(&a));
	printf("Destructor:\n");
	P_(pool_)(&a);
	PP_(valid_state)(&a);
	printf("Done basic tests.\n\n");
}

static void PP_(test_random)(void) {
	struct P_(pool) a = POOL_IDLE;
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
			PP_(type) *data = P_(pool_new)(&a);
			if(!data) { perror("Error"), assert(0); return;}
			size++;
			PP_(filler)(data);
			PP_(to_string)(data, &str);
			if(is_print) printf("%lu: Created %s.\n", (unsigned long)i, str);
		} else {
			struct PP_(block) *block;
			struct PP_(node) *node = 0, *end;
			size_t idx = (unsigned)rand() / (RAND_MAX / size + 1);
			assert(a.largest);
			/* Pick random. */
			for(block = a.largest; block; block = block->smaller) {
				for(node = PP_(block_nodes)(block), end = node
					+ (block == a.largest ? block->size : block->capacity);
					node < end; node++) {
					if(node->x.prev) continue;
					if(!idx) break;
					idx--;
				}
				if(node < end) break;
			}
			assert(block);
			PP_(to_string)(&node->data, &str);
			if(is_print) printf("%lu: Removing %s in block %p.\n",
				(unsigned long)i, str, (const void *)block);
			{
				const int ret = P_(pool_remove)(&a, &node->data);
				assert(ret || (perror("Removing"),
					PP_(graph)(&a, "graph/" QUOTE(POOL_NAME) "-rem-err.gv"),0));
			}
			size--;
		}
		if(is_print && i < graph_max) {
			sprintf(graph_fn, "graph/" QUOTE(POOL_NAME) "-%u.gv", (unsigned)i);
			PP_(graph)(&a, graph_fn);
			printf("%s.\n", PP_(pool_to_string)(&a));
		}
		PP_(valid_state)(&a);
	}
	if(i < graph_max) {
		sprintf(graph_fn, "graph/" QUOTE(POOL_NAME) "-%u-end.gv", (unsigned)i);
		PP_(graph)(&a, graph_fn);
	}
	P_(pool_)(&a);
}

/** The list will be tested on stdout; requires `POOL_TEST` and not `NDEBUG`.
 @allow */
static void P_(pool_test)(void) {
	printf("<" QUOTE(POOL_NAME) ">pool: of type <" QUOTE(POOL_TYPE)
		"> was created using: "
#ifdef POOL_TO_STRING
		"POOL_TO_STRING<" QUOTE(POOL_TO_STRING) ">; "
#endif
#ifdef POOL_TEST
		"POOL_TEST<" QUOTE(POOL_TEST) ">; "
#endif
		"testing:\n");
	PP_(test_basic)();
	PP_(test_random)();
	fprintf(stderr, "Done tests of <" QUOTE(POOL_NAME) ">pool.\n\n");
}

#else /* to string --><!-- */
#error Test unsupported option; testing is out-of-sync?
#endif /* --> */

#undef QUOTE
#undef QUOTE_

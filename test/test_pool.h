/* Intended to be included by `Pool.h` on `POOL_TEST`. */

#include <limits.h>
#include <stdlib.h>

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
			fprintf(fp, "\";\n"
				"\t\t{ rank=same;\n");
			if(i || !chunk->size) {
				fprintf(fp, "\t\tdata%lu_0 [style=invis];\n", i);
			} else {
				struct { int is; size_t free; } *rem
					= calloc(chunk->size, sizeof *rem);
				if(!rem) {
					fprintf(fp, "\t\tdata%lu_0 [label=\"%s\", "
						"fillcolor=red];\n", (unsigned long)i, strerror(errno));
				} else {
					size_t k;
					for(k = 0; k < pool->free0.a.size; k++) {
						size_t *kc = pool->free0.a.data + k;
						assert(kc && *kc < chunk->size && !rem[*kc].is);
						rem[*kc].is = 1, rem[*kc].free = k;
					}
					for(j = 0; j < chunk->size; j++) {
						if(!rem[j].is) {
							PX_(to_string)(data + j, &str);
							fprintf(fp,
								"\t\tdata%lu_%lu [label=\"[%lu] %s\"];\n",
								(unsigned long)i, (unsigned long)j,
								(unsigned long)j, str);
						} else {
							fprintf(fp, "\t\tdata%lu_%lu [label=\"[%lu]\" "
								"shape=none, style=empty];\n",
								(unsigned long)i, (unsigned long)j,
								(unsigned long)j);
							if(rem[j].free) fprintf(fp, "\t\tdata%lu_%lu -> "
								"data%lu_%lu [constraint=false];\n",
								(unsigned long)i, (unsigned long)(
									pool->free0.a.data[(rem[j].free - 1) / 2]
								), (unsigned long)i, (unsigned long)j);
						}
						if(j) fprintf(fp, "\t\tdata%lu_%lu -> data%lu_%lu "
							"[style=invis];\n", (unsigned long)i,
							(unsigned long)j, (unsigned long)i,
							(unsigned long)j-1);
					}
					free(rem);
				}
			}
			fprintf(fp, "\t}}\n"
				"\tslot%lu -> data%lu_0;\n",
				(unsigned long)i, (unsigned long)i);
		}
	}
	if(pool->free0.a.size) {
		size_t i;
		fprintf(fp, "\tsubgraph cluster_free0 {\n"
			"\t\trankdir=TB; // doesn't do anything\n"
			"\t\tstyle=filled;\n"
			"\t\tlabel=\"free0 %lu/%lu\";\n"
			"\t\tnode [style=none, shape=none];\n",
			(unsigned long)pool->free0.a.size,
			(unsigned long)pool->free0.a.capacity);
		for(i = 0; i < pool->free0.a.size; i++) {
			fprintf(fp, "\t\tfree0_%lu [label=\"[%lu]\"];\n",
				i, pool->free0.a.data[i]);
			if(i) fprintf(fp, "\t\tfree0_%lu -> free0_%lu;\n",
				(unsigned long)(i - 1 >> 1), i);
		}
		fprintf(fp, "\t}\n"
			"\tpool -> free0_0;\n");
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
	PX_(type) *t;
	const size_t size[] = { 9, 14, 22 };
	size_t i, j;
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

	printf("Pool buffer 9.\n");
	r = X_(pool_buffer)(&pool, 9), assert(r), PX_(valid_state)(&pool);
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-04-buffer.gv");

	for(i = 0; i < size[0]; i++) t = X_(pool_new)(&pool), assert(t),
		PX_(filler)(t), PX_(valid_state)(&pool);
	assert(pool.slots.size == 1);
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-05-one-chunk.gv");
	for(i = 0; i < size[1]; i++) t = X_(pool_new)(&pool), assert(t),
		PX_(filler)(t), PX_(valid_state)(&pool);
	assert(pool.slots.size == 2);
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-06-two-chunks.gv");
	t = X_(pool_new)(&pool), assert(t), PX_(filler)(t), PX_(valid_state)(&pool);
	assert(pool.slots.size == 3);
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-07-three-chunks.gv");
	if(pool.slots.data[1]->size == size[0]) i = 0;
	else assert(pool.slots.data[1]->size == size[1]);
	if(pool.slots.data[2]->size == size[0]) i = 1;
	else assert(pool.slots.data[2]->size == size[1]);
	X_(pool_remove)(&pool, PX_(data)(pool.slots.data[!i + 1]));
	X_(pool_remove)(&pool, PX_(data)(pool.slots.data[0]));
	assert(pool.slots.data[i + 1]->size == size[0]);
	for(j = 0; j < size[0]; j++)
		X_(pool_remove)(&pool, PX_(data)(pool.slots.data[i + 1]));
	PX_(valid_state)(&pool);
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-08-remove-chunk.gv");
	assert(pool.slots.size == 2);
	X_(pool_clear)(&pool);
	assert(pool.slots.size == 1);
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-09-clear.gv");

	/* Remove at random. */
	for(i = 0; i < size[2]; i++) t = X_(pool_new)(&pool), assert(t), PX_(filler)(t), PX_(valid_state)(&pool);
	{
		const size_t n1 = size[2] - 1, n0 = n1 >> 1;
		size_t n;
		unsigned bits = 0, copy, copy_bit, rnd, rnd_bit;
		srand((unsigned)clock());
		assert(n1 <= sizeof bits * CHAR_BIT);
		for(n = 0; n < n0; n++) {
			rnd = (unsigned)rand() / (RAND_MAX / (n1 - n) + 1);
			rnd_bit = 1 << rnd;
			copy = bits;
			while(copy) {
				copy_bit = copy;
				copy &= copy - 1;
				copy_bit ^= copy;
				if(copy_bit > rnd_bit) break;
				rnd++, rnd_bit <<= 1;
			}
			bits |= rnd_bit;
		}
		for(i = 0; i < size[2]; i++)
			if(bits & (1 << i))
			X_(pool_remove)(&pool, PX_(data)(pool.slots.data[0]) + i);
		i = n0;
	}
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-10-remove.gv");
	assert(pool.slots.size == 1 && pool.slots.data[0]->size == size[2]
		&& pool.capacity0 == size[2] && pool.free0.a.size == i);

	/* Add at random to an already removed. */
	while(i) t = X_(pool_new)(&pool), assert(t),
		PX_(filler)(t), PX_(valid_state)(&pool), i--;
	PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-11-replace.gv");
	assert(pool.slots.size == 1 && pool.slots.data[0]->size == size[2]
		&& pool.capacity0 == size[2] && pool.free0.a.size == 0);

	printf("Destructor:\n");
	X_(pool_)(&pool);
	PX_(valid_state)(&pool);
	printf("Done basic tests.\n\n");
}

#define ARRAY_NAME PX_(test)
#define ARRAY_TYPE PX_(type) *
#define ARRAY_SUBTYPE
#define ARRAY_NO_ITERATE
#include "../src/array.h"

static void PX_(test_random)(void) {
	struct X_(pool) pool = POOL_IDLE;
	struct PX_(test_array) test = ARRAY_IDLE;
	size_t i;
	const size_t length = 120000; /* Controls how many iterations. */
	char graph_fn[64];
	const size_t graph_max = 100000;
	PX_(type) *data, **ptr;

	for(i = 0; i < length; i++) {
		char str[12];
		double r = rand() / (RAND_MAX + 1.0);
		int is_print = !(i & (i - 1));
		/* This parameter controls how big the pool wants to be. */
		if(r > test.size / 5000.0) {
			/* Create. */
			data = X_(pool_new)(&pool);
			if(!data) { perror("Error"), assert(0); return;}
			PX_(filler)(data);
			PX_(to_string)(data, &str);
			if(is_print) printf("%lu: Created %s.\n", (unsigned long)i, str);
			ptr = PX_(test_array_new)(&test);
			assert(ptr);
			*ptr = data;
		} else {
			/* Remove. */
			int ret;
			assert(test.size);
			ptr = test.data + (unsigned)rand() / (RAND_MAX / test.size + 1);
			PX_(to_string)(*ptr, &str);
			if(is_print) printf("%lu: Removing %s.\n", (unsigned long)i, str);
			ret = X_(pool_remove)(&pool, *ptr);
			assert(ret || (perror("Removing"),
				PX_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-rem-err.gv"), 0));
			PX_(test_array_remove)(&test, ptr);
		}
		if(is_print && i < graph_max) {
			sprintf(graph_fn, "graph/" QUOTE(POOL_NAME) "-%lu.gv",
				(unsigned long)(i + 1));
			PX_(graph)(&pool, graph_fn);
			printf("%s.\n", PX_(pool_to_string)(&pool));
		}
		PX_(valid_state)(&pool);
	}
	if(i < graph_max) {
		sprintf(graph_fn, "graph/" QUOTE(POOL_NAME) "-%lu-end.gv",
			(unsigned long)i);
		PX_(graph)(&pool, graph_fn);
	}
	PX_(test_array_)(&test);
	X_(pool_)(&pool);
}

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
	PX_(test_random)();
	fprintf(stderr, "Done tests of <" QUOTE(POOL_NAME) ">pool.\n\n");
}

#else /* to string --><!-- */
#error Test unsupported option; testing is out-of-sync?
#endif /* --> */

#undef QUOTE
#undef QUOTE_

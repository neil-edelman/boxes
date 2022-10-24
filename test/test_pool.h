/* Intended to be included on `POOL_TEST`. */

#include <limits.h>
#include <stdlib.h>

#if defined(QUOTE) || defined(QUOTE_)
#error QUOTE_? cannot be defined.
#endif
#define QUOTE_(name) #name
#define QUOTE(name) QUOTE_(name)

/** Operates by side-effects. */
typedef void (*PP_(action_fn))(PP_(type) *);

/** Graphs `pool` output to `fn`. */
static void PP_(graph)(const struct P_(pool) *const pool,
	const char *const fn) {
	FILE *fp;
	char str[12];
	size_t i, j;
	struct PP_(slot) *slot;
	PP_(type) *slab;

	assert(pool && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	printf("*** %s\n", fn);
	fprintf(fp, "digraph {\n"
		"\tgraph [rankdir=LR, truecolor=true, bgcolor=transparent,"
		" fontname=modern];\n"
		"\tnode [shape=none, fontname=modern];\n");
	if(!pool->free0.as_array.size) goto no_free0;
	for(i = 0; i < pool->free0.as_array.size; i++) {
		fprintf(fp, "\tfree0_%lu [label=<<font color=\"Gray50\" "
			" face=\"Times-Italic\">%lu</font>>, width=0, height=0,"
			" margin=0.05];\n", i, pool->free0.as_array.data[i]);
		if(i) fprintf(fp, "\tfree0_%lu -> free0_%lu [dir=back];\n",
			i, (unsigned long)((i - 1) / 2));
	}
	fprintf(fp, "\t{rank=same; pool; free0_0; }\n"
		"\tpool:free -> free0_0;\n");
no_free0:
	fprintf(fp, "\tpool [label=<\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td colspan=\"3\" align=\"left\">"
		"<font color=\"Grey75\">&lt;" QUOTE(POOL_NAME)
		"&gt;pool: " QUOTE(POOL_TYPE) "</font></td></tr>\n"
		"\t<hr/>\n"
		"\t<tr>\n"
		"\t\t<td border=\"0\" align=\"right\">"
		"capacity0</td>\n"
		"\t\t<td border=\"0\">&#8205;</td>\n"
		"\t\t<td border=\"0\" align=\"right\">%lu</td>\n"
		"\t</tr>\n", (unsigned long)pool->capacity0);
	fprintf(fp, "\t<tr>\n"
		"\t\t<td border=\"0\" align=\"right\" bgcolor=\"Gray95\">slots</td>\n"
		"\t\t<td border=\"0\" align=\"right\" bgcolor=\"Gray95\">%lu</td>\n"
		"\t\t<td port=\"slots\" border=\"0\" align=\"right\" "
		"bgcolor=\"Gray95\">%lu</td>\n"
		"\t</tr>\n"
		"\t<tr>\n"
		"\t\t<td border=\"0\" align=\"right\">freeheap0</td>\n"
		"\t\t<td border=\"0\" align=\"right\">%lu</td>\n"
		"\t\t<td port=\"free\" border=\"0\" align=\"right\">%lu</td>\n"
		"\t</tr>\n"
		"\t<hr/>\n"
		"\t<tr><td></td></tr>\n"
		"</table>>];\n",
		(unsigned long)pool->slots.size,
		(unsigned long)pool->slots.capacity,
		(unsigned long)pool->free0.as_array.size,
		(unsigned long)pool->free0.as_array.capacity);
	if(!pool->slots.data) goto no_slots;
	fprintf(fp, "\tpool:slots -> slots;\n"
		"\tslots [label = <\n"
		"<table border=\"0\" cellspacing=\"0\">\n"
		"\t<tr><td></td></tr>\n"
		"\t<hr/>\n"
		"\t<tr>\n"
		"\t\t<td border=\"0\"><font face=\"Times-Italic\">i</font></td>\n"
		"\t\t<td border=\"0\"><font face=\"Times-Italic\">slab</font></td>\n"
		"\t\t<td border=\"0\"><font face=\"Times-Italic\">size</font></td>\n"
		"\t</tr>\n"
		"\t<hr/>\n");
	for(i = 0; i < pool->slots.size; i++) {
		const char *const bgc = i & 1 ? " bgcolor=\"Grey95\"" : "";
		fprintf(fp, "\t<tr>\n"
			"\t\t<td align=\"right\"%s>%lu</td>\n"
			"\t\t<td align=\"left\"%s>%s</td>\n"
			"\t\t<td port=\"%lu\" align=\"right\"%s>%lu</td>\n"
			"\t</tr>\n",
			bgc, (unsigned long)i, bgc, orcify(pool->slots.data[i].slab),
			(unsigned long)i, bgc, pool->slots.data[i].size);
	}
	fprintf(fp, "\t<hr/>\n"
		"\t<tr><td></td></tr>\n"
		"</table>>];\n");
	/* For each slot, there is a slab. */
	for(i = 0; i < pool->slots.size; i++) {
		char *bmp;
		slot = pool->slots.data + i;
		slab = slot->slab;
		fprintf(fp,
			"\tslots:%lu -> slab%lu;\n"
			"\tslab%lu [label=<\n"
			"<table border=\"0\" cellspacing=\"0\">\n"
			"\t<tr><td align=\"left\">"
			"<font color=\"Gray75\">%s</font></td></tr>\n"
			"\t<hr/>\n",
			(unsigned long)i, (unsigned long)i,
			(unsigned long)i, orcify(slab));
		if(i || !slot->size) {
			fprintf(fp, "\t<tr><td align=\"left\">"
				"<font face=\"Times-Italic\">count %lu</font></td></tr>\n",
				slot->size);
			goto no_slab_data;
		}
		/* Primary buffer: print rows. */
		if(!(bmp = calloc(slot->size, sizeof *bmp)))
			{ perror("temp bitmap"); assert(0); exit(EXIT_FAILURE); }
		for(j = 0; j < pool->free0.as_array.size; j++) {
			size_t *f0p = pool->free0.as_array.data + j;
			assert(f0p && *f0p < slot->size);
			bmp[*f0p] = 1;
		}
		for(j = 0; j < slot->size; j++) {
			const char *const bgc = j & 1 ? " bgcolor=\"Grey95\"" : "";
			fprintf(fp, "\t<tr><td port=\"%lu\" align=\"left\"%s>",
				(unsigned long)j, bgc);
			if(bmp[j]) {
				fprintf(fp, "<font color=\"Gray50\" face=\"Times-Italic\">%lu</font>",
					(unsigned long)j);
			} else {
				P_(to_string)(slab + j, &str);
				fprintf(fp, "%s", str);
			}
			fprintf(fp, "</td></tr>\n");
		}
		free(bmp);
no_slab_data:
		fprintf(fp, "\t<hr/>\n"
			"\t<tr><td></td></tr>\n"
			"</table>>];\n");
		/* Too crowded to draw heap.
		if(i) continue;
		for(j = 1; j < pool->free0._.size; j++) {
			const size_t *const f0low = pool->free0._.data + j / 2,
				*const f0high = pool->free0._.data + j;
			fprintf(fp, "\tslab0:%lu -> slab0:%lu;\n", *f0low, *f0high);
		} */
	}
no_slots:
	fprintf(fp, "\tnode [fillcolour=red];\n"
		"}\n");
	fclose(fp);
}

/** Crashes if `pool` is not in a valid state. */
static void PP_(valid_state)(const struct P_(pool) *const pool) {
	size_t i;
	if(!pool) return;
	/* If there's no capacity, there's no slots. */
	if(!pool->capacity0) assert(!pool->slots.size);
	/* Every slot up to size is active. */
	for(i = 0; i < pool->slots.size; i++) {
		assert(pool->slots.data[i].slab);
		assert(!i || pool->slots.data[i].size);
	}
	if(!pool->slots.size) {
		/* There are no free0 without slots. */
		assert(!pool->free0.as_array.size);
	} else {
		/* size[0] <= capacity0 */
		assert(pool->slots.data[0].size <= pool->capacity0);
		/* The free-heap indices are strictly less than the size. */
		for(i = 0; i < pool->free0.as_array.size; i++)
			assert(pool->free0.as_array.data[i] < pool->slots.data[0].size);
	}
}

static void PP_(test_states)(void) {
	struct P_(pool) pool = P_(pool)();
	PP_(type) *t, *slab;
	const size_t size[] = { 9, 14, 22 };
	enum { CHUNK1_IS_ZERO, CHUNK2_IS_ZERO } conf = CHUNK1_IS_ZERO;
	size_t i;
	int r;
	char z[12];

	printf("Test null.\n");
	errno = 0;
	PP_(valid_state)(0);

	printf("Empty.\n");
	PP_(valid_state)(&pool);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-01-idle.gv");

	/** `<P>filler` must be a function implementing <typedef:<PP>action_fn>. */
	printf("One element.\n");
	t = P_(pool_new)(&pool), assert(t), P_(filler)(t), PP_(valid_state)(&pool);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-02-one.gv");

	printf("Remove.\n");
	r = P_(pool_remove)(&pool, pool.slots.data[0].slab + 0), assert(r),
		PP_(valid_state)(&pool);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-03-remove.gv");

	printf("Pool buffer %lu.\n", (unsigned long)size[0]);
	r = P_(pool_buffer)(&pool, size[0]), assert(r), PP_(valid_state)(&pool);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-04-buffer.gv");

	for(i = 0; i < size[0]; i++) t = P_(pool_new)(&pool), assert(t),
		P_(filler)(t), PP_(valid_state)(&pool);
	assert(pool.slots.size == 1);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-05-one-slab.gv");
	for(i = 0; i < size[1]; i++) t = P_(pool_new)(&pool), assert(t),
		P_(filler)(t), PP_(valid_state)(&pool);
	assert(pool.slots.size == 2);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-06-two-slabs.gv");
	t = P_(pool_new)(&pool), assert(t), P_(filler)(t), PP_(valid_state)(&pool);
	assert(pool.slots.size == 3);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-07-three-slabs.gv");
	/* It's `malloc` whether, `conf = { 0: { 2, 0, 1 }, 1: { 2, 1, 0 } }`. */
	if(pool.slots.data[1].size == size[0]) conf = CHUNK1_IS_ZERO;
	else assert(pool.slots.data[1].size == size[1]);
	if(pool.slots.data[2].size == size[0]) conf = CHUNK2_IS_ZERO;
	else assert(pool.slots.data[2].size == size[1]);
	assert(pool.slots.data[0].size == 1);
	printf("%s is %u, %s is %u.\n", orcify(pool.slots.data[1].slab), conf,
		orcify(pool.slots.data[2].slab), !conf);
	t = pool.slots.data[!conf + 1].slab + 0, P_(to_string)(t, &z);
	printf("Removing index-zero %s from slab %u %s.\n", z, !conf + 1,
		orcify(pool.slots.data[!conf + 1].slab));
	P_(pool_remove)(&pool, t), PP_(valid_state)(&pool);
	t = pool.slots.data[0].slab + 0, P_(to_string)(t, &z);
	printf("Removing index-zero %s from slab %u %s.\n", z, 0,
		orcify(pool.slots.data[0].slab));
	P_(pool_remove)(&pool, pool.slots.data[0].slab + 0);
	PP_(valid_state)(&pool);
	assert(pool.slots.data[conf + 1].size == size[0]);
	slab = pool.slots.data[conf + 1].slab;
	printf("Removing all in slab %s.\n", orcify(slab));
	for(i = 0; i < size[0]; i++) P_(pool_remove)(&pool, slab + i);
	PP_(valid_state)(&pool);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-08-remove-slab.gv");
	assert(pool.slots.size == 2);
	P_(pool_clear)(&pool);
	assert(pool.slots.size == 1);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-09-clear.gv");

	/* Remove at random. */
	for(i = 0; i < size[2]; i++) t = P_(pool_new)(&pool), assert(t), P_(filler)(t), PP_(valid_state)(&pool);
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
		for(i = 0; i < size[2]; i++) if(bits & (1 << i))
			P_(pool_remove)(&pool, pool.slots.data[0].slab + i);
		i = n0;
	}
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-10-remove.gv");
	assert(pool.slots.size == 1 && pool.slots.data[0].size == size[2]
		&& pool.capacity0 == size[2] && pool.free0.as_array.size == i);

	/* Add at random to an already removed. */
	while(i) t = P_(pool_new)(&pool), assert(t),
		P_(filler)(t), PP_(valid_state)(&pool), i--;
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-11-replace.gv");
	assert(pool.slots.size == 1 && pool.slots.data[0].size == size[2]
		&& pool.capacity0 == size[2] && pool.free0.as_array.size == 0);

	printf("Destructor:\n");
	P_(pool_)(&pool);
	PP_(valid_state)(&pool);
	printf("Done basic tests.\n\n");
}

/* #define ARRAY_NAME PP_(test)
#define ARRAY_TYPE PP_(type) *
#include "../src/array.h" <- Now BOX is defined in this; too much work. */

static void PP_(test_random)(void) {
	struct P_(pool) pool = P_(pool)();
	/* This parameter controls how big the pool wants to be. */
	struct { size_t size; PP_(type) *data[2*5000]; } test;
	const size_t test_size = sizeof test.data / sizeof *test.data,
		length = 120000; /* Controls how many iterations. */
	size_t i;
	char graph_fn[64];
	const size_t graph_max = 100000;
	PP_(type) *data, **ptr;

	test.size = 0;
	for(i = 0; i < length; i++) {
		char str[12];
		unsigned r = (unsigned)rand();
		int is_print = !(i & (i - 1));
		if(r > test.size * (RAND_MAX / test_size) && test.size < test_size) {
			/* Create. */
			data = P_(pool_new)(&pool);
			if(!data) { perror("Error"), assert(0); return;}
			P_(filler)(data);
			P_(to_string)(data, &str);
			if(is_print) printf("%lu: Created %s.\n", (unsigned long)i, str);
			test.data[test.size++] = data;
		} else {
			/* Remove. */
			int ret;
			assert(test.size);
			ptr = test.data + (unsigned)rand() / (RAND_MAX / test.size + 1);
			P_(to_string)(*ptr, &str);
			if(is_print) printf("%lu: Removing %s.\n", (unsigned long)i, str);
			ret = P_(pool_remove)(&pool, *ptr);
			assert(ret || (perror("Removing"),
				PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-rem-err.gv"), 0));
			/* test.remove(ptr) */
			memmove(ptr, ptr + 1,
				sizeof *test.data * (--test.size - (size_t)(ptr - test.data)));
		}
		if(is_print) printf("test.size: %lu.\n", (unsigned long)test.size);
		if(is_print && i < graph_max) {
			sprintf(graph_fn, "graph/" QUOTE(POOL_NAME) "-step-%lu.gv",
				(unsigned long)(i + 1));
			PP_(graph)(&pool, graph_fn);
			printf("%s.\n", P_(pool_to_string)(&pool));
		}
		PP_(valid_state)(&pool);
	}
	if(i < graph_max) {
		sprintf(graph_fn, "graph/" QUOTE(POOL_NAME) "-step-%lu-end.gv",
			(unsigned long)i);
		PP_(graph)(&pool, graph_fn);
	}
	P_(pool_)(&pool);
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
	PP_(test_states)();
	PP_(test_random)();
	fprintf(stderr, "Done tests of <" QUOTE(POOL_NAME) ">pool.\n\n");
}

#undef QUOTE
#undef QUOTE_

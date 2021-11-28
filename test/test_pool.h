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

/** `POOL_TEST` must be a function that implements <typedef:<PP>action_fn>. */
static const PP_(action_fn) PP_(filler) = (POOL_TEST);

/** Tries to graphs `pool` output to `fn`. */
static void PP_(graph)(const struct P_(pool) *const pool,
	const char *const fn) {
	FILE *fp;
	char str[12];
	size_t i, j;
	struct pool_chunk *chunk;
	PP_(type) *data;

	assert(pool && fn);
	if(!(fp = fopen(fn, "w"))) { perror(fn); return; }
	printf("*** %s\n", fn);
	fprintf(fp, "digraph {\n"
		"\trankdir=LR;\n"
		"\tfontface=modern;"
		"\tnode [shape=box, style=filled, fillcolor=\"Gray95\"];\n");
	if(!pool->free0.a.size) goto no_free0;
	for(i = 0; i < pool->free0.a.size; i++) {
		fprintf(fp, "\tfree0_%lu [label=<<FONT COLOR=\"Gray75\">%lu</FONT>>,"
			" shape=circle];\n", i, pool->free0.a.data[i]);
		if(i) fprintf(fp, "\tfree0_%lu -> free0_%lu [dir=back];\n",
			i, (unsigned long)((i - 1) / 2));
	}
	fprintf(fp, "\t{rank=same; pool; free0_0; }\n"
		"\tpool:free -> free0_0;\n");
no_free0:
	fprintf(fp, "\tpool [label=<\n"
		"<TABLE BORDER=\"0\">\n"
		"\t<TR><TD COLSPAN=\"3\" ALIGN=\"LEFT\">"
		"<FONT COLOR=\"Gray85\">&lt;" QUOTE(POOL_NAME)
		"&gt;pool</FONT></TD></TR>\n"
		"\t<TR>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">"
		"capacity0</TD>\n"
		"\t\t<TD BORDER=\"0\" BGCOLOR=\"Gray90\">&#8205;</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">%lu</TD>\n"
		"\t</TR>\n"
		"\t<TR>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\">slots"
		"</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\">%lu</TD>\n"
		"\t\t<TD PORT=\"slots\" BORDER=\"0\" ALIGN=\"RIGHT\">%lu</TD>\n"
		"\t</TR>\n"
		"\t<TR>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">"
		"freeheap0</TD>\n"
		"\t\t<TD BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">%lu</TD>\n"
		"\t\t<TD PORT=\"free\" BORDER=\"0\" ALIGN=\"RIGHT\" BGCOLOR=\"Gray90\">"
		"%lu</TD>\n"
		"\t</TR>\n"
		"</TABLE>>];\n",
		(unsigned long)pool->capacity0,
		(unsigned long)pool->slots.size,
		(unsigned long)pool->slots.capacity,
		(unsigned long)pool->free0.a.size,
		(unsigned long)pool->free0.a.capacity);
	if(!pool->slots.data) goto no_slots;
	fprintf(fp, "\tpool:slots -> slots;\n"
		"\tslots [label = <\n"
		"<TABLE BORDER=\"0\">\n"
		"\t<TR>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Italic\">i</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\"><FONT FACE=\"Times-Italic\">chunk"
		"</FONT></TD>\n"
		"\t\t<TD BORDER=\"0\">"
		"<FONT FACE=\"Times-Italic\">size</FONT></TD>\n"
		"\t</TR>\n");
	for(i = 0; i < pool->slots.size; i++) {
		const char *const bgc = i & 1 ? "" : " BGCOLOR=\"Gray90\"";
		fprintf(fp, "\t<TR>\n"
			"\t\t<TD ALIGN=\"RIGHT\"%s>%lu</TD>\n"
			"\t\t<TD ALIGN=\"LEFT\"%s>%s</TD>\n"
			"\t\t<TD PORT=\"%lu\" ALIGN=\"RIGHT\"%s>%lu</TD>\n"
			"\t</TR>\n",
			bgc, (unsigned long)i, bgc, orcify(pool->slots.data[i]),
			(unsigned long)i, bgc, pool->slots.data[i]->size);
	}
	fprintf(fp, "</TABLE>>];\n");
	/* For each slot, there is a chunk array with data. */
	for(i = 0; i < pool->slots.size; i++) {
		char *bmp;
		chunk = pool->slots.data[i];
		data = PP_(data)(chunk);
		fprintf(fp,
			"\tslots:%lu -> chunk%lu;\n"
			"\tchunk%lu [label=<\n"
			"<TABLE BORDER=\"0\">\n"
			"\t<TR><TD COLSPAN=\"2\" ALIGN=\"LEFT\">"
			"<FONT COLOR=\"Gray85\">%s</FONT></TD></TR>\n",
			(unsigned long)i, (unsigned long)i,
			(unsigned long)i, orcify(chunk));
		if(i || !chunk->size) {
			fprintf(fp, "\t<TR><TD COLSPAN=\"2\" ALIGN=\"LEFT\">"
				"<FONT FACE=\"Times-Italic\">count %lu</FONT></TD></TR>\n",
				chunk->size);
			goto no_chunk_data;
		}
		/* Primary buffer: print rows. */
		if(!(bmp = calloc(chunk->size, sizeof *bmp)))
			{ perror("temp bitmap"); assert(0); exit(EXIT_FAILURE); };
		for(j = 0; j < pool->free0.a.size; j++) {
			size_t *f0p = pool->free0.a.data + j;
			assert(f0p && *f0p < chunk->size);
			bmp[*f0p] = 1;
		}
		for(j = 0; j < chunk->size; j++) {
			const char *const bgc = j & 1 ? "" : " BGCOLOR=\"Gray90\"";
			fprintf(fp, "\t<TR>\n"
				"\t\t<TD PORT=\"%lu\" ALIGN=\"RIGHT\"%s>%lu</TD>\n",
				(unsigned long)j, bgc, (unsigned long)j);
			if(bmp[j]) {
				fprintf(fp, "\t\t<TD ALIGN=\"LEFT\"%s>"
					"<FONT COLOR=\"Gray75\">deleted"
					"</FONT></TD>\n", bgc);
			} else {
				PP_(to_string)(data + j, &str);
				fprintf(fp, "\t\t<TD ALIGN=\"LEFT\"%s>%s</TD>\n", bgc, str);
			}
			fprintf(fp, "\t</TR>\n");
		}
		free(bmp);
no_chunk_data:
		fprintf(fp, "</TABLE>>];\n");
		/* Too crowded!
		if(i) continue;
		for(j = 1; j < pool->free0.a.size; j++) {
			const size_t *const f0low = pool->free0.a.data + j / 2,
				*const f0high = pool->free0.a.data + j;
			fprintf(fp, "\tchunk0:%lu -> chunk0:%lu;\n", *f0low, *f0high);
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

static void PP_(test_states)(void) {
	struct P_(pool) pool = POOL_IDLE;
	PP_(type) *t;
	const size_t size[] = { 9, 14, 22 };
	size_t i, j;
	int r;

	printf("Test null.\n");
	errno = 0;
	PP_(valid_state)(0);

	printf("Empty.\n");
	PP_(valid_state)(&pool);
	P_(pool)(&pool);
	PP_(valid_state)(&pool);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-01-idle.gv");

	printf("One element.\n");
	t = P_(pool_new)(&pool), assert(t), PP_(filler)(t), PP_(valid_state)(&pool);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-02-one.gv");

	printf("Remove.\n");
	r = P_(pool_remove)(&pool, PP_(data)(pool.slots.data[0])), assert(r),
		PP_(valid_state)(&pool);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-03-remove.gv");

	printf("Pool buffer 9.\n");
	r = P_(pool_buffer)(&pool, 9), assert(r), PP_(valid_state)(&pool);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-04-buffer.gv");

	for(i = 0; i < size[0]; i++) t = P_(pool_new)(&pool), assert(t),
		PP_(filler)(t), PP_(valid_state)(&pool);
	assert(pool.slots.size == 1);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-05-one-chunk.gv");
	for(i = 0; i < size[1]; i++) t = P_(pool_new)(&pool), assert(t),
		PP_(filler)(t), PP_(valid_state)(&pool);
	assert(pool.slots.size == 2);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-06-two-chunks.gv");
	t = P_(pool_new)(&pool), assert(t), PP_(filler)(t), PP_(valid_state)(&pool);
	assert(pool.slots.size == 3);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-07-three-chunks.gv");
	if(pool.slots.data[1]->size == size[0]) i = 0;
	else assert(pool.slots.data[1]->size == size[1]);
	if(pool.slots.data[2]->size == size[0]) i = 1;
	else assert(pool.slots.data[2]->size == size[1]);
	P_(pool_remove)(&pool, PP_(data)(pool.slots.data[!i + 1]));
	P_(pool_remove)(&pool, PP_(data)(pool.slots.data[0]));
	assert(pool.slots.data[i + 1]->size == size[0]);
	for(j = 0; j < size[0]; j++)
		P_(pool_remove)(&pool, PP_(data)(pool.slots.data[i + 1]));
	PP_(valid_state)(&pool);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-08-remove-chunk.gv");
	assert(pool.slots.size == 2);
	P_(pool_clear)(&pool);
	assert(pool.slots.size == 1);
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-09-clear.gv");

	/* Remove at random. */
	for(i = 0; i < size[2]; i++) t = P_(pool_new)(&pool), assert(t), PP_(filler)(t), PP_(valid_state)(&pool);
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
			P_(pool_remove)(&pool, PP_(data)(pool.slots.data[0]) + i);
		i = n0;
	}
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-10-remove.gv");
	assert(pool.slots.size == 1 && pool.slots.data[0]->size == size[2]
		&& pool.capacity0 == size[2] && pool.free0.a.size == i);

	/* Add at random to an already removed. */
	while(i) t = P_(pool_new)(&pool), assert(t),
		PP_(filler)(t), PP_(valid_state)(&pool), i--;
	PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-11-replace.gv");
	assert(pool.slots.size == 1 && pool.slots.data[0]->size == size[2]
		&& pool.capacity0 == size[2] && pool.free0.a.size == 0);

	printf("Destructor:\n");
	P_(pool_)(&pool);
	PP_(valid_state)(&pool);
	printf("Done basic tests.\n\n");
}

#define ARRAY_NAME PP_(test)
#define ARRAY_TYPE PP_(type) *
#include "../src/array.h"

static void PP_(test_random)(void) {
	struct P_(pool) pool = POOL_IDLE;
	struct PP_(test_array) test = ARRAY_IDLE;
	size_t i;
	const size_t length = 120000; /* Controls how many iterations. */
	char graph_fn[64];
	const size_t graph_max = 100000;
	PP_(type) *data, **ptr;

	for(i = 0; i < length; i++) {
		char str[12];
		unsigned r = (unsigned)rand();
		int is_print = !(i & (i - 1));
		/* This parameter controls how big the pool wants to be. */
		if(r > test.size * (RAND_MAX / (2 * 5000))) {
			/* Create. */
			data = P_(pool_new)(&pool);
			if(!data) { perror("Error"), assert(0); return;}
			PP_(filler)(data);
			PP_(to_string)(data, &str);
			if(is_print) printf("%lu: Created %s.\n", (unsigned long)i, str);
			ptr = PP_(test_array_new)(&test);
			assert(ptr);
			*ptr = data;
		} else {
			/* Remove. */
			int ret;
			assert(test.size);
			ptr = test.data + (unsigned)rand() / (RAND_MAX / test.size + 1);
			PP_(to_string)(*ptr, &str);
			if(is_print) printf("%lu: Removing %s.\n", (unsigned long)i, str);
			ret = P_(pool_remove)(&pool, *ptr);
			assert(ret || (perror("Removing"),
				PP_(graph)(&pool, "graph/" QUOTE(POOL_NAME) "-rem-err.gv"), 0));
			PP_(test_array_remove)(&test, ptr);
		}
		if(is_print) printf("test.size: %lu.\n", (unsigned long)test.size);
		if(is_print && i < graph_max) {
			sprintf(graph_fn, "graph/" QUOTE(POOL_NAME) "-step-%lu.gv",
				(unsigned long)(i + 1));
			PP_(graph)(&pool, graph_fn);
			printf("%s.\n", PP_(pool_to_string)(&pool));
		}
		PP_(valid_state)(&pool);
	}
	if(i < graph_max) {
		sprintf(graph_fn, "graph/" QUOTE(POOL_NAME) "-step-%lu-end.gv",
			(unsigned long)i);
		PP_(graph)(&pool, graph_fn);
	}
	PP_(test_array_)(&test);
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
